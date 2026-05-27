//
//  MetadataTypes.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "MetadataTypes.h"
#include "Versions.h"

namespace Il2Dumper
{
    void SGlobalMetadataHeader::Read(CBinaryStream& s)
    {
        m_Sanity                                  = s.ReadU32();
        m_Version                                 = s.ReadI32();
        m_StringLiteralOffset                     = s.ReadU32();
        m_StringLiteralSize                       = s.ReadI32();
        m_StringLiteralDataOffset                 = s.ReadU32();
        m_StringLiteralDataSize                   = s.ReadI32();
        m_StringOffset                            = s.ReadU32();
        m_StringSize                              = s.ReadI32();
        m_EventsOffset                            = s.ReadU32();
        m_EventsSize                              = s.ReadI32();
        m_PropertiesOffset                        = s.ReadU32();
        m_PropertiesSize                          = s.ReadI32();
        m_MethodsOffset                           = s.ReadU32();
        m_MethodsSize                             = s.ReadI32();
        m_ParameterDefaultValuesOffset            = s.ReadU32();
        m_ParameterDefaultValuesSize              = s.ReadI32();
        m_FieldDefaultValuesOffset                = s.ReadU32();
        m_FieldDefaultValuesSize                  = s.ReadI32();
        m_FieldAndParameterDefaultValueDataOffset = s.ReadU32();
        m_FieldAndParameterDefaultValueDataSize   = s.ReadI32();
        m_FieldMarshaledSizesOffset               = s.ReadI32();
        m_FieldMarshaledSizesSize                 = s.ReadI32();
        m_ParametersOffset                        = s.ReadU32();
        m_ParametersSize                          = s.ReadI32();
        m_FieldsOffset                            = s.ReadU32();
        m_FieldsSize                              = s.ReadI32();
        m_GenericParametersOffset                 = s.ReadU32();
        m_GenericParametersSize                   = s.ReadI32();
        m_GenericParameterConstraintsOffset       = s.ReadU32();
        m_GenericParameterConstraintsSize         = s.ReadI32();
        m_GenericContainersOffset                 = s.ReadU32();
        m_GenericContainersSize                   = s.ReadI32();
        m_NestedTypesOffset                       = s.ReadU32();
        m_NestedTypesSize                         = s.ReadI32();
        m_InterfacesOffset                        = s.ReadU32();
        m_InterfacesSize                          = s.ReadI32();
        m_VtableMethodsOffset                     = s.ReadU32();
        m_VtableMethodsSize                       = s.ReadI32();
        m_InterfaceOffsetsOffset                  = s.ReadI32();
        m_InterfaceOffsetsSize                    = s.ReadI32();
        m_TypeDefinitionsOffset                   = s.ReadU32();
        m_TypeDefinitionsSize                     = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24_1
        m_RgctxEntriesOffset = s.ReadU32();
        m_RgctxEntriesCount  = s.ReadI32();
    #endif
        m_ImagesOffset     = s.ReadU32();
        m_ImagesSize       = s.ReadI32();
        m_AssembliesOffset = s.ReadU32();
        m_AssembliesSize   = s.ReadI32();
    #if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
        m_MetadataUsageListsOffset = s.ReadU32();
        m_MetadataUsageListsCount  = s.ReadI32();
        m_MetadataUsagePairsOffset = s.ReadU32();
        m_MetadataUsagePairsCount  = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_19
        m_FieldRefsOffset = s.ReadU32();
        m_FieldRefsSize   = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_20
        m_ReferencedAssembliesOffset = s.ReadI32();
        m_ReferencedAssembliesSize   = s.ReadI32();
    #endif
    #if defined(IL2CPP_VER_GE_21) && defined(IL2CPP_VER_LE_27_2)
        m_AttributesInfoOffset = s.ReadU32();
        m_AttributesInfoCount  = s.ReadI32();
        m_AttributeTypesOffset = s.ReadU32();
        m_AttributeTypesCount  = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_29
        m_AttributeDataOffset      = s.ReadU32();
        m_AttributeDataSize        = s.ReadI32();
        m_AttributeDataRangeOffset = s.ReadU32();
        m_AttributeDataRangeSize   = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_22
        m_UnresolvedVirtualCallParameterTypesOffset  = s.ReadI32();
        m_UnresolvedVirtualCallParameterTypesSize    = s.ReadI32();
        m_UnresolvedVirtualCallParameterRangesOffset = s.ReadI32();
        m_UnresolvedVirtualCallParameterRangesSize   = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_23
        m_WindowsRuntimeTypeNamesOffset = s.ReadI32();
        m_WindowsRuntimeTypeNamesSize   = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_27
        m_WindowsRuntimeStringsOffset = s.ReadI32();
        m_WindowsRuntimeStringsSize   = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_24
        m_ExportedTypeDefinitionsOffset = s.ReadI32();
        m_ExportedTypeDefinitionsSize   = s.ReadI32();
    #endif
    }

    void SImageDefinition::Read(CBinaryStream& s)
    {
        m_NameIndex     = s.ReadU32();
        m_AssemblyIndex = s.ReadI32();
        m_TypeStart     = s.ReadI32();
        m_TypeCount     = s.ReadU32();
    #ifdef IL2CPP_VER_GE_24
        m_ExportedTypeStart = s.ReadI32();
        m_ExportedTypeCount = s.ReadU32();
    #endif
        m_EntryPointIndex = s.ReadI32();
    #ifdef IL2CPP_VER_GE_19
        m_Token = s.ReadU32();
    #endif
    #ifdef IL2CPP_VER_GE_24_1
        m_CustomAttributeStart = s.ReadI32();
        m_CustomAttributeCount = s.ReadU32();
    #endif
    }

    void STypeDefinition::Read(CBinaryStream& s)
    {
        m_NameIndex      = s.ReadU32();
        m_NamespaceIndex = s.ReadU32();
    #ifdef IL2CPP_VER_LE_24
        m_CustomAttributeIndex = s.ReadI32();
    #endif
        m_ByvalTypeIndex = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24_5
        m_ByrefTypeIndex = s.ReadI32();
    #endif
        m_DeclaringTypeIndex = s.ReadI32();
        m_ParentIndex        = s.ReadI32();
        m_ElementTypeIndex   = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24_1
        m_RgctxStartIndex = s.ReadI32();
        m_RgctxCount      = s.ReadI32();
    #endif
        m_GenericContainerIndex = s.ReadI32();
    #ifdef IL2CPP_VER_LE_22
        m_DelegateWrapperFromManagedToNativeIndex = s.ReadI32();
        m_MarshalingFunctionsIndex                = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_IN_21_22
        m_CcwFunctionIndex = s.ReadI32();
        m_GuidIndex        = s.ReadI32();
    #endif
        m_Flags                 = s.ReadU32();
        m_FieldStart            = s.ReadI32();
        m_MethodStart           = s.ReadI32();
        m_EventStart            = s.ReadI32();
        m_PropertyStart         = s.ReadI32();
        m_NestedTypesStart      = s.ReadI32();
        m_InterfacesStart       = s.ReadI32();
        m_VtableStart           = s.ReadI32();
        m_InterfaceOffsetsStart = s.ReadI32();
        m_MethodCount           = s.ReadU16();
        m_PropertyCount         = s.ReadU16();
        m_FieldCount            = s.ReadU16();
        m_EventCount            = s.ReadU16();
        m_NestedTypeCount       = s.ReadU16();
        m_VtableCount           = s.ReadU16();
        m_InterfacesCount       = s.ReadU16();
        m_InterfaceOffsetsCount = s.ReadU16();
        m_Bitfield              = s.ReadU32();
    #ifdef IL2CPP_VER_GE_19
        m_Token = s.ReadU32();
    #endif
    }

    void SMethodDefinition::Read(CBinaryStream& s)
    {
        m_NameIndex     = s.ReadU32();
        m_DeclaringType = s.ReadI32();
        m_ReturnType    = s.ReadI32();
    #ifdef IL2CPP_VER_GE_31
        m_ReturnParameterToken = s.ReadI32();
    #endif
        m_ParameterStart = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24
        m_CustomAttributeIndex = s.ReadI32();
    #endif
        m_GenericContainerIndex = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24_1
        m_MethodIndex          = s.ReadI32();
        m_InvokerIndex         = s.ReadI32();
        m_DelegateWrapperIndex = s.ReadI32();
        m_RgctxStartIndex      = s.ReadI32();
        m_RgctxCount           = s.ReadI32();
    #endif
        m_Token          = s.ReadU32();
        m_Flags          = s.ReadU16();
        m_IFlags         = s.ReadU16();
        m_Slot           = s.ReadU16();
        m_ParameterCount = s.ReadU16();
    }

    void SParameterDefinition::Read(CBinaryStream& s)
    {
        m_NameIndex = s.ReadU32();
        m_Token     = s.ReadU32();
    #ifdef IL2CPP_VER_LE_24
        m_CustomAttributeIndex = s.ReadI32();
    #endif
        m_TypeIndex = s.ReadI32();
    }

    void SFieldDefinition::Read(CBinaryStream& s)
    {
        m_NameIndex = s.ReadU32();
        m_TypeIndex = s.ReadI32();
    #ifdef IL2CPP_VER_LE_24
        m_CustomAttributeIndex = s.ReadI32();
    #endif
    #ifdef IL2CPP_VER_GE_19
        m_Token = s.ReadU32();
    #endif
    }

    void SFieldDefaultValue::Read(CBinaryStream& s)
    {
        m_FieldIndex = s.ReadI32();
        m_TypeIndex  = s.ReadI32();
        m_DataIndex  = s.ReadI32();
    }

    void SParameterDefaultValue::Read(CBinaryStream& s)
    {
        m_ParameterIndex = s.ReadI32();
        m_TypeIndex      = s.ReadI32();
        m_DataIndex      = s.ReadI32();
    }

#if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
    void SMetadataUsageList::Read(CBinaryStream& s)
    {
        m_Start = s.ReadU32();
        m_Count = s.ReadU32();
    }

    void SMetadataUsagePair::Read(CBinaryStream& s)
    {
        m_DestinationIndex   = s.ReadU32();
        m_EncodedSourceIndex = s.ReadU32();
    }
#endif

    void SStringLiteral::Read(CBinaryStream& s)
    {
        m_Length    = s.ReadU32();
        m_DataIndex = s.ReadI32();
    }

    void SGenericParameter::Read(CBinaryStream& s)
    {
        m_OwnerIndex       = s.ReadI32();
        m_NameIndex        = s.ReadU32();
        m_ConstraintsStart = s.ReadI16();
        m_ConstraintsCount = s.ReadI16();
        m_Num              = s.ReadU16();
        m_Flags            = s.ReadU16();
    }
}
