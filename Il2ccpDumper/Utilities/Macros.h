#pragma once

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define CallAfterSeconds(sec) dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sec * NSEC_PER_SEC), dispatch_get_main_queue(), ^

#define CallOnMainThread \
    dispatch_async(dispatch_get_main_queue(), ^

#define MainView UIViewController *vc = [UIApplication sharedApplication].keyWindow.rootViewController;

#define MainView ([UIApplication sharedApplication].keyWindow.rootViewController)

#define MainViewController ([UIApplication sharedApplication].keyWindow.rootViewController)

#define FORCEINLINE inline __attribute__((always_inline)) // you can use this to secure ur hook function, so you don't have skid or crackers copying or hooking it
#define FORCENOINLINE __attribute__((noinline))
#define RESTRICT __restrict

#define ENTRY_POINT __attribute__((constructor(101))) // init point, u call ur main func cheat here

#define INVOKE(fn) [&] { fn; }
