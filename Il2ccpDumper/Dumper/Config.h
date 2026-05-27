//
//  Config.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

namespace Il2Dumper
{
    struct SDumpConfig
    {
        // Per-section toggles for dump.cs.
        bool bDumpMethod       { true };
        bool bDumpField        { true };
        bool bDumpFieldOffset  { true };
        bool bDumpMethodOffset { true };
        bool bDumpTypeDefIndex { true };

        // Optional output files alongside dump.cs.
        bool bGenerateStruct    { true };
        bool bGenerateScript    { true };
        bool bDumpStringLiteral { true };

        // Empty -> Documents/Il2cppDump/.
        std::string m_OutputDir;
    };

    struct SDumpSources
    {
        // File-mode paths.
        std::string m_Il2cppPath;
        std::string m_MetadataPath;

        /* Memory-mode pointers; when non-null take precedence over the
         matching path field */
        const void* p_Il2cppMemory   { nullptr };
        size_t      m_Il2cppSize     { 0 };
        const void* p_MetadataMemory { nullptr };
        size_t      m_MetadataSize   { 0 };

        /* Live-image base address (= dyld load address of the il2cpp slice).
         Non-zero only in memory mode; used as the slide for the Mach-O
         pointer-scrubber */
        uint64_t    m_ImageBase      { 0 };
    };
}
