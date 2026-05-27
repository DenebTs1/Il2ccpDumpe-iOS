//
//  CResolverShim.mm
//  Il2ccpDumper
//
//  Created by ts1 on 27/05/2026.
//
//  Self-contained runtime reflection helper. We don't link the external
//  Resolver lib; we just dlsym the one il2cpp_* export we need and define
//  the slimmest possible Il2CppClass/Il2CppType layouts to navigate the
//  returned pointers.
//

#import <Foundation/Foundation.h>

#include "CResolverShim.h"

#include <atomic>
#include <dlfcn.h>

namespace Il2Dumper
{

    struct RtIl2CppImage; // forward decl, opaque
    struct RtIl2CppType;  // forward decl, opaque

    struct RtIl2CppClass
    {
        const RtIl2CppImage* m_pImage;
        void*                m_pGC;
        const char*          m_pName;
        const char*          m_pNamespace;
    };

    using FClassFromIl2cppType = RtIl2CppClass* (*)(const RtIl2CppType*);


    static FClassFromIl2cppType LoadClassFromIl2cppType()
    {
        /* RTLD_DEFAULT walks every loaded image, works whether
         UnityFramework is a framework or a dylib, and whether the symbol
         is in the main binary or one of its dependencies */
        
        void* p_Sym = dlsym(RTLD_DEFAULT, "il2cpp_class_from_il2cpp_type");
        return reinterpret_cast<FClassFromIl2cppType>(p_Sym);
    }

    static FClassFromIl2cppType ClassFromType()
    {
        static std::atomic<FClassFromIl2cppType> s_Cached { nullptr };
        static std::atomic<bool>                  s_Tried  { false };

        FClassFromIl2cppType p_Fn = s_Cached.load(std::memory_order_acquire);
        if (p_Fn) return p_Fn;
        if (s_Tried.load(std::memory_order_acquire)) return nullptr;

        p_Fn = LoadClassFromIl2cppType();
        s_Cached.store(p_Fn, std::memory_order_release);
        s_Tried.store(true,  std::memory_order_release);
        return p_Fn;
    }

    bool ResolverGetClassName(void*        p_RtIl2CppType,
                              const char** pp_OutName,
                              const char** pp_OutNamespace)
    {
        if (!p_RtIl2CppType) return false;

        FClassFromIl2cppType p_Fn = ClassFromType();
        if (!p_Fn) return false;

        RtIl2CppClass* p_Klass = p_Fn(reinterpret_cast<const RtIl2CppType*>(p_RtIl2CppType));
        if (!p_Klass)          return false;
        if (!p_Klass->m_pName) return false;

        if (pp_OutName)      *pp_OutName      = p_Klass->m_pName;
        if (pp_OutNamespace) *pp_OutNamespace = p_Klass->m_pNamespace;
        return true;
    }
}
