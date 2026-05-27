//
//  CScriptJson.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "../Utils/CIl2CppExecutor.h"

#include <string>

namespace Il2Dumper
{
    /* Emits script.json, the symbol mapping consumed by ida.py and the
     Ghidra / Binja scripts. Each entry pairs an address with a name and
     a signature (best-effort) */

    class CScriptJson
    {
    public:
        explicit CScriptJson(CIl2CppExecutor* p_Exec) : p_Exec_(p_Exec) {}

        bool Write(const std::string& m_OutDir);

    private:
        CIl2CppExecutor* p_Exec_;
    };
}
