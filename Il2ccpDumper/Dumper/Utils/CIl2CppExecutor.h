//
//  CIl2CppExecutor.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "../Il2Cpp/CIl2Cpp.h"
#include "../Il2Cpp/CMetadata.h"

#include <string>
#include <unordered_map>

namespace Il2Dumper
{
    class CIl2CppExecutor
    {
    public:
        CIl2CppExecutor(CMetadata* p_Meta, CIl2Cpp* p_Il2Cpp)
            : p_Meta_(p_Meta), p_Il2Cpp_(p_Il2Cpp) {}

        CMetadata* Meta()   { return p_Meta_;   }
        CIl2Cpp*   Il2Cpp() { return p_Il2Cpp_; }

        std::string GetTypeName(const SIl2CppType& m_T, bool b_AddNamespace, bool b_Nested);
        std::string GetTypeDefName(const STypeDefinition& m_Td, bool b_AddNamespace, bool b_Nested);

    private:
        std::string TryResolveRuntimeClass(const SIl2CppType& m_T, bool b_AddNamespace) const;
        void EnsureTypeDefIndex();
        int  ResolveTypeDefIndex(uint64_t m_Data) const;

        CMetadata* p_Meta_;
        CIl2Cpp*   p_Il2Cpp_;

        bool                                bIndexBuilt_ { false };
        std::unordered_map<uint64_t, int>   m_DataToTypeDef_;
    };
}
