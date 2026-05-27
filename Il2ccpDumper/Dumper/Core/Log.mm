//
//  Log.mm
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#import <Foundation/Foundation.h>

#include "Log.h"

#include <cstdarg>
#include <cstdlib>

#if IL2DUMPER_LOG_OSLOG
    #include <os/log.h>

    static os_log_t Il2DumperLoggerHandle(void)
    {
        static os_log_t s_p_Logger = ^os_log_t {
            NSString* p_Bundle = [[NSBundle mainBundle] bundleIdentifier] ?: @"unknown";
            NSString* p_Subsys = [p_Bundle stringByAppendingString:@".il2dumper"];
            return os_log_create([p_Subsys UTF8String], "dumper");
        }();
        return s_p_Logger;
    }
#endif

extern "C" void Il2DumperLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char* p_Formatted = NULL;
    int   m_Wrote     = vasprintf(&p_Formatted, fmt, args);
    va_end(args);
    if (m_Wrote < 0 || !p_Formatted) return;

#if IL2DUMPER_LOG_OSLOG
    #if IL2DUMPER_LOG_PUBLIC
        os_log(Il2DumperLoggerHandle(), "%{public}s",  p_Formatted);
    #else
        os_log(Il2DumperLoggerHandle(), "%{private}s", p_Formatted);
    #endif
#else
    // NSLog has no redaction; the IL2DUMPER_LOG_PUBLIC flag is a no-op here.
    NSLog(@"%s", p_Formatted);
#endif

    free(p_Formatted);
}
