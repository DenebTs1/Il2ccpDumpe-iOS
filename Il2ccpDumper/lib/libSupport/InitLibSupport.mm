//
//  InitLibSupport.mm
//  Asura
//
//  Created by Euclid Jan Guillermo on 6/8/25.
//

#include "support/support.h"

LS_CTOR_(0)
{
    // Add any files you wish to restrict the target app from accessing.
    const char *files[] = {
        "libUnrealEngine_Menu_PUBG.dylib",
    };

    // Add any url schemes you wish to restrict the target app from opening/accessing.
    const char *urls[] = {

    };

    SupportEntryInfo entry_info = {
        .teamIdentifier = NULL,                         // The original team identifier of the target app (todo)
        .bundleIdentifier = NULL,                       // The original bundleIdentifier of the terget app (null-nochange)
        .hookFlags = SupportHookFlagNone,          // Set of options to customize libSupport hooks
        .restrictedFiles = files,                       // The files the victim has no perm to access
        .restrictedFileCount = LS_ARRAYSIZE(files),     // The size of the restrictedFiles array
        .restrictedURLSchemes = urls,                   // The url schemes the victim has no perm to access
        .restrictedURLSchemeCount = LS_ARRAYSIZE(urls)  // The size of restrictedURLSchemes array
    };

    // Lets bully him
SupportInitialize(&entry_info);
}
