//
//  Il2CppTypes.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "Il2CppTypes.h"

namespace Il2Dumper
{
    void SIl2CppCodeRegistration::Read(CBinaryStream& s)
    {
    #ifdef IL2CPP_VER_LE_24_1
        m_MethodPointersCount = s.ReadU64();
        m_MethodPointers      = s.ReadU64();
    #endif
    #if !defined(IL2CPP_VER_GE_22)
        m_DelegateWrappersFromNativeToManagedCount = s.ReadU64();
        m_DelegateWrappersFromNativeToManaged      = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_GE_22
        m_ReversePInvokeWrapperCount = s.ReadU64();
        m_ReversePInvokeWrappers     = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_LE_22
        m_DelegateWrappersFromManagedToNativeCount = s.ReadU64();
        m_DelegateWrappersFromManagedToNative      = s.ReadU64();
        m_MarshalingFunctionsCount                 = s.ReadU64();
        m_MarshalingFunctions                      = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        m_CcwMarshalingFunctionsCount = s.ReadU64();
        m_CcwMarshalingFunctions      = s.ReadU64();
    #endif
        m_GenericMethodPointersCount = s.ReadU64();
        m_GenericMethodPointers      = s.ReadU64();
    #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        m_GenericAdjustorThunks = s.ReadU64();
    #endif
        m_InvokerPointersCount = s.ReadU64();
        m_InvokerPointers      = s.ReadU64();
    #ifdef IL2CPP_VER_LE_24_5
        m_CustomAttributeCount      = s.ReadU64();
        m_CustomAttributeGenerators = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        m_GuidCount = s.ReadU64();
        m_Guids     = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_GE_22
        m_UnresolvedVirtualCallCount    = s.ReadU64();
        m_UnresolvedVirtualCallPointers = s.ReadU64();
    #endif
    #if defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        m_UnresolvedInstanceCallPointers = s.ReadU64();
        m_UnresolvedStaticCallPointers   = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_GE_23
        m_InteropDataCount = s.ReadU64();
        m_InteropData      = s.ReadU64();
    #endif
    #if defined(IL2CPP_VER_24_3) || defined(IL2CPP_VER_24_4) || defined(IL2CPP_VER_24_5) \
     || defined(IL2CPP_VER_27)   || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        m_WindowsRuntimeFactoryCount = s.ReadU64();
        m_WindowsRuntimeFactoryTable = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_GE_24_2
        m_CodeGenModulesCount = s.ReadU64();
        m_CodeGenModules      = s.ReadU64();
    #endif
    }

    void SIl2CppMetadataRegistration::Read(CBinaryStream& s)
    {
        m_GenericClassesCount     = s.ReadI64();
        m_GenericClasses          = s.ReadU64();
        m_GenericInstsCount       = s.ReadI64();
        m_GenericInsts            = s.ReadU64();
        m_GenericMethodTableCount = s.ReadI64();
        m_GenericMethodTable      = s.ReadU64();
        m_TypesCount              = s.ReadI64();
        m_Types                   = s.ReadU64();
        m_MethodSpecsCount        = s.ReadI64();
        m_MethodSpecs             = s.ReadU64();
    #if !defined(IL2CPP_VER_GE_19)
        m_MethodReferencesCount = s.ReadI64();
        m_MethodReferences      = s.ReadU64();
    #endif
        m_FieldOffsetsCount         = s.ReadI64();
        m_FieldOffsets              = s.ReadU64();
        m_TypeDefinitionsSizesCount = s.ReadI64();
        m_TypeDefinitionsSizes      = s.ReadU64();
    #ifdef IL2CPP_VER_GE_19
        m_MetadataUsagesCount = s.ReadU64();
        m_MetadataUsages      = s.ReadU64();
    #endif
    }

    void SIl2CppGenericClass::Read(CBinaryStream& s)
    {
    #ifdef IL2CPP_VER_LE_24_5
        m_TypeDefinitionIndex = s.ReadI64();
    #endif
    #ifdef IL2CPP_VER_GE_27
        m_Type = s.ReadU64();
    #endif
        m_Context.Read(s);
        m_CachedClass = s.ReadU64();
    }

    void SIl2CppArrayType::Read(CBinaryStream& s)
    {
        m_EType       = s.ReadU64();
        m_Rank        = s.ReadU8();
        m_NumSizes    = s.ReadU8();
        m_NumLoBounds = s.ReadU8();
        // The on-disk layout has padding back up to 8-byte alignment.
        s.SeekTo((s.Position() + 7) & ~uint64_t(7));
        m_Sizes       = s.ReadU64();
        m_LoBounds    = s.ReadU64();
    }

    void SIl2CppCodeGenModule::Read(CBinaryStream& s)
    {
        m_ModuleName         = s.ReadU64();
        m_MethodPointerCount = s.ReadI64();
        m_MethodPointers     = s.ReadU64();
    #if defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2) \
     || defined(IL2CPP_VER_29)   || defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_31)
        m_AdjustorThunkCount = s.ReadI64();
        m_AdjustorThunks     = s.ReadU64();
    #endif
        m_InvokerIndices               = s.ReadU64();
        m_ReversePInvokeWrapperCount   = s.ReadU64();
        m_ReversePInvokeWrapperIndices = s.ReadU64();
        m_RgctxRangesCount             = s.ReadI64();
        m_RgctxRanges                  = s.ReadU64();
        m_RgctxsCount                  = s.ReadI64();
        m_Rgctxs                       = s.ReadU64();
        m_DebuggerMetadata             = s.ReadU64();
    #if defined(IL2CPP_VER_27) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2)
        m_CustomAttributeCacheGenerator = s.ReadU64();
    #endif
    #ifdef IL2CPP_VER_GE_27
        m_ModuleInitializer            = s.ReadU64();
        m_StaticConstructorTypeIndices = s.ReadU64();
        m_MetadataRegistration         = s.ReadU64();
        m_CodeRegistration             = s.ReadU64();
    #endif
    }
}
