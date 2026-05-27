//
//  CMemorySource.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "IDataSource.h"

namespace Il2Dumper
{
    class CMemorySource final : public IDataSource
    {
    public:
        CMemorySource(const void* p_Bytes, size_t m_Bytes)
            : p_Bytes_(static_cast<const uint8_t*>(p_Bytes))
            , m_Bytes_(m_Bytes)
        {
        }

        const uint8_t* Data() const override { return p_Bytes_; }
        size_t         Size() const override { return m_Bytes_; }
        bool           IsOk() const override { return p_Bytes_ != nullptr && m_Bytes_ != 0; }

    private:
        const uint8_t* p_Bytes_;
        size_t         m_Bytes_;
    };
}
