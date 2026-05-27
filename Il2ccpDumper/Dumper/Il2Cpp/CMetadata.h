//
//  CMetadata.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "Versions.h"
#include "MetadataTypes.h"
#include "../Core/Types.h"
#include "../IO/IDataSource.h"
#include "../IO/CBinaryStream.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Il2Dumper
{
    class CMetadata
    {
    public:
        explicit CMetadata(const IDataSource& m_Src);

        E_Status Init();
        bool     Ok() const { return m_Status_ == E_Status::E_Ok; }

        // Compile-time target Il2Cpp version (selected via IL2CPP_VER_X macro).
        static constexpr double Version() { return IL2CPP_TARGET_VERSION; }

        int64_t  MetadataUsagesCount() const { return m_MetadataUsagesCount_; }

        const std::vector<SImageDefinition>&     ImageDefs()        const { return m_ImageDefs_;        }
        const std::vector<STypeDefinition>&      TypeDefs()         const { return m_TypeDefs_;         }
        const std::vector<SMethodDefinition>&    MethodDefs()       const { return m_MethodDefs_;       }
        const std::vector<SParameterDefinition>& ParameterDefs()    const { return m_ParameterDefs_;    }
        const std::vector<SFieldDefinition>&     FieldDefs()        const { return m_FieldDefs_;        }
        const std::vector<SGenericParameter>&    GenericParameters() const { return m_GenericParameters_; }
        const std::vector<SStringLiteral>&       StringLiterals()    const { return m_StringLiterals_; }

        std::string GetStringFromIndex(uint32_t m_Index);
        std::string GetStringLiteralFromIndex(uint32_t m_Index);

        CBinaryStream& Stream() { return m_S_; }

    private:
    #if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
        void ProcessMetadataUsage();
    #endif

        template <typename T>
        void ReadArray(std::vector<T>& m_Out, uint64_t m_Off, int m_TotalBytes)
        {
            if (m_TotalBytes <= 0) return;
            int m_Count = m_TotalBytes / int(sizeof(T));
            m_Out.resize(size_t(m_Count));
            m_S_.SeekTo(m_Off);
            for (int i = 0; i < m_Count; ++i) m_Out[i].Read(m_S_);
        }

        template <typename T>
        void ReadPrimArray(std::vector<T>& m_Out, uint64_t m_Off, int m_TotalBytes)
        {
            if (m_TotalBytes <= 0) return;
            int m_Count = m_TotalBytes / int(sizeof(T));
            m_Out.resize(size_t(m_Count));
            m_S_.SeekTo(m_Off);
            for (int i = 0; i < m_Count; ++i) m_Out[i] = m_S_.Read<T>();
        }

        CBinaryStream         m_S_;
        SGlobalMetadataHeader m_Header_ {};
        int64_t               m_MetadataUsagesCount_ { 0 };
        E_Status              m_Status_              { E_Status::E_NotInitialised };

        std::vector<SImageDefinition>     m_ImageDefs_;
        std::vector<STypeDefinition>      m_TypeDefs_;
        std::vector<SMethodDefinition>    m_MethodDefs_;
        std::vector<SParameterDefinition> m_ParameterDefs_;
        std::vector<SFieldDefinition>     m_FieldDefs_;
        std::vector<SGenericParameter>    m_GenericParameters_;
        std::vector<SStringLiteral>       m_StringLiterals_;
    #if defined(IL2CPP_VER_GE_19) && defined(IL2CPP_VER_LE_24_5)
        std::vector<SMetadataUsageList>   m_MetadataUsageLists_;
        std::vector<SMetadataUsagePair>   m_MetadataUsagePairs_;
    #endif

        std::unordered_map<uint32_t, std::string> m_StringCache_;
    };
}
