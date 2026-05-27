//
//  Il2CppConstants.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

namespace Il2Dumper
{
    /* ECMA-335 TypeAttributes / MethodAttributes / FieldAttributes,
     mirrored from the C# Il2CppConstants.cs. Names follow the project
     E_ enum convention; values are unchanged */

    enum E_TypeAttribute : unsigned
    {
        E_TA_VisibilityMask        = 0x00000007,
        E_TA_NotPublic             = 0x00000000,
        E_TA_Public                = 0x00000001,
        E_TA_NestedPublic          = 0x00000002,
        E_TA_NestedPrivate         = 0x00000003,
        E_TA_NestedFamily          = 0x00000004,
        E_TA_NestedAssembly        = 0x00000005,
        E_TA_NestedFamAndAssem     = 0x00000006,
        E_TA_NestedFamOrAssem      = 0x00000007,
        E_TA_LayoutMask            = 0x00000018,
        E_TA_AutoLayout            = 0x00000000,
        E_TA_SequentialLayout      = 0x00000008,
        E_TA_ExplicitLayout        = 0x00000010,
        E_TA_ClassSemanticsMask    = 0x00000020,
        E_TA_Class                 = 0x00000000,
        E_TA_Interface             = 0x00000020,
        E_TA_Abstract              = 0x00000080,
        E_TA_Sealed                = 0x00000100,
        E_TA_SpecialName           = 0x00000400,
        E_TA_Import                = 0x00001000,
        E_TA_Serializable          = 0x00002000
    };

    enum E_MethodAttribute : unsigned
    {
        E_MA_MemberAccessMask = 0x0007,
        E_MA_Private          = 0x0001,
        E_MA_FamAndAssem      = 0x0002,
        E_MA_Assem            = 0x0003,
        E_MA_Family           = 0x0004,
        E_MA_FamOrAssem       = 0x0005,
        E_MA_Public           = 0x0006,
        E_MA_Static           = 0x0010,
        E_MA_Final            = 0x0020,
        E_MA_Virtual          = 0x0040,
        E_MA_HideBySig        = 0x0080,
        E_MA_Abstract         = 0x0400,
        E_MA_SpecialName      = 0x0800
    };

    enum E_FieldAttribute : unsigned
    {
        E_FA_FieldAccessMask = 0x0007,
        E_FA_PrivateScope    = 0x0000,
        E_FA_Private         = 0x0001,
        E_FA_FamAndAssem     = 0x0002,
        E_FA_Assembly        = 0x0003,
        E_FA_Family          = 0x0004,
        E_FA_FamOrAssem      = 0x0005,
        E_FA_Public          = 0x0006,
        E_FA_Static          = 0x0010,
        E_FA_InitOnly        = 0x0020,
        E_FA_Literal         = 0x0040,
        E_FA_NotSerialized   = 0x0080,
        E_FA_SpecialName     = 0x0200
    };
}
