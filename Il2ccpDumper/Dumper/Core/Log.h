//
//  Log.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//
//  Logging macro for the dumper. Goes through Il2DumperLog() (implemented
//  in Log.mm). Defaults: os_log backend, payloads tagged {public} so they
//  survive release-build redaction in Console.app.
//
//  Build flags (all default ON unless noted):
//    IL2DUMPER_LOG_OSLOG   -- 1 -> os_log, 0 -> NSLog (default 1)
//    IL2DUMPER_LOG_PUBLIC  -- 1 -> %{public}s, 0 -> %{private}s (default 1)
//    IL2DUMPER_LOG_DISABLE -- if defined, compile every IL2D_LOG out
//

#pragma once

// Defaults -- the build can override either with -DIL2DUMPER_LOG_*=0.
#ifndef IL2DUMPER_LOG_OSLOG
    #define IL2DUMPER_LOG_OSLOG 1
#endif

#ifndef IL2DUMPER_LOG_PUBLIC
    #define IL2DUMPER_LOG_PUBLIC 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

void Il2DumperLog(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#ifndef IL2DUMPER_LOG_DISABLE
    #define IL2D_LOG(fmt, ...) Il2DumperLog("[Il2Dumper] " fmt, ##__VA_ARGS__)
#else
    #define IL2D_LOG(fmt, ...) ((void)0)
#endif
