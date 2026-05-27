#pragma once

#include "ImGuiDrawView.h"
#include "MenuLoad.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_metal.h"

#include "../Utilities/Singleton.h"
#include "../Utilities/Macros.h"

#include "../Utilities/Format/format.h"
#include "../Utilities/Format/xchar.h"
#include "../Utilities/Format/core.h"

#include <stdio.h>
#include <vector>
#include <map>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <functional>
#include <iostream>
#include <queue>
#include <codecvt>
#include <mutex>
#include <unordered_set>
#include <concepts>
#include <unordered_map>
#include <regex>
#include <array>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#import <Security/Security.h>
#import <os/log.h>
#import <dlfcn.h>
#import <mach-o/dyld.h>
#import <mach/mach.h>
#import <cmath>
#import <pthread/pthread.h>

extern UIButton* GInvisibleMenuButton;
extern UIButton* GVisibleMenuButton;
extern MenuInteraction* GMenuTouchView;
extern UITextField* GHideRecordTextField;
extern UIView* GHideRecordView;

inline ImFont* GIconFont = nullptr;
inline ImFont* Font = nullptr;


FORCEINLINE void CrashSafe()
{
    *(volatile int*)0 = 1;
    return;
}

template<typename To>
FORCEINLINE To* Cast(void* Src)
{
    return static_cast<To*>(Src);
}

template<typename To>
FORCEINLINE const To* Cast(const void* Src)
{
    return static_cast<const To*>(Src);
}

struct
{
    int Width;
    int Height;
    int HalfWidth;
    int HalfHeight;
    int Scale;
    
    void Init()
    {
        this->Height = [UIScreen mainScreen].bounds.size.height;
        this->Width = [UIScreen mainScreen].bounds.size.width;
        this->HalfHeight = this->Height / 2;
        this->HalfWidth = this->Width / 2;
        this->Scale = [UIScreen mainScreen].scale;
    }
    
}inline ScreenRect;

#include "../Menu/Settings.h"

static Settings &settings = Settings::GetInstance();
