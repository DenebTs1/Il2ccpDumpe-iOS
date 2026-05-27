//
//  CResolverShim.h
//  Il2ccpDumper
//
//  Created by ts1 on 27/05/2026.
//


#pragma once

namespace Il2Dumper
{
    bool ResolverGetClassName(void*        p_RtIl2CppType,
                              const char** pp_OutName,
                              const char** pp_OutNamespace);
}
