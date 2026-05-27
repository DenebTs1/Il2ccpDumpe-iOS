//
//  CSectionHelper.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CSectionHelper.h"
#include "../Il2Cpp/Versions.h"
#include "../Core/Log.h"

#include <algorithm>
#include <cstring>

namespace Il2Dumper
{
    static const uint8_t k_FeatureMscorlib[] = {
        0x6D, 0x73, 0x63, 0x6F, 0x72, 0x6C, 0x69, 0x62, 0x2E, 0x64, 0x6C, 0x6C, 0x00
    };

    CSectionHelper::CSectionHelper(CMachO64* p_Mach,
                                   int     m_MethodCount,
                                   int     m_TypeDefCount,
                                   int64_t m_MetadataUsagesCount,
                                   int     m_ImageCount)
        : p_Mach_(p_Mach)
        , m_MethodCount_(m_MethodCount)
        , m_TypeDefCount_(m_TypeDefCount)
        , m_MetadataUsagesCount_(m_MetadataUsagesCount)
        , m_ImageCount_(m_ImageCount)
    {
    }

    void CSectionHelper::SetSection(E_SectionKind m_Kind, const std::vector<SSearchSection>& m_Secs)
    {
        switch (m_Kind)
        {
        case E_SectionKind::E_Exec: m_Exec_ = m_Secs; break;
        case E_SectionKind::E_Data: m_Data_ = m_Secs; break;
        case E_SectionKind::E_Bss:  m_Bss_  = m_Secs; break;
        }
    }

    uint64_t CSectionHelper::FindCodeRegistration()
    {
    #ifdef IL2CPP_VER_GE_24_2
        // Mach-O: data first, exec fallback (matches C# branch for non-ELF).
        IL2D_LOG("FindCodeRegistration: scanning data sections (%zu)...", m_Data_.size());
        uint64_t m_Cr = FindCodeRegistration2019(m_Data_);
        if (m_Cr == 0)
        {
            IL2D_LOG("FindCodeRegistration: data miss, scanning exec sections (%zu)...", m_Exec_.size());
            m_Cr = FindCodeRegistration2019(m_Exec_);
            bPointerInExec_ = true;
        }
        return m_Cr;
    #else
        IL2D_LOG("FindCodeRegistration: legacy <24.2 path");
        return FindCodeRegistrationOld();
    #endif
    }

    uint64_t CSectionHelper::FindMetadataRegistration()
    {
    #ifndef IL2CPP_VER_GE_19
        IL2D_LOG("FindMetadataRegistration: target<19 -> 0");
        return 0;
    #else
        #ifdef IL2CPP_VER_GE_27
            IL2D_LOG("FindMetadataRegistration: using V21 path (target>=27)");
            return FindMetadataRegistrationV21();
        #else
            IL2D_LOG("FindMetadataRegistration: using legacy path (target<27)");
            return FindMetadataRegistrationOld();
        #endif
    #endif
    }

    bool CSectionHelper::CheckPointerRangeDataRa(uint64_t m_Ptr) const
    {
        for (const auto& s : m_Data_)
        {
            if (m_Ptr >= s.m_Offset && m_Ptr <= s.m_OffsetEnd)
            {
                return true;
            }
        }
        return false;
    }

    bool CSectionHelper::CheckPointerRangeExecVa(const std::vector<uint64_t>& m_Ptrs) const
    {
        for (uint64_t x : m_Ptrs)
        {
            bool bHit = false;
            for (const auto& s : m_Exec_)
            {
                if (x >= s.m_Address && x <= s.m_AddressEnd)
                {
                    bHit = true;
                    break;
                }
            }
            if (!bHit)
            {
                return false;
            }
        }
        return !m_Ptrs.empty();
    }

    bool CSectionHelper::CheckPointerRangeDataVa(const std::vector<uint64_t>& m_Ptrs) const
    {
        for (uint64_t x : m_Ptrs)
        {
            bool bHit = false;
            for (const auto& s : m_Data_)
            {
                if (x >= s.m_Address && x <= s.m_AddressEnd)
                {
                    bHit = true;
                    break;
                }
            }
            if (!bHit)
            {
                return false;
            }
        }
        return !m_Ptrs.empty();
    }

    bool CSectionHelper::CheckPointerRangeBssVa(const std::vector<uint64_t>& m_Ptrs) const
    {
        for (uint64_t x : m_Ptrs)
        {
            bool bHit = false;
            for (const auto& s : m_Bss_)
            {
                if (x >= s.m_Address && x <= s.m_AddressEnd)
                {
                    bHit = true;
                    break;
                }
            }
            if (!bHit)
            {
                return false;
            }
        }
        return !m_Ptrs.empty();
    }

    void CSectionHelper::FindReference(uint64_t m_Addr, std::vector<uint64_t>& m_Out)
    {
        CBinaryStream& m_S       = p_Mach_->Stream();
        const size_t   m_PtrSize = m_S.PointerSize();
        
        for (const auto& sec : m_Data_)
        {
            uint64_t m_Pos = sec.m_Offset;
            uint64_t m_End = std::min<uint64_t>(sec.m_OffsetEnd, m_S.Length()) - m_PtrSize;
            while (m_Pos < m_End)
            {
                m_S.SeekTo(m_Pos);
                uint64_t m_V = p_Mach_->ReadVaPointer();
                if (m_V == m_Addr)
                {
                    m_Out.push_back(m_Pos - sec.m_Offset + sec.m_Address);
                }
                m_Pos += m_PtrSize;
            }
        }
    }

    uint64_t CSectionHelper::FindCodeRegistration2019(const std::vector<SSearchSection>& m_Secs)
    {
        CBinaryStream& m_S       = p_Mach_->Stream();
        const size_t   m_PtrSize = m_S.PointerSize();
        size_t         m_MscorlibHits = 0;

        for (const auto& sec : m_Secs)
        {
            if (sec.m_OffsetEnd <= sec.m_Offset)
            {
                continue;
            }
            const uint8_t* p_Base = m_S.Bytes() + sec.m_Offset;
            uint64_t       m_Len  = sec.m_OffsetEnd - sec.m_Offset;
            
            if (sec.m_Offset + m_Len > m_S.Length())
            {
                continue;
            }

            // Naive substring search for "mscorlib.dll\0".
            for (uint64_t i = 0; i + sizeof(k_FeatureMscorlib) <= m_Len; ++i)
            {
                if (std::memcmp(p_Base + i, k_FeatureMscorlib, sizeof(k_FeatureMscorlib)) != 0) { continue; }
                uint64_t m_DllVa = sec.m_Address + i;
                ++m_MscorlibHits;
                
                IL2D_LOG("  mscorlib.dll found at VA 0x%llx (section off=0x%llx)",
                         (unsigned long long)m_DllVa, (unsigned long long)sec.m_Offset);

                std::vector<uint64_t> m_Ref1;
                FindReference(m_DllVa, m_Ref1);
                IL2D_LOG("    ref1 hits=%zu", m_Ref1.size());
                for (uint64_t m_RefVa : m_Ref1)
                {
                    std::vector<uint64_t> m_Ref2;
                    FindReference(m_RefVa, m_Ref2);
                    IL2D_LOG("    ref2 hits=%zu (for ref1 VA 0x%llx)",
                             m_Ref2.size(), (unsigned long long)m_RefVa);
                    
                    for (uint64_t m_RefVa2 : m_Ref2)
                    {
                    #ifdef IL2CPP_VER_GE_27
                        for (int k = m_ImageCount_ - 1; k >= 0; --k)
                        {
                            uint64_t              m_Target = m_RefVa2 - uint64_t(k) * m_PtrSize;
                            std::vector<uint64_t> m_Ref3;
                            FindReference(m_Target, m_Ref3);
                            for (uint64_t m_RefVa3 : m_Ref3)
                            {
                                uint64_t m_Off = p_Mach_->MapVATR(m_RefVa3 - m_PtrSize);
                                if (m_Off == 0)
                                {
                                    continue;
                                }
                                m_S.SeekTo(m_Off);
                                if (int(m_S.ReadIntPtr()) == m_ImageCount_)
                                {
                                #ifdef IL2CPP_VER_GE_29
                                    return m_RefVa3 - m_PtrSize * 14;
                                #else
                                    return m_RefVa3 - m_PtrSize * 13;
                                #endif
                                }
                            }
                        }
                    #else
                        for (int k = 0; k < m_ImageCount_; ++k)
                        {
                            uint64_t              m_Target = m_RefVa2 - uint64_t(k) * m_PtrSize;
                            std::vector<uint64_t> m_Ref3;
                            FindReference(m_Target, m_Ref3);
                            if (!m_Ref3.empty())
                            {
                                return m_Ref3.front() - m_PtrSize * 13;
                            }
                        }
                    #endif
                    }
                }
            }
        }
        
        IL2D_LOG("FindCodeRegistration2019: bailed (mscorlib hits=%zu, no valid imageCount match)",
                 m_MscorlibHits);
        return 0;
    }

    uint64_t CSectionHelper::FindCodeRegistrationOld()
    {
        CBinaryStream& m_S       = p_Mach_->Stream();
        const size_t   m_PtrSize = m_S.PointerSize();
        for (const auto& sec : m_Data_)
        {
            uint64_t m_Pos = sec.m_Offset;
            
            while (m_Pos < sec.m_OffsetEnd)
            {
                m_S.SeekTo(m_Pos);
                int64_t m_V = m_S.ReadIntPtr();
                if (m_V == int64_t(m_MethodCount_))
                {
                    uint64_t m_Ptr = p_Mach_->MapVATR(p_Mach_->ReadVaPointer());
                    if (m_Ptr && CheckPointerRangeDataRa(m_Ptr))
                    {
                        m_S.SeekTo(m_Ptr);
                        std::vector<uint64_t> m_Ptrs;
                        m_Ptrs.reserve(size_t(m_MethodCount_));
                        for (int k = 0; k < m_MethodCount_; ++k)
                        {
                            m_Ptrs.push_back(p_Mach_->ReadVaPointer());
                        }
                        if (CheckPointerRangeExecVa(m_Ptrs))
                        {
                            return m_Pos - sec.m_Offset + sec.m_Address;
                        }
                    }
                }
                m_Pos += m_PtrSize;
            }
        }
        return 0;
    }

    uint64_t CSectionHelper::FindMetadataRegistrationV21()
    {
        CBinaryStream& m_S       = p_Mach_->Stream();
        const size_t   m_PtrSize = m_S.PointerSize();
        IL2D_LOG("FindMetadataRegistrationV21: scanning %zu data sections for typeDefCount=%d (doubled)",
                 m_Data_.size(), m_TypeDefCount_);
        for (const auto& sec : m_Data_)
        {
            uint64_t m_Pos = sec.m_Offset;
            uint64_t m_End = std::min<uint64_t>(sec.m_OffsetEnd, m_S.Length()) - m_PtrSize;
            
            while (m_Pos < m_End)
            {
                m_S.SeekTo(m_Pos);
                int64_t m_V = m_S.ReadIntPtr();
                if (m_V == int64_t(m_TypeDefCount_))
                {
                    m_S.SeekTo(m_S.Position() + m_PtrSize);
                    int64_t m_V2 = m_S.ReadIntPtr();
                    
                    if (m_V2 == int64_t(m_TypeDefCount_))
                    {
                        uint64_t m_Ptr = p_Mach_->MapVATR(p_Mach_->ReadVaPointer());
                        
                        if (m_Ptr && CheckPointerRangeDataRa(m_Ptr))
                        {
                            m_S.SeekTo(m_Ptr);
                            std::vector<uint64_t> m_Ptrs;
                            m_Ptrs.reserve(size_t(m_TypeDefCount_));
                            for (int k = 0; k < m_TypeDefCount_; ++k)
                            {
                                m_Ptrs.push_back(p_Mach_->ReadVaPointer());
                            }
                            bool bHit = bPointerInExec_
                                ? CheckPointerRangeExecVa(m_Ptrs)
                                : CheckPointerRangeDataVa(m_Ptrs);
                            if (bHit)
                            {
                                return m_Pos - m_PtrSize * 10 - sec.m_Offset + sec.m_Address;
                            }
                        }
                    }
                }
                m_Pos += m_PtrSize;
            }
        }
        IL2D_LOG("FindMetadataRegistrationV21: no match found");
        return 0;
    }

    uint64_t CSectionHelper::FindMetadataRegistrationOld()
    {
        CBinaryStream& m_S       = p_Mach_->Stream();
        const size_t   m_PtrSize = m_S.PointerSize();
        
        for (const auto& sec : m_Data_)
        {
            uint64_t m_Pos = sec.m_Offset;
            uint64_t m_End = std::min<uint64_t>(sec.m_OffsetEnd, m_S.Length()) - m_PtrSize;
            
            while (m_Pos < m_End)
            {
                m_S.SeekTo(m_Pos);
                int64_t m_V = m_S.ReadIntPtr();
                if (m_V == int64_t(m_TypeDefCount_))
                {
                    m_S.SeekTo(m_S.Position() + m_PtrSize * 2);
                    uint64_t m_Ptr = p_Mach_->MapVATR(p_Mach_->ReadVaPointer());
                    if (m_Ptr && CheckPointerRangeDataRa(m_Ptr))
                    {
                        m_S.SeekTo(m_Ptr);
                        std::vector<uint64_t> m_Ptrs;
                        m_Ptrs.reserve(size_t(m_MetadataUsagesCount_));
                        for (int64_t k = 0; k < m_MetadataUsagesCount_; ++k)
                        {
                            m_Ptrs.push_back(p_Mach_->ReadVaPointer());
                        }
                        if (CheckPointerRangeBssVa(m_Ptrs))
                        {
                            return m_Pos - m_PtrSize * 12 - sec.m_Offset + sec.m_Address;
                        }
                    }
                }
                m_Pos += m_PtrSize;
            }
        }
        return 0;
    }
}
