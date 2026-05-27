//
//  CBinaryStream.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "IDataSource.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

namespace Il2Dumper
{
    /* Little-endian random-access reader over an IDataSource. C# BinaryReader
    equivalent, minus the writer (we only read). Position is a uint64 file
    offset, NOT a virtual address: callers translate VAs first. */
    class CBinaryStream
    {
    public:
        explicit CBinaryStream(const IDataSource& m_Src)
            : p_Bytes_(m_Src.Data())
            , m_Size_(m_Src.Size())
        {
        }

        uint64_t       Position() const          { return m_Pos_; }
        void           SeekTo(uint64_t v)        { m_Pos_ = v; }
        size_t         Length() const            { return m_Size_; }
        bool           Is32Bit() const           { return bIs32Bit_; }
        void           SetIs32Bit(bool v)        { bIs32Bit_ = v; }
        size_t         PointerSize() const       { return bIs32Bit_ ? 4u : 8u; }
        const uint8_t* Bytes() const             { return p_Bytes_; }

        /* Bounds-checked memcpy from m_Off into p_Out. Returns false if the
        request would overflow the buffer. */
        bool Copy(uint64_t m_Off, void* p_Out, size_t m_N) const
        {
            if (m_Off + m_N > m_Size_) return false;
            std::memcpy(p_Out, p_Bytes_ + m_Off, m_N);
            return true;
        }

        template <typename T>
        T Read()
        {
            /* Bounds-checked: returns a zeroed T past EOF instead of
             walking into unmapped memory. Critical when the source is a
             live framework mapping -- reads past `m_Size_` would land in
             unmapped pages and EXC_BAD_ACCESS the host process */
            
            T v{};
            if (m_Pos_ + sizeof(T) > m_Size_)
            {
                m_Pos_ = m_Size_;
                return v;
            }
            std::memcpy(&v, p_Bytes_ + m_Pos_, sizeof(T));
            m_Pos_ += sizeof(T);
            return v;
        }

        template <typename T>
        T ReadAt(uint64_t m_Off)
        {
            m_Pos_ = m_Off;
            return Read<T>();
        }

        int8_t   ReadI8 () { return Read<int8_t>();  }
        uint8_t  ReadU8 () { return Read<uint8_t>(); }
        int16_t  ReadI16() { return Read<int16_t>();  }
        uint16_t ReadU16() { return Read<uint16_t>(); }
        int32_t  ReadI32() { return Read<int32_t>();  }
        uint32_t ReadU32() { return Read<uint32_t>(); }
        int64_t  ReadI64() { return Read<int64_t>();  }
        uint64_t ReadU64() { return Read<uint64_t>(); }

        int64_t  ReadIntPtr () { return bIs32Bit_ ? int64_t(ReadI32()) : ReadI64(); }
        uint64_t ReadUIntPtr() { return bIs32Bit_ ? uint64_t(ReadU32()) : ReadU64(); }

        std::vector<uint8_t> ReadBytes(size_t m_N)
        {
            std::vector<uint8_t> v(m_N);
            if (m_Pos_ + m_N <= m_Size_)
            {
                std::memcpy(v.data(), p_Bytes_ + m_Pos_, m_N);
                m_Pos_ += m_N;
            }
            return v;
        }

        // Mono-style compressed uint (attribute blobs / metadata fields).
        uint32_t ReadCompressedU32()
        {
            uint8_t b0 = ReadU8();
            if ((b0 & 0x80) == 0)    return b0;
            if ((b0 & 0xC0) == 0x80) return (uint32_t(b0 & 0x3F) << 8) | ReadU8();
            if ((b0 & 0xE0) == 0xC0)
            {
                uint32_t b1 = ReadU8();
                uint32_t b2 = ReadU8();
                uint32_t b3 = ReadU8();
                return (uint32_t(b0 & 0x1F) << 24) | (b1 << 16) | (b2 << 8) | b3;
            }
            if (b0 == 0xF0) return ReadU32();
            if (b0 == 0xFE) return uint32_t(~0u) - 1;
            return uint32_t(~0u);
        }

        int32_t ReadCompressedI32()
        {
            uint32_t v = ReadCompressedU32();
            if (v == uint32_t(~0u)) return INT32_MIN;
            bool bNeg = (v & 1) != 0;
            v >>= 1;
            return bNeg ? -int32_t(v + 1) : int32_t(v);
        }

        std::string ReadStringToNull(uint64_t m_Off)
        {
            std::string s;
            uint64_t i = m_Off;
            while (i < m_Size_ && p_Bytes_[i] != 0)
            {
                s.push_back(static_cast<char>(p_Bytes_[i]));
                ++i;
            }
            return s;
        }

    private:
        const uint8_t* p_Bytes_  { nullptr };
        size_t         m_Size_   { 0 };
        uint64_t       m_Pos_    { 0 };
        bool           bIs32Bit_ { false };
    };
}
