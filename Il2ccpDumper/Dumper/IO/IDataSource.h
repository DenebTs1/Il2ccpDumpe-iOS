//
//  IDataSource.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace Il2Dumper
{
    /* Abstract view over a contiguous byte buffer. Implementations may own
    their bytes (CFileSource maps a file) or borrow them (CMemorySource
    wraps a region of the live process address space) */
    class IDataSource
    {
    public:
        virtual ~IDataSource() = default;

        virtual const uint8_t* Data() const = 0;
        virtual size_t         Size() const = 0;
        virtual bool           IsOk() const = 0;
    };
}
