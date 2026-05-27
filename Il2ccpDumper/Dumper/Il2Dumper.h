//
//  Il2Dumper.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "Core/Types.h"
#include "Config.h"

namespace Il2Dumper
{
    /* One-shot dumper entry point. Reads sources defined by m_Src,
     produces dump.cs / il2cpp.h / script.json / stringliteral.json
     under m_Cfg.OutputDir(or Documents/Il2cppDump/ by default) */

    // Returns E_Ok on success. On failure, partial output may exist on disk
    E_Status Run(const SDumpSources& m_Src, const SDumpConfig& m_Cfg);

    /* Resolves the default output directory the dumper would
     use when m_OutputDir is empty, the Documents subfolder. Caller can
     use this to display the path to the user before/after running */
    std::string DefaultOutputDir();
}
