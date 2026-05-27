//
//  CMachoFat.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "SMachoTypes.h"
#include "../IO/IDataSource.h"

namespace Il2Dumper
{
    /* Thin Mach-O fat-archive reader. The header is big-endian; the slices
    we care about for iOS are the arm64 ones (FEEDFACF). */

    class CMachoFat
    {
    public:
        explicit CMachoFat(const IDataSource& m_Src);

        const std::vector<SFatSlice>& Slices() const { return m_Slices_; }

        // Returns true if the slice's bytes were copied into out.
        bool ExtractSlice(size_t m_Index, std::vector<uint8_t>& m_OutBytes) const;

        // Picks the first arm64 (FEEDFACF) slice, or -1 if none.
        int PickArm64() const;

    private:
        const uint8_t*         p_Bytes_ { nullptr };
        size_t                 m_Size_  { 0 };
        std::vector<SFatSlice> m_Slices_;
    };
}
