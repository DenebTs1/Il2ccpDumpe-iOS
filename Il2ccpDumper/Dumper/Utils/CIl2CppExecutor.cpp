//
//  CIl2CppExecutor.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CIl2CppExecutor.h"
#include "CResolverShim.h"
#include "../Il2Cpp/Versions.h"

namespace Il2Dumper
{
    static const char* PrimitiveName(E_TypeKind m_K)
    {
        switch (m_K)
        {
        case E_TypeKind::E_Void:       return "void";
        case E_TypeKind::E_Boolean:    return "bool";
        case E_TypeKind::E_Char:       return "char";
        case E_TypeKind::E_I1:         return "sbyte";
        case E_TypeKind::E_U1:         return "byte";
        case E_TypeKind::E_I2:         return "short";
        case E_TypeKind::E_U2:         return "ushort";
        case E_TypeKind::E_I4:         return "int";
        case E_TypeKind::E_U4:         return "uint";
        case E_TypeKind::E_I8:         return "long";
        case E_TypeKind::E_U8:         return "ulong";
        case E_TypeKind::E_R4:         return "float";
        case E_TypeKind::E_R8:         return "double";
        case E_TypeKind::E_String:     return "string";
        case E_TypeKind::E_TypedByRef: return "TypedReference";
        case E_TypeKind::E_I:          return "IntPtr";
        case E_TypeKind::E_U:          return "UIntPtr";
        case E_TypeKind::E_Object:     return "object";
        default: return nullptr;
        }
    }

    std::string CIl2CppExecutor::GetTypeDefName(const STypeDefinition& m_Td, bool b_AddNamespace, bool /*b_Nested*/)
    {
        std::string m_Ns = p_Meta_->GetStringFromIndex(m_Td.m_NamespaceIndex);
        std::string m_Nm = p_Meta_->GetStringFromIndex(m_Td.m_NameIndex);
        if (b_AddNamespace && !m_Ns.empty())
        {
            return m_Ns + "." + m_Nm;
        }
        return m_Nm;
    }

    void CIl2CppExecutor::EnsureTypeDefIndex()
    {
        if (bIndexBuilt_) return;
        bIndexBuilt_ = true;
        const auto& m_Types = p_Il2Cpp_->Types();
        const auto& m_Defs  = p_Meta_->TypeDefs();
        for (size_t i = 0; i < m_Defs.size(); ++i)
        {
            int m_Byval = m_Defs[i].m_ByvalTypeIndex;
            if (m_Byval < 0 || size_t(m_Byval) >= m_Types.size()) continue;
            const uint64_t m_Data = m_Types[size_t(m_Byval)].m_DataPoint;
            if (m_Data != 0)
            {
                m_DataToTypeDef_[m_Data] = int(i);
            }
        }
    }

    int CIl2CppExecutor::ResolveTypeDefIndex(uint64_t m_Data) const
    {
        auto it = m_DataToTypeDef_.find(m_Data);
        if (it != m_DataToTypeDef_.end()) return it->second;
        // Fallback: treat as raw index (works on non-runtime-mutated binaries).
        if (m_Data < uint64_t(p_Meta_->TypeDefs().size())) return int(m_Data);
        return -1;
    }

    std::string CIl2CppExecutor::TryResolveRuntimeClass(const SIl2CppType& m_T, bool b_AddNamespace) const
    {
        /* Need a live image (slide set) and m_T to actually live in
        CIl2Cpp::Types() so we can recover its runtime address*/
        
        const uint64_t m_Slide = p_Il2Cpp_->Mach()->Slide();
        if (m_Slide == 0) return {};

        const auto& m_Types = p_Il2Cpp_->Types();
        if (m_Types.empty()) return {};
        const SIl2CppType* p_Base = m_Types.data();
        if (&m_T < p_Base || &m_T >= p_Base + m_Types.size()) return {};
        size_t m_Idx = size_t(&m_T - p_Base);

        const auto& m_Addrs = p_Il2Cpp_->TypeAddresses();
        if (m_Idx >= m_Addrs.size()) return {};

        void*       p_RtType = reinterpret_cast<void*>(m_Slide + m_Addrs[m_Idx]);
        const char* p_Name   = nullptr;
        const char* p_Ns     = nullptr;
        if (!ResolverGetClassName(p_RtType, &p_Name, &p_Ns)) return {};
        if (!p_Name || !*p_Name) return {};

        std::string m_Out;
        if (b_AddNamespace && p_Ns && *p_Ns)
        {
            m_Out  = p_Ns;
            m_Out += '.';
        }
        m_Out += p_Name;
        return m_Out;
    }

    std::string CIl2CppExecutor::GetTypeName(const SIl2CppType& m_T, bool b_AddNamespace, bool b_Nested)
    {
        EnsureTypeDefIndex();

        if (const char* p_Prim = PrimitiveName(m_T.m_Type))
        {
            return p_Prim;
        }

        switch (m_T.m_Type)
        {
        case E_TypeKind::E_Class:
        case E_TypeKind::E_ValueType:
        {
            int m_Idx = ResolveTypeDefIndex(m_T.m_DataPoint);
            if (m_Idx >= 0 && size_t(m_Idx) < p_Meta_->TypeDefs().size())
            {
                return GetTypeDefName(p_Meta_->TypeDefs()[size_t(m_Idx)], b_AddNamespace, b_Nested);
            }
            /* Static map missed, ask the Resolver to walk the runtime
             Il2CppClass and read its name directly. Returns "" if the
             Resolver isn't built in or isn't initialised */
            
            std::string m_Rt = TryResolveRuntimeClass(m_T, b_AddNamespace);
            if (!m_Rt.empty()) return m_Rt;
            return "<unresolved>";
        }
        case E_TypeKind::E_GenericInst:
        {
            // m_DataPoint is a VA to an SIl2CppGenericClass. Scrub then read.
            uint64_t m_GcVa = p_Il2Cpp_->Mach()->ScrubVa(m_T.m_DataPoint);
            SIl2CppGenericClass m_Gc = p_Il2Cpp_->ReadAtVA<SIl2CppGenericClass>(m_GcVa);

            std::string m_Base = "<generic>";
        #ifdef IL2CPP_VER_GE_27
            // gc.m_Type is a VA to the base SIl2CppType.
            if (const SIl2CppType* p_BaseT = p_Il2Cpp_->GetIl2CppType(p_Il2Cpp_->Mach()->ScrubVa(m_Gc.m_Type)))
            {
                int m_Idx = ResolveTypeDefIndex(p_BaseT->m_DataPoint);
                if (m_Idx >= 0 && size_t(m_Idx) < p_Meta_->TypeDefs().size())
                {
                    m_Base = GetTypeDefName(p_Meta_->TypeDefs()[size_t(m_Idx)], b_AddNamespace, b_Nested);
                }
            }
        #else
            if (m_Gc.m_TypeDefinitionIndex >= 0 &&
                size_t(m_Gc.m_TypeDefinitionIndex) < p_Meta_->TypeDefs().size())
            {
                m_Base = GetTypeDefName(p_Meta_->TypeDefs()[size_t(m_Gc.m_TypeDefinitionIndex)], b_AddNamespace, b_Nested);
            }
        #endif

            // Strip the `1, `2 generic-arity suffix Mono leaves on the metadata name, C# shows `List<T>` not `List`1<T>`.
            auto m_Tick = m_Base.rfind('`');
            if (m_Tick != std::string::npos)
            {
                m_Base.erase(m_Tick);
            }

            // Resolve actual type arguments: read SIl2CppGenericInst at class_inst, then read each arg type via the VA list.
            std::string m_Args;
            if (m_Gc.m_Context.m_ClassInst)
            {
                uint64_t m_InstVa = p_Il2Cpp_->Mach()->ScrubVa(m_Gc.m_Context.m_ClassInst);
                SIl2CppGenericInst m_Inst = p_Il2Cpp_->ReadAtVA<SIl2CppGenericInst>(m_InstVa);
                if (m_Inst.m_TypeArgc > 0 && m_Inst.m_TypeArgc < 32 && m_Inst.m_TypeArgv)
                {
                    uint64_t m_ArgvOff = p_Il2Cpp_->Mach()->MapVATR(p_Il2Cpp_->Mach()->ScrubVa(m_Inst.m_TypeArgv));
                    if (m_ArgvOff)
                    {
                        CBinaryStream& m_S = p_Il2Cpp_->Mach()->Stream();
                        m_S.SeekTo(m_ArgvOff);
                        for (int64_t a = 0; a < m_Inst.m_TypeArgc; ++a)
                        {
                            uint64_t m_ArgVa = p_Il2Cpp_->Mach()->ReadVaPointer();
                            if (const SIl2CppType* p_ArgT = p_Il2Cpp_->GetIl2CppType(m_ArgVa))
                            {
                                if (a) m_Args += ", ";
                                m_Args += GetTypeName(*p_ArgT, false, false);
                            }
                        }
                    }
                }
            }
            return m_Base + "<" + (m_Args.empty() ? "..." : m_Args) + ">";
        }
        case E_TypeKind::E_SzArray:
        {
            uint64_t m_InnerVa = p_Il2Cpp_->Mach()->ScrubVa(m_T.m_DataPoint);
            if (const SIl2CppType* p_Inner = p_Il2Cpp_->GetIl2CppType(m_InnerVa))
            {
                return GetTypeName(*p_Inner, b_AddNamespace, false) + "[]";
            }
            return "<unresolved>[]";
        }
        case E_TypeKind::E_Array:
        {
            uint64_t m_ArrVa = p_Il2Cpp_->Mach()->ScrubVa(m_T.m_DataPoint);
            SIl2CppArrayType m_Arr = p_Il2Cpp_->ReadAtVA<SIl2CppArrayType>(m_ArrVa);
            std::string m_InnerName = "<unresolved>";
            if (const SIl2CppType* p_Inner = p_Il2Cpp_->GetIl2CppType(p_Il2Cpp_->Mach()->ScrubVa(m_Arr.m_EType)))
            {
                m_InnerName = GetTypeName(*p_Inner, b_AddNamespace, false);
            }
            int m_Rank = m_Arr.m_Rank > 0 ? m_Arr.m_Rank : 1;
            return m_InnerName + "[" + std::string(size_t(m_Rank - 1), ',') + "]";
        }
        case E_TypeKind::E_Ptr:
        {
            uint64_t m_InnerVa = p_Il2Cpp_->Mach()->ScrubVa(m_T.m_DataPoint);
            if (const SIl2CppType* p_Inner = p_Il2Cpp_->GetIl2CppType(m_InnerVa))
            {
                return GetTypeName(*p_Inner, b_AddNamespace, false) + "*";
            }
            return "<unresolved>*";
        }
        case E_TypeKind::E_Var:
        case E_TypeKind::E_MVar:
        {
            int64_t m_Idx = m_T.GenericParameterIndex();
            if (m_Idx >= 0 && size_t(m_Idx) < p_Meta_->GenericParameters().size())
            {
                const SGenericParameter& m_Gp = p_Meta_->GenericParameters()[size_t(m_Idx)];
                std::string m_Name = p_Meta_->GetStringFromIndex(m_Gp.m_NameIndex);
                if (!m_Name.empty()) return m_Name;
            }
            return (m_T.m_Type == E_TypeKind::E_MVar) ? "TMethod" : "T";
        }
        case E_TypeKind::E_ByRef:
        {
            uint64_t m_InnerVa = p_Il2Cpp_->Mach()->ScrubVa(m_T.m_DataPoint);
            if (const SIl2CppType* p_Inner = p_Il2Cpp_->GetIl2CppType(m_InnerVa))
            {
                return "ref " + GetTypeName(*p_Inner, b_AddNamespace, false);
            }
            return "ref <unresolved>";
        }
        case E_TypeKind::E_FnPtr:
            return "IntPtr";
        default:
            return "object";
        }
    }
}
