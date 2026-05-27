//
//  Il2Dumper.mm
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#import <Foundation/Foundation.h>

#include "Il2Dumper.h"

#include "Core/Log.h"
#include "ExecutableFormats/CMachO64.h"
#include "ExecutableFormats/CMachoFat.h"
#include "IO/CFileSource.h"
#include "IO/CMemorySource.h"
#include "Il2Cpp/CIl2Cpp.h"
#include "Il2Cpp/CMetadata.h"
#include "Outputs/CDecompiler.h"
#include "Outputs/CScriptJson.h"
#include "Outputs/CStringLiteralJson.h"
#include "Outputs/CStructGenerator.h"
#include "Utils/CIl2CppExecutor.h"

#include <cstring>
#include <memory>
#include <vector>
#include <sys/stat.h>

namespace Il2Dumper
{
    std::string DefaultOutputDir()
    {
        NSArray* p_Paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* p_Doc  = [p_Paths firstObject];
        NSString* p_Dir  = [p_Doc stringByAppendingPathComponent:@"Il2cppDump/"];
        return std::string([p_Dir UTF8String]);
    }

    static bool EnsureDir(const std::string& m_Path)
    {
        // mkdir -p, layer by layer. Most cases the parent already exists.
        std::string m_Cur;
        for (size_t i = 0; i < m_Path.size(); ++i)
        {
            char c = m_Path[i];
            m_Cur += c;
            if (c == '/' && m_Cur.size() > 1)
            {
                ::mkdir(m_Cur.c_str(), 0755);
            }
        }
        struct stat m_St;
        return (::stat(m_Path.c_str(), &m_St) == 0 && S_ISDIR(m_St.st_mode));
    }

    // Resolve a data source from the sources struct. Returns nullptr on failure.
    static std::unique_ptr<IDataSource> MakeIl2cppSource(const SDumpSources& m_Src)
    {
        if (m_Src.p_Il2cppMemory && m_Src.m_Il2cppSize)
        {
            return std::make_unique<CMemorySource>(m_Src.p_Il2cppMemory, m_Src.m_Il2cppSize);
        }
        if (!m_Src.m_Il2cppPath.empty())
        {
            auto p_Fs = std::make_unique<CFileSource>(m_Src.m_Il2cppPath);
            if (!p_Fs->IsOk()) return nullptr;
            return p_Fs;
        }
        return nullptr;
    }

    static std::unique_ptr<IDataSource> MakeMetadataSource(const SDumpSources& m_Src)
    {
        if (m_Src.p_MetadataMemory && m_Src.m_MetadataSize)
        {
            return std::make_unique<CMemorySource>(m_Src.p_MetadataMemory, m_Src.m_MetadataSize);
        }
        if (!m_Src.m_MetadataPath.empty())
        {
            auto p_Fs = std::make_unique<CFileSource>(m_Src.m_MetadataPath);
            if (!p_Fs->IsOk()) return nullptr;
            return p_Fs;
        }
        return nullptr;
    }

    static E_Status RunImpl(const SDumpSources& m_Src, const SDumpConfig& m_Cfg);

    E_Status Run(const SDumpSources& m_Src, const SDumpConfig& m_Cfg)
    {
        /* Wrap the whole pipeline so any uncaught std::exception (bad_alloc
         from a garbage count, length_error, runtime_error from a malformed
         input) becomes a logged status code instead of silent process
         termination */
        try
        {
            return RunImpl(m_Src, m_Cfg);
        }
        catch (const std::exception& e)
        {
            IL2D_LOG("Run: uncaught std::exception: %s", e.what());
            return E_Status::E_IoFailure;
        }
    }

    static E_Status RunImpl(const SDumpSources& m_Src, const SDumpConfig& m_Cfg)
    {
        IL2D_LOG("Run: starting (memory mode=%s)",
                 (m_Src.p_Il2cppMemory ? "yes" : "no"));

        // 1. Load metadata
        auto p_MetaSrc = MakeMetadataSource(m_Src);
        if (!p_MetaSrc)
        {
            IL2D_LOG("Run: metadata source missing (path='%s', memptr=%p)",
                     m_Src.m_MetadataPath.c_str(), m_Src.p_MetadataMemory);
            return E_Status::E_IoFailure;
        }
        IL2D_LOG("Run: metadata source ready, %zu bytes", p_MetaSrc->Size());

        CMetadata m_Meta(*p_MetaSrc);
        E_Status m_S = m_Meta.Init();
        if (m_S != E_Status::E_Ok)
        {
            IL2D_LOG("Run: metadata init failed (code=%d)", int(m_S));
            return m_S;
        }
        IL2D_LOG("Run: metadata loaded, typeDefs=%zu methodDefs=%zu imageDefs=%zu fieldDefs=%zu",
                 m_Meta.TypeDefs().size(), m_Meta.MethodDefs().size(),
                 m_Meta.ImageDefs().size(), m_Meta.FieldDefs().size());

        // 2. Load il2cpp binary, dealing with fat archives if needed.
        auto p_BinSrc = MakeIl2cppSource(m_Src);
        if (!p_BinSrc)
        {
            IL2D_LOG("Run: il2cpp source missing");
            return E_Status::E_IoFailure;
        }
        IL2D_LOG("Run: il2cpp source ready, %zu bytes", p_BinSrc->Size());

        std::vector<uint8_t>            m_FatSlice;
        std::unique_ptr<CMemorySource>  p_FatBacked;
        const IDataSource*              p_ActiveBin = p_BinSrc.get();
        if (p_BinSrc->Size() >= 4)
        {
            uint32_t m_Magic = 0;
            std::memcpy(&m_Magic, p_BinSrc->Data(), 4);
            IL2D_LOG("Run: il2cpp magic=0x%08x", m_Magic);
            
            if (m_Magic == 0xCAFEBABE || m_Magic == 0xBEBAFECA)
            {
                IL2D_LOG("Run: fat archive detected, extracting arm64 slice");
                CMachoFat m_Fat(*p_BinSrc);
                int m_Idx = m_Fat.PickArm64();
                if (m_Idx < 0)
                {
                    IL2D_LOG("Run: no arm64 slice in fat archive (%zu slices)", m_Fat.Slices().size());
                    return E_Status::E_NoArm64Slice;
                }
                if (!m_Fat.ExtractSlice(size_t(m_Idx), m_FatSlice))
                {
                    IL2D_LOG("Run: fat slice extract failed");
                    return E_Status::E_NoArm64Slice;
                }
                p_FatBacked = std::make_unique<CMemorySource>(m_FatSlice.data(), m_FatSlice.size());
                p_ActiveBin = p_FatBacked.get();
                IL2D_LOG("Run: using arm64 slice, %zu bytes", p_ActiveBin->Size());
            }
        }

        CMachO64 m_Mach(*p_ActiveBin);
        if (!m_Mach.Ok())
        {
            IL2D_LOG("Run: Mach-O parse failed");
            return E_Status::E_UnsupportedBinaryFormat;
        }
        IL2D_LOG("Run: Mach-O parsed, vmaddr=0x%llx sections=%zu",
                 (unsigned long long)m_Mach.VmAddr(), m_Mach.Sections().size());


        if (m_Src.p_Il2cppMemory && m_Src.m_ImageBase)
        {
            m_Mach.SetSlide(m_Src.m_ImageBase);
            IL2D_LOG("Run: live image slide=0x%llx", (unsigned long long)m_Src.m_ImageBase);
        }

        // 3. Build Il2Cpp parser and run the registration search. Il2Cpp
        //    version is compile-time-selected via the IL2CPP_VER_* macro.

        CIl2Cpp m_Il2Cpp(&m_Mach);
        m_Il2Cpp.SetMetadataUsagesCount(m_Meta.MetadataUsagesCount());

        // Count methods with a real method index (pre-v24.2 single-table path).
        int m_MethodCount = 0;
        for (const auto& md : m_Meta.MethodDefs())
        {
        #ifndef IL2CPP_VER_GE_24_2
            if (md.m_MethodIndex >= 0) ++m_MethodCount;
        #else
            (void)md;
            ++m_MethodCount; // count is unused for v24.2+; pass total
        #endif
        }
        IL2D_LOG("Run: PlusSearch with methodCount=%d typeDefCount=%d imageCount=%d",
                 m_MethodCount, int(m_Meta.TypeDefs().size()), int(m_Meta.ImageDefs().size()));
        if (!m_Il2Cpp.PlusSearch(m_MethodCount, int(m_Meta.TypeDefs().size()), int(m_Meta.ImageDefs().size())))
        {
            IL2D_LOG("Run: PlusSearch FAILED -> E_SearchFailed (-5)");
            return E_Status::E_SearchFailed;
        }
        IL2D_LOG("Run: PlusSearch ok");

        // 4. Pick output dir
        std::string m_OutDir = m_Cfg.m_OutputDir.empty() ? DefaultOutputDir() : m_Cfg.m_OutputDir;
        if (!m_OutDir.empty() && m_OutDir.back() != '/') m_OutDir.push_back('/');
        if (!EnsureDir(m_OutDir)) return E_Status::E_IoFailure;

        // 5. Generate outputs.
        CIl2CppExecutor m_Exec(&m_Meta, &m_Il2Cpp);

        CDecompiler m_Dec(&m_Exec);
        if (!m_Dec.Decompile(m_Cfg, m_OutDir)) return E_Status::E_IoFailure;

        if (m_Cfg.bGenerateStruct)
        {
            CStructGenerator m_Sg(&m_Exec);
            m_Sg.WriteHeader(m_OutDir);
        }
        if (m_Cfg.bGenerateScript)
        {
            CScriptJson m_Sj(&m_Exec);
            m_Sj.Write(m_OutDir);
        }
        if (m_Cfg.bDumpStringLiteral)
        {
            CStringLiteralJson m_Sl(&m_Meta);
            m_Sl.Write(m_OutDir);
        }

        NSLog(@"[Il2ccpDumper] Dump complete -> %s", m_OutDir.c_str());
        return E_Status::E_Ok;
    }
}
