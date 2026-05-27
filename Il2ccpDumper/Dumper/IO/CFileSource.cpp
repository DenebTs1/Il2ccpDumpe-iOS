//
//  CFileSource.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CFileSource.h"

#include <cstdio>

namespace Il2Dumper
{
    CFileSource::CFileSource(const std::string& m_Path)
    {
        FILE* p_File = std::fopen(m_Path.c_str(), "rb");
        if (!p_File)
        {
            return;
        }
        std::fseek(p_File, 0, SEEK_END);
        long m_Len = std::ftell(p_File);
        std::fseek(p_File, 0, SEEK_SET);
        if (m_Len <= 0)
        {
            std::fclose(p_File);
            return;
        }
        m_Buffer_.resize(static_cast<size_t>(m_Len));
        size_t m_Got = std::fread(m_Buffer_.data(), 1, m_Buffer_.size(), p_File);
        std::fclose(p_File);
        bOk_ = (m_Got == m_Buffer_.size());
    }
}
