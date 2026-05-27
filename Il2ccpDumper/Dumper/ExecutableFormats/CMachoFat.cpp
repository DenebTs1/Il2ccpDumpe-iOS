//
//  CMachoFat.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CMachoFat.h"

#include <cstring>

namespace Il2Dumper
{
    static uint32_t BE32(const uint8_t* p)
    {
        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16)
             | (uint32_t(p[2]) << 8 ) |  uint32_t(p[3]);
    }

    CMachoFat::CMachoFat(const IDataSource& m_Src)
        : p_Bytes_(m_Src.Data())
        , m_Size_(m_Src.Size())
    {
        if (!p_Bytes_ || m_Size_ < 8)
        {
            return;
        }
        uint32_t m_NSlices = BE32(p_Bytes_ + 4);
        m_Slices_.reserve(m_NSlices);
        for (uint32_t i = 0; i < m_NSlices; ++i)
        {
            const uint8_t* p_Entry = p_Bytes_ + 8 + i * 20;
            if (p_Entry + 20 > p_Bytes_ + m_Size_)
            {
                break;
            }
            SFatSlice s;
            s.m_Offset = BE32(p_Entry + 8);
            s.m_Size   = BE32(p_Entry + 12);
            if (s.m_Offset + 4 <= m_Size_)
            {
                std::memcpy(&s.m_Magic, p_Bytes_ + s.m_Offset, 4); // little-endian in slice
            }
            m_Slices_.push_back(s);
        }
    }

    bool CMachoFat::ExtractSlice(size_t m_Index, std::vector<uint8_t>& m_OutBytes) const
    {
        if (m_Index >= m_Slices_.size())
        {
            return false;
        }
        const SFatSlice& s = m_Slices_[m_Index];
        if (uint64_t(s.m_Offset) + s.m_Size > m_Size_)
        {
            return false;
        }
        m_OutBytes.assign(p_Bytes_ + s.m_Offset, p_Bytes_ + s.m_Offset + s.m_Size);
        return true;
    }

    int CMachoFat::PickArm64() const
    {
        for (size_t i = 0; i < m_Slices_.size(); ++i)
        {
            if (m_Slices_[i].m_Magic == 0xFEEDFACF)
            {
                return int(i);
            }
        }
        return -1;
    }
}
