//
//  Types.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "../../Utilities/Core.h"

namespace Il2Dumper
{
    enum class E_Status : int
    {
        E_Ok                       = 0,
        E_BadSanity                = -1,
        E_UnsupportedMetaVersion   = -2,
        E_UnsupportedBinaryFormat  = -3,
        E_NoArm64Slice             = -4,
        E_SearchFailed             = -5,
        E_BadAddress               = -6,
        E_IoFailure                = -7,
        E_OutOfMemory              = -8,
        E_BadArgument              = -9,
        E_NotInitialised           = -10
    };

    enum class E_Format : uint32_t
    {
        E_Unknown   = 0,
        E_MachO32   = 0xFEEDFACE,
        E_MachO64   = 0xFEEDFACF,
        E_MachoFatB = 0xCAFEBABE,
        E_MachoFatL = 0xBEBAFECA
    };

    enum class E_MetadataUsage : uint32_t
    {
        E_Invalid       = 0,
        E_TypeInfo      = 1,
        E_Il2CppType    = 2,
        E_MethodDef     = 3,
        E_FieldInfo     = 4,
        E_StringLiteral = 5,
        E_MethodRef     = 6
    };

    static constexpr uint32_t k_MetadataSanity = 0xFAB11BAF;
    static constexpr int      k_MinMetaVersion = 16;
    static constexpr int      k_MaxMetaVersion = 31;
}
