//
//  MetadataTypes.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//
//  Metadata record layouts, version-selected at compile time via the
//  IL2CPP_VER_GE_* / IL2CPP_VER_LE_* flags in Versions.h. Only the structs
//  the dumper actually consumes live here; anything the outputs never read
//  has been purged.
//

#pragma once

#include "Versions.h"
#include "../IO/CBinaryStream.h"

#include <cstdint>

namespace Il2Dumper
{
    struct SGlobalMetadataHeader
    {
        uint32_t m_Sanity;
        int32_t  m_Version;
        uint32_t m_StringLiteralOffset;
        int32_t  m_StringLiteralSize;
        uint32_t m_StringLiteralDataOffset;
        int32_t  m_StringLiteralDataSize;
        uint32_t m_StringOffset;
        int32_t  m_StringSize;
        uint32_t m_EventsOffset;
        int32_t  m_EventsSize;
        uint32_t m_PropertiesOffset;
        int32_t  m_PropertiesSize;
        uint32_t m_MethodsOffset;
        int32_t  m_MethodsSize;
        uint32_t m_ParameterDefaultValuesOffset;
        int32_t  m_ParameterDefaultValuesSize;
        uint32_t m_FieldDefaultValuesOffset;
        int32_t  m_FieldDefaultValuesSize;
        uint32_t m_FieldAndParameterDefaultValueDataOffset;
        int32_t  m_FieldAndParameterDefaultValueDataSize;
        int32_t  m_FieldMarshaledSizesOffset;
        int32_t  m_FieldMarshaledSizesSize;
        uint32_t m_ParametersOffset;
        int32_t  m_ParametersSize;
        uint32_t m_FieldsOffset;
        int32_t  m_FieldsSize;
        uint32_t m_GenericParametersOffset;
        int32_t  m_GenericParametersSize;
        uint32_t m_GenericParameterConstraintsOffset;
        int32_t  m_GenericParameterConstraintsSize;
        uint32_t m_GenericContainersOffset;
        int32_t  m_GenericContainersSize;
        uint32_t m_NestedTypesOffset;
        int32_t  m_NestedTypesSize;
        uint32_t m_InterfacesOffset;
        int32_t  m_InterfacesSize;
        uint32_t m_VtableMethodsOffset;
        int32_t  m_VtableMethodsSize;
        int32_t  m_InterfaceOffsetsOffset;
        int32_t  m_InterfaceOffsetsSize;
        uint32_t m_TypeDefinitionsOffset;
        int32_t  m_TypeDefinitionsSize;
    #ifdef IL2CPP_VER_LE_24_1
        uint32_t m_RgctxEntriesOffset;
        int32_t  m_RgctxEntriesCount;
    #endif
        uint32_t m_ImagesOffset;
        int32_t  m_ImagesSize;
        uint32_t m_AssembliesOffset;
        int32_t  m_AssembliesSize;
    #if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
        uint32_t m_MetadataUsageListsOffset;
        int32_t  m_MetadataUsageListsCount;
        uint32_t m_MetadataUsagePairsOffset;
        int32_t  m_MetadataUsagePairsCount;
    #endif
    #ifdef IL2CPP_VER_GE_19
        uint32_t m_FieldRefsOffset;
        int32_t  m_FieldRefsSize;
    #endif
    #ifdef IL2CPP_VER_GE_20
        int32_t  m_ReferencedAssembliesOffset;
        int32_t  m_ReferencedAssembliesSize;
    #endif
    #if defined(IL2CPP_VER_GE_21) && defined(IL2CPP_VER_LE_27_2)
        uint32_t m_AttributesInfoOffset;
        int32_t  m_AttributesInfoCount;
        uint32_t m_AttributeTypesOffset;
        int32_t  m_AttributeTypesCount;
    #endif
    #ifdef IL2CPP_VER_GE_29
        uint32_t m_AttributeDataOffset;
        int32_t  m_AttributeDataSize;
        uint32_t m_AttributeDataRangeOffset;
        int32_t  m_AttributeDataRangeSize;
    #endif
    #ifdef IL2CPP_VER_GE_22
        int32_t  m_UnresolvedVirtualCallParameterTypesOffset;
        int32_t  m_UnresolvedVirtualCallParameterTypesSize;
        int32_t  m_UnresolvedVirtualCallParameterRangesOffset;
        int32_t  m_UnresolvedVirtualCallParameterRangesSize;
    #endif
    #ifdef IL2CPP_VER_GE_23
        int32_t  m_WindowsRuntimeTypeNamesOffset;
        int32_t  m_WindowsRuntimeTypeNamesSize;
    #endif
    #ifdef IL2CPP_VER_GE_27
        int32_t  m_WindowsRuntimeStringsOffset;
        int32_t  m_WindowsRuntimeStringsSize;
    #endif
    #ifdef IL2CPP_VER_GE_24
        int32_t  m_ExportedTypeDefinitionsOffset;
        int32_t  m_ExportedTypeDefinitionsSize;
    #endif

        void Read(CBinaryStream& s);
    };

    struct SImageDefinition
    {
        uint32_t m_NameIndex;
        int32_t  m_AssemblyIndex;
        int32_t  m_TypeStart;
        uint32_t m_TypeCount;
    #ifdef IL2CPP_VER_GE_24
        int32_t  m_ExportedTypeStart;
        uint32_t m_ExportedTypeCount;
    #endif
        int32_t  m_EntryPointIndex;
    #ifdef IL2CPP_VER_GE_19
        uint32_t m_Token;
    #endif
    #ifdef IL2CPP_VER_GE_24_1
        int32_t  m_CustomAttributeStart;
        uint32_t m_CustomAttributeCount;
    #endif

        void Read(CBinaryStream& s);
    };

    struct STypeDefinition
    {
        uint32_t m_NameIndex;
        uint32_t m_NamespaceIndex;
    #ifdef IL2CPP_VER_LE_24
        int32_t  m_CustomAttributeIndex;
    #endif
        int32_t  m_ByvalTypeIndex;
    #ifdef IL2CPP_VER_LE_24_5
        int32_t  m_ByrefTypeIndex;
    #endif
        int32_t  m_DeclaringTypeIndex;
        int32_t  m_ParentIndex;
        int32_t  m_ElementTypeIndex;
    #ifdef IL2CPP_VER_LE_24_1
        int32_t  m_RgctxStartIndex;
        int32_t  m_RgctxCount;
    #endif
        int32_t  m_GenericContainerIndex;
    #ifdef IL2CPP_VER_LE_22
        int32_t  m_DelegateWrapperFromManagedToNativeIndex;
        int32_t  m_MarshalingFunctionsIndex;
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        int32_t  m_CcwFunctionIndex;
        int32_t  m_GuidIndex;
    #endif
        uint32_t m_Flags;
        int32_t  m_FieldStart;
        int32_t  m_MethodStart;
        int32_t  m_EventStart;
        int32_t  m_PropertyStart;
        int32_t  m_NestedTypesStart;
        int32_t  m_InterfacesStart;
        int32_t  m_VtableStart;
        int32_t  m_InterfaceOffsetsStart;
        uint16_t m_MethodCount;
        uint16_t m_PropertyCount;
        uint16_t m_FieldCount;
        uint16_t m_EventCount;
        uint16_t m_NestedTypeCount;
        uint16_t m_VtableCount;
        uint16_t m_InterfacesCount;
        uint16_t m_InterfaceOffsetsCount;
        uint32_t m_Bitfield;
    #ifdef IL2CPP_VER_GE_19
        uint32_t m_Token;
    #endif

        bool IsValueType() const { return (m_Bitfield & 0x1) != 0; }
        bool IsEnum     () const { return ((m_Bitfield >> 1) & 0x1) != 0; }

        void Read(CBinaryStream& s);
    };

    struct SMethodDefinition
    {
        uint32_t m_NameIndex;
        int32_t  m_DeclaringType;
        int32_t  m_ReturnType;
    #ifdef IL2CPP_VER_GE_31
        int32_t  m_ReturnParameterToken;
    #endif
        int32_t  m_ParameterStart;
    #ifdef IL2CPP_VER_LE_24
        int32_t  m_CustomAttributeIndex;
    #endif
        int32_t  m_GenericContainerIndex;
    #ifdef IL2CPP_VER_LE_24_1
        int32_t  m_MethodIndex;
        int32_t  m_InvokerIndex;
        int32_t  m_DelegateWrapperIndex;
        int32_t  m_RgctxStartIndex;
        int32_t  m_RgctxCount;
    #endif
        uint32_t m_Token;
        uint16_t m_Flags;
        uint16_t m_IFlags;
        uint16_t m_Slot;
        uint16_t m_ParameterCount;

        void Read(CBinaryStream& s);
    };

    struct SParameterDefinition
    {
        uint32_t m_NameIndex;
        uint32_t m_Token;
    #ifdef IL2CPP_VER_LE_24
        int32_t  m_CustomAttributeIndex;
    #endif
        int32_t  m_TypeIndex;

        void Read(CBinaryStream& s);
    };

    struct SFieldDefinition
    {
        uint32_t m_NameIndex;
        int32_t  m_TypeIndex;
    #ifdef IL2CPP_VER_LE_24
        int32_t  m_CustomAttributeIndex;
    #endif
    #ifdef IL2CPP_VER_GE_19
        uint32_t m_Token;
    #endif

        void Read(CBinaryStream& s);
    };

    struct SFieldDefaultValue
    {
        int32_t m_FieldIndex;
        int32_t m_TypeIndex;
        int32_t m_DataIndex;
        void Read(CBinaryStream& s);
    };

    struct SParameterDefaultValue
    {
        int32_t m_ParameterIndex;
        int32_t m_TypeIndex;
        int32_t m_DataIndex;
        void Read(CBinaryStream& s);
    };

    // Used internally by CMetadata::ProcessMetadataUsage on v19..v24.5.
#if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
    struct SMetadataUsageList
    {
        uint32_t m_Start;
        uint32_t m_Count;
        void Read(CBinaryStream& s);
    };

    struct SMetadataUsagePair
    {
        uint32_t m_DestinationIndex;
        uint32_t m_EncodedSourceIndex;
        void Read(CBinaryStream& s);
    };
#endif

    struct SStringLiteral
    {
        uint32_t m_Length;
        int32_t  m_DataIndex;
        void Read(CBinaryStream& s);
    };

    struct SGenericParameter
    {
        int32_t  m_OwnerIndex;
        uint32_t m_NameIndex;
        int16_t  m_ConstraintsStart;
        int16_t  m_ConstraintsCount;
        uint16_t m_Num;
        uint16_t m_Flags;
        void Read(CBinaryStream& s);
    };
}
