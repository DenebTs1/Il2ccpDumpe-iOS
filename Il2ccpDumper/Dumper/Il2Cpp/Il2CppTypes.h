//
//  Il2CppTypes.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//
//  Binary-side runtime types (CodeRegistration, MetadataRegistration,
//  CodeGenModule, etc.) version-selected at compile time via IL2CPP_VER_*
//  flags from Versions.h.
//

#pragma once

#include "../IO/CBinaryStream.h"
#include "Versions.h"

#include <cstdint>

namespace Il2Dumper
{
    // Mirrors Il2CppDumper's Il2CppTypeEnum (ECMA-335 element types).
    enum class E_TypeKind : uint8_t
    {
        E_End          = 0x00,
        E_Void         = 0x01,
        E_Boolean      = 0x02,
        E_Char         = 0x03,
        E_I1           = 0x04,
        E_U1           = 0x05,
        E_I2           = 0x06,
        E_U2           = 0x07,
        E_I4           = 0x08,
        E_U4           = 0x09,
        E_I8           = 0x0a,
        E_U8           = 0x0b,
        E_R4           = 0x0c,
        E_R8           = 0x0d,
        E_String       = 0x0e,
        E_Ptr          = 0x0f,
        E_ByRef        = 0x10,
        E_ValueType    = 0x11,
        E_Class        = 0x12,
        E_Var          = 0x13,
        E_Array        = 0x14,
        E_GenericInst  = 0x15,
        E_TypedByRef   = 0x16,
        E_I            = 0x18,
        E_U            = 0x19,
        E_FnPtr        = 0x1b,
        E_Object       = 0x1c,
        E_SzArray      = 0x1d,
        E_MVar         = 0x1e,
        E_CmodReqd     = 0x1f,
        E_CmodOpt      = 0x20,
        E_Internal     = 0x21,
        E_Modifier     = 0x40,
        E_Sentinel     = 0x41,
        E_Pinned       = 0x45,
        E_Enum         = 0x55,
        E_TypeIndex    = 0xff
    };

    struct SIl2CppType
    {
        uint64_t   m_DataPoint { 0 };
        uint32_t   m_Bits      { 0 };
        // Decoded fields filled by InitDecoded() after the raw read:
        uint32_t   m_Attrs     { 0 };
        E_TypeKind m_Type      { E_TypeKind::E_End };
        uint32_t   m_NumMods   { 0 };
        uint32_t   m_ByRef     { 0 };
        uint32_t   m_Pinned    { 0 };
        uint32_t   m_ValueType { 0 };

        void Read(CBinaryStream& m_S)
        {
            m_DataPoint = m_S.ReadU64();
            m_Bits      = m_S.ReadU32();
        }

        /* The bitfield layout shifted at v27.2 (one extra bit moved from
        num_mods into a new value-type slot) */
        void InitDecoded()
        {
            m_Attrs = m_Bits & 0xFFFF;
            m_Type  = E_TypeKind((m_Bits >> 16) & 0xFF);
        #if defined(IL2CPP_VER_27_2) || defined(IL2CPP_VER_29) || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
            m_NumMods   = (m_Bits >> 24) & 0x1F;
            m_ByRef     = (m_Bits >> 29) & 1;
            m_Pinned    = (m_Bits >> 30) & 1;
            m_ValueType = (m_Bits >> 31) & 1;
        #else
            m_NumMods = (m_Bits >> 24) & 0x3F;
            m_ByRef   = (m_Bits >> 30) & 1;
            m_Pinned  = (m_Bits >> 31) & 1;
        #endif
        }

        static constexpr size_t SizeOf() { return 16; } // 8 + 4 + 4 trailing pad on disk

        int64_t  KlassIndex()            const { return int64_t(m_DataPoint); }
        uint64_t TypeHandle()            const { return m_DataPoint; }
        uint64_t ElementType()           const { return m_DataPoint; }
        int64_t  GenericParameterIndex() const { return int64_t(m_DataPoint); }
        uint64_t GenericClass()          const { return m_DataPoint; }
    };

    struct SIl2CppCodeRegistration
    {
    #ifdef IL2CPP_VER_LE_24_1
        uint64_t m_MethodPointersCount;
        uint64_t m_MethodPointers;
    #endif
    #if !defined(IL2CPP_VER_GE_22)
        uint64_t m_DelegateWrappersFromNativeToManagedCount;
        uint64_t m_DelegateWrappersFromNativeToManaged;
    #endif
    #ifdef IL2CPP_VER_GE_22
        uint64_t m_ReversePInvokeWrapperCount;
        uint64_t m_ReversePInvokeWrappers;
    #endif
    #ifdef IL2CPP_VER_LE_22
        uint64_t m_DelegateWrappersFromManagedToNativeCount;
        uint64_t m_DelegateWrappersFromManagedToNative;
        uint64_t m_MarshalingFunctionsCount;
        uint64_t m_MarshalingFunctions;
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        uint64_t m_CcwMarshalingFunctionsCount;
        uint64_t m_CcwMarshalingFunctions;
    #endif
        uint64_t m_GenericMethodPointersCount;
        uint64_t m_GenericMethodPointers;
    #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        uint64_t m_GenericAdjustorThunks;
    #endif
        uint64_t m_InvokerPointersCount;
        uint64_t m_InvokerPointers;
    #ifdef IL2CPP_VER_LE_24_5
        uint64_t m_CustomAttributeCount;
        uint64_t m_CustomAttributeGenerators;
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        uint64_t m_GuidCount;
        uint64_t m_Guids;
    #endif
    #ifdef IL2CPP_VER_GE_22
        uint64_t m_UnresolvedVirtualCallCount;
        uint64_t m_UnresolvedVirtualCallPointers;
    #endif
    #if defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        uint64_t m_UnresolvedInstanceCallPointers;
        uint64_t m_UnresolvedStaticCallPointers;
    #endif
    #ifdef IL2CPP_VER_GE_23
        uint64_t m_InteropDataCount;
        uint64_t m_InteropData;
    #endif
    #if defined(IL2CPP_VER_24_3) || defined(IL2CPP_VER_24_4) || defined(IL2CPP_VER_24_5) \
     || defined(IL2CPP_VER_27)   || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        uint64_t m_WindowsRuntimeFactoryCount;
        uint64_t m_WindowsRuntimeFactoryTable;
    #endif
    #ifdef IL2CPP_VER_GE_24_2
        uint64_t m_CodeGenModulesCount;
        uint64_t m_CodeGenModules;
    #endif

        void Read(CBinaryStream& s);
        static constexpr size_t SizeOf() { return sizeof(SIl2CppCodeRegistration); }
    };

    struct SIl2CppMetadataRegistration
    {
        int64_t  m_GenericClassesCount;
        uint64_t m_GenericClasses;
        int64_t  m_GenericInstsCount;
        uint64_t m_GenericInsts;
        int64_t  m_GenericMethodTableCount;
        uint64_t m_GenericMethodTable;
        int64_t  m_TypesCount;
        uint64_t m_Types;
        int64_t  m_MethodSpecsCount;
        uint64_t m_MethodSpecs;
    #if !defined(IL2CPP_VER_GE_19)
        int64_t  m_MethodReferencesCount;
        uint64_t m_MethodReferences;
    #endif
        int64_t  m_FieldOffsetsCount;
        uint64_t m_FieldOffsets;
        int64_t  m_TypeDefinitionsSizesCount;
        uint64_t m_TypeDefinitionsSizes;
    #ifdef IL2CPP_VER_GE_19
        uint64_t m_MetadataUsagesCount;
        uint64_t m_MetadataUsages;
    #endif

        void Read(CBinaryStream& s);
        static constexpr size_t SizeOf() { return sizeof(SIl2CppMetadataRegistration); }
    };

    struct SIl2CppCodeGenModule
    {
        uint64_t m_ModuleName;
        int64_t  m_MethodPointerCount;
        uint64_t m_MethodPointers;
    #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        int64_t  m_AdjustorThunkCount;
        uint64_t m_AdjustorThunks;
    #endif
        uint64_t m_InvokerIndices;
        uint64_t m_ReversePInvokeWrapperCount;
        uint64_t m_ReversePInvokeWrapperIndices;
        int64_t  m_RgctxRangesCount;
        uint64_t m_RgctxRanges;
        int64_t  m_RgctxsCount;
        uint64_t m_Rgctxs;
        uint64_t m_DebuggerMetadata;
    #if defined(IL2CPP_VER_27) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2)
        uint64_t m_CustomAttributeCacheGenerator;
    #endif
    #ifdef IL2CPP_VER_GE_27
        uint64_t m_ModuleInitializer;
        uint64_t m_StaticConstructorTypeIndices;
        uint64_t m_MetadataRegistration;
        uint64_t m_CodeRegistration;
    #endif

        void Read(CBinaryStream& s);
        static constexpr size_t SizeOf() { return sizeof(SIl2CppCodeGenModule); }
    };

    struct SIl2CppGenericInst
    {
        int64_t  m_TypeArgc;
        uint64_t m_TypeArgv;
        void Read(CBinaryStream& s)
        {
            m_TypeArgc = s.ReadI64();
            m_TypeArgv = s.ReadU64();
        }
        static constexpr size_t SizeOf() { return 16; }
    };

    // Pair of instantiation lists used by every generic context.
    struct SIl2CppGenericContext
    {
        uint64_t m_ClassInst;
        uint64_t m_MethodInst;
        void Read(CBinaryStream& s)
        {
            m_ClassInst  = s.ReadU64();
            m_MethodInst = s.ReadU64();
        }
        static constexpr size_t SizeOf() { return 16; }
    };

    // Generic class instantiation: points at the open generic definition
    // plus the instantiation context.
    struct SIl2CppGenericClass
    {
    #ifdef IL2CPP_VER_LE_24_5
        int64_t  m_TypeDefinitionIndex;
    #endif
    #ifdef IL2CPP_VER_GE_27
        uint64_t m_Type; // VA to an SIl2CppType representing the base type
    #endif
        SIl2CppGenericContext m_Context;
        uint64_t m_CachedClass;

        void Read(CBinaryStream& s);
        static constexpr size_t SizeOf() { return sizeof(SIl2CppGenericClass); }
    };

    // Multi-dim array type (rank > 1). SzArray is a separate type kind that
    // doesn't go through this struct.
    struct SIl2CppArrayType
    {
        uint64_t m_EType;
        uint8_t  m_Rank;
        uint8_t  m_NumSizes;
        uint8_t  m_NumLoBounds;
        uint64_t m_Sizes;
        uint64_t m_LoBounds;

        void Read(CBinaryStream& s);
        static constexpr size_t SizeOf() { return sizeof(SIl2CppArrayType); }
    };

    struct SIl2CppGenericMethodIndices
    {
        int32_t m_MethodIndex;
        int32_t m_InvokerIndex;
    #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        int32_t m_AdjustorThunk;
    #endif
        void Read(CBinaryStream& s)
        {
            m_MethodIndex  = s.ReadI32();
            m_InvokerIndex = s.ReadI32();
        #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
         || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
            m_AdjustorThunk = s.ReadI32();
        #endif
        }
    };

    struct SIl2CppGenericMethodFunctionsDefinitions
    {
        int32_t                     m_GenericMethodIndex;
        SIl2CppGenericMethodIndices m_Indices;
        void Read(CBinaryStream& s)
        {
            m_GenericMethodIndex = s.ReadI32();
            m_Indices.Read(s);
        }
        static constexpr size_t SizeOf() { return sizeof(SIl2CppGenericMethodFunctionsDefinitions); }
    };

    struct SIl2CppMethodSpec
    {
        int32_t m_MethodDefinitionIndex;
        int32_t m_ClassIndexIndex;
        int32_t m_MethodIndexIndex;
        void Read(CBinaryStream& s)
        {
            m_MethodDefinitionIndex = s.ReadI32();
            m_ClassIndexIndex       = s.ReadI32();
            m_MethodIndexIndex      = s.ReadI32();
        }
        static constexpr size_t SizeOf() { return 12; }
    };

    struct SIl2CppRange
    {
        int32_t m_Start;
        int32_t m_Length;
        void Read(CBinaryStream& s) { m_Start = s.ReadI32(); m_Length = s.ReadI32(); }
        static constexpr size_t SizeOf() { return 8; }
    };

    struct SIl2CppTokenRangePair
    {
        uint32_t     m_Token;
        SIl2CppRange m_Range;
        void Read(CBinaryStream& s) { m_Token = s.ReadU32(); m_Range.Read(s); }
        static constexpr size_t SizeOf() { return 12; }
    };
}
