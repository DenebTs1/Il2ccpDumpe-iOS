//
//  CStructGenerator.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "../Utils/CIl2CppExecutor.h"

#include <string>

namespace Il2Dumper
{
    /* Emits a minimal il2cpp.h with the runtime struct declarations callers
     typically import into IDA/Ghidra. This is a stripped-down port of
     StructGenerator.cs, the full attribute / generic expansion is out
     of scope; what we emit is enough for the common reverse-engineering
     workflow (named class shells, field tables) */

    class CStructGenerator
    {
    public:
        explicit CStructGenerator(CIl2CppExecutor* p_Exec) : p_Exec_(p_Exec) {}

        bool WriteHeader(const std::string& m_OutDir);

    private:
        CIl2CppExecutor* p_Exec_;
    };
}
