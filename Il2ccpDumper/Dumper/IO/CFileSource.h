//
//  CFileSource.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "IDataSource.h"

#include <string>
#include <vector>

namespace Il2Dumper
{
    /* Reads a whole file into a heap-allocated buffer on construction.
    Small enough for metadata + binary; mmap-backed loading is not worth
    the extra plumbing for a one-shot dump. */
    class CFileSource final : public IDataSource
    {
    public:
        explicit CFileSource(const std::string& m_Path);

        const uint8_t* Data() const override { return m_Buffer_.empty() ? nullptr : m_Buffer_.data(); }
        size_t         Size() const override { return m_Buffer_.size(); }
        bool           IsOk() const override { return bOk_; }

    private:
        std::vector<uint8_t> m_Buffer_;
        bool                 bOk_ { false };
    };
}
