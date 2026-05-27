//
//  CDecompiler.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//


#pragma once

#include "../Utils/CIl2CppExecutor.h"
#include "../Config.h"

#include <string>

namespace Il2Dumper
{
    /*Writes dump.cs, the human-readable per-namespace breakdown of all
     managed types with their fields, methods, offsets and tokens. The
     output is informational; it is NOT valid C# (lots of corners are
     approximated, generics in particular) */

    class CDecompiler
    {
    public:
        explicit CDecompiler(CIl2CppExecutor* p_Exec) : p_Exec_(p_Exec) {}

        bool Decompile(const SDumpConfig& m_Cfg, const std::string& m_OutDir);

    private:
        std::string VisibilityForType(uint32_t m_Flags) const;
        std::string VisibilityForMember(uint32_t m_Flags) const;

        CIl2CppExecutor* p_Exec_;
    };
}
