//
//  CMachO64.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CMachO64.h"
#include "../Core/Log.h"

#include <cstring>

namespace Il2Dumper
{
    static std::string ReadName(const uint8_t* p, size_t m_N)
    {
        std::string s;
        for (size_t i = 0; i < m_N && p[i]; ++i)
        {
            s.push_back(char(p[i]));
        }
        return s;
    }

    CMachO64::CMachO64(const IDataSource& m_Src)
        : m_Stream_(m_Src)
    {
        m_Stream_.SetIs32Bit(false);
        bOk_ = Parse();
    }

    bool CMachO64::Parse()
    {
        if (m_Stream_.Length() < 32)
        {
            IL2D_LOG("CMachO64: buffer too small (%zu bytes)", m_Stream_.Length());
            return false;
        }
        uint32_t m_Magic = 0;
        std::memcpy(&m_Magic, m_Stream_.Bytes(), 4);
        if (m_Magic != 0xFEEDFACF)
        {
            IL2D_LOG("CMachO64: bad magic 0x%08x (expected FEEDFACF)", m_Magic);
            return false;
        }

        m_Stream_.SeekTo(16);                              // skip magic, cputype, cpusubtype, filetype
        uint32_t m_NCmds = m_Stream_.ReadU32();
        m_Stream_.SeekTo(m_Stream_.Position() + 12);       // sizeofcmds, flags, reserved

        for (uint32_t i = 0; i < m_NCmds; ++i)
        {
            uint64_t m_CmdStart = m_Stream_.Position();
            uint32_t m_Cmd      = m_Stream_.ReadU32();
            uint32_t m_CmdSize  = m_Stream_.ReadU32();

            switch (m_Cmd)
            {
            case 0x19: // LC_SEGMENT_64
            {
                uint8_t m_SegName[16];
                if (!m_Stream_.Copy(m_Stream_.Position(), m_SegName, 16))
                {
                    return false;
                }
                m_Stream_.SeekTo(m_Stream_.Position() + 16);
                std::string m_Seg = ReadName(m_SegName, 16);

                if (m_Seg == "__TEXT")
                {
                    m_VmAddr_ = m_Stream_.ReadU64();
                }
                else
                {
                    m_Stream_.SeekTo(m_Stream_.Position() + 8); // skip vmaddr
                }

                m_Stream_.SeekTo(m_Stream_.Position() + 32);    // skip vmsize, fileoff, filesize, maxprot, initprot
                uint32_t m_NSects = m_Stream_.ReadU32();
                m_Stream_.SeekTo(m_Stream_.Position() + 4);     // skip flags

                for (uint32_t j = 0; j < m_NSects; ++j)
                {
                    SMachoSection64 m_Sec;
                    uint8_t         m_NameBuf[16];
                    if (!m_Stream_.Copy(m_Stream_.Position(), m_NameBuf, 16))
                    {
                        return false;
                    }
                    m_Stream_.SeekTo(m_Stream_.Position() + 16);
                    m_Sec.m_SectName = ReadName(m_NameBuf, 16);
                    m_Stream_.SeekTo(m_Stream_.Position() + 16); // skip segname
                    m_Sec.m_Addr   = m_Stream_.ReadU64();
                    m_Sec.m_Size   = m_Stream_.ReadU64();
                    m_Sec.m_Offset = m_Stream_.ReadU32();
                    m_Stream_.SeekTo(m_Stream_.Position() + 12); // skip align, reloff, nreloc
                    m_Sec.m_Flags  = m_Stream_.ReadU32();
                    m_Stream_.SeekTo(m_Stream_.Position() + 12); // skip reserved1..3
                    m_Sections_.push_back(m_Sec);
                }
                break;
            }
            case 0x2C: // LC_ENCRYPTION_INFO_64
            {
                m_Stream_.SeekTo(m_Stream_.Position() + 8);
                (void)m_Stream_.ReadU32();
                break;
            }
            default:
                break;
            }
            m_Stream_.SeekTo(m_CmdStart + m_CmdSize);
        }
        IL2D_LOG("CMachO64: parsed %u load cmds, vmaddr=0x%llx, %zu sections",
                 m_NCmds, (unsigned long long)m_VmAddr_, m_Sections_.size());
        for (size_t i = 0; i < m_Sections_.size(); ++i)
        {
            const auto& s = m_Sections_[i];
            IL2D_LOG("  sect[%zu] %-16s addr=0x%llx size=0x%llx off=0x%llx flags=0x%08x",
                     i, s.m_SectName.c_str(),
                     (unsigned long long)s.m_Addr,
                     (unsigned long long)s.m_Size,
                     (unsigned long long)s.m_Offset, s.m_Flags);
        }
        return !m_Sections_.empty();
    }

    uint64_t CMachO64::MapVATR(uint64_t m_Addr) const
    {
        for (const auto& s : m_Sections_)
        {
            if (m_Addr >= s.m_Addr && m_Addr <= s.m_Addr + s.m_Size)
            {
                if (s.m_SectName == "__bss")
                {
                    return 0;
                }
                return m_Addr - s.m_Addr + s.m_Offset;
            }
        }
        return 0;
    }

    uint64_t CMachO64::ReadVaPointer()
    {
        uint64_t m_Ptr = m_Stream_.ReadU64();
        return ScrubVa(m_Ptr);
    }

    uint64_t CMachO64::ScrubVa(uint64_t m_Ptr) const
    {
        if (m_Ptr == 0) return 0;

        /* Live in-memory image: dyld put us at m_Slide_; every stored
         pointer is `slide + relative`. Subtract slide to recover the
         relative VA we search against */
        
        if (m_Slide_ != 0 && m_Ptr > m_Slide_)
        {
            return m_Ptr - m_Slide_;
        }

        /* Otherwise apply the original C# Macho64 trick, works for on-disk
         binaries (vmaddr=0x100000000) and 4-GB-aligned dumps */
        if (m_Ptr > m_VmAddr_ + 0xFFFFFFFFull)
        {
            uint64_t m_Rva = (m_Ptr - m_VmAddr_) & 0xFFFFFFFFull;
            return m_Rva + m_VmAddr_;
        }
        return m_Ptr;
    }
}
