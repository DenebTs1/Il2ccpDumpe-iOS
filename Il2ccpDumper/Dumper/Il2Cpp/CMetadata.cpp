//
//  CMetadata.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CMetadata.h"

#include <cstring>

namespace Il2Dumper
{
    CMetadata::CMetadata(const IDataSource& m_Src)
        : m_S_(m_Src)
    {
    }

    E_Status CMetadata::Init()
    {
        if (m_S_.Length() < 8) return (m_Status_ = E_Status::E_IoFailure);

        uint32_t m_Sanity = m_S_.ReadAt<uint32_t>(0);
        int32_t  m_RawVer = m_S_.ReadI32();
        if (m_Sanity != k_MetadataSanity) return (m_Status_ = E_Status::E_BadSanity);
        if (m_RawVer != int(IL2CPP_TARGET_VERSION))
        {
            return (m_Status_ = E_Status::E_UnsupportedMetaVersion);
        }

        m_S_.SeekTo(0);
        m_Header_.Read(m_S_);

        ReadArray(m_ImageDefs_,         m_Header_.m_ImagesOffset,             m_Header_.m_ImagesSize);
        ReadArray(m_TypeDefs_,          m_Header_.m_TypeDefinitionsOffset,    m_Header_.m_TypeDefinitionsSize);
        ReadArray(m_MethodDefs_,        m_Header_.m_MethodsOffset,            m_Header_.m_MethodsSize);
        ReadArray(m_ParameterDefs_,     m_Header_.m_ParametersOffset,         m_Header_.m_ParametersSize);
        ReadArray(m_FieldDefs_,         m_Header_.m_FieldsOffset,             m_Header_.m_FieldsSize);
        ReadArray(m_GenericParameters_, m_Header_.m_GenericParametersOffset,  m_Header_.m_GenericParametersSize);
        ReadArray(m_StringLiterals_,    m_Header_.m_StringLiteralOffset,      m_Header_.m_StringLiteralSize);

    #if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
        ReadArray(m_MetadataUsageLists_, m_Header_.m_MetadataUsageListsOffset, m_Header_.m_MetadataUsageListsCount);
        ReadArray(m_MetadataUsagePairs_, m_Header_.m_MetadataUsagePairsOffset, m_Header_.m_MetadataUsagePairsCount);
        ProcessMetadataUsage();
    #endif

        m_Status_ = E_Status::E_Ok;
        return m_Status_;
    }

#if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
    void CMetadata::ProcessMetadataUsage()
    {
        uint32_t m_Max = 0;
        for (const auto& p : m_MetadataUsagePairs_)
        {
            if (p.m_DestinationIndex > m_Max) m_Max = p.m_DestinationIndex;
        }
        m_MetadataUsagesCount_ = int64_t(m_Max) + 1;
    }
#endif

    std::string CMetadata::GetStringFromIndex(uint32_t m_Index)
    {
        auto it = m_StringCache_.find(m_Index);
        if (it != m_StringCache_.end()) return it->second;
        std::string m_Str = m_S_.ReadStringToNull(uint64_t(m_Header_.m_StringOffset) + m_Index);
        m_StringCache_[m_Index] = m_Str;
        return m_Str;
    }

    std::string CMetadata::GetStringLiteralFromIndex(uint32_t m_Index)
    {
        if (m_Index >= m_StringLiterals_.size()) return {};
        const SStringLiteral& m_Lit = m_StringLiterals_[m_Index];
        uint64_t m_Off = uint64_t(m_Header_.m_StringLiteralDataOffset) + uint64_t(m_Lit.m_DataIndex);
        std::string m_Out;
        m_Out.resize(m_Lit.m_Length);
        if (m_Off + m_Lit.m_Length <= m_S_.Length())
        {
            std::memcpy(&m_Out[0], m_S_.Bytes() + m_Off, m_Lit.m_Length);
        }
        return m_Out;
    }
}
