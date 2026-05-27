//
//  CDecompiler.mm
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CDecompiler.h"
#include "Il2CppConstants.h"
#include "../Core/Log.h"

#include <cstdio>
#include <cstdint>
#include <string>

namespace Il2Dumper
{
    /* Field-visibility modifier from the field type's attrs (FIELD_ATTRIBUTE_*).
     Mirrors the original Il2CppDumper logic where the access mask lives in
     the field's Il2CppType, not on the FieldDefinition itself */

    static const char* FieldAccessText(uint32_t m_Attrs)
    {
        switch (m_Attrs & E_FA_FieldAccessMask)
        {
        case E_FA_Private:     return "private ";
        case E_FA_Public:      return "public ";
        case E_FA_Family:      return "protected ";
        case E_FA_Assembly:
        case E_FA_FamAndAssem: return "internal ";
        case E_FA_FamOrAssem:  return "protected internal ";
        default:               return "";
        }
    }

    /* Method-attribute helpers. The full set is encoded in m_Md.m_Flags as a
     METHOD_ATTRIBUTE_* bitfield; the access mask is the same shape as
     FieldAttributes but the storage modifiers (static/virtual/abstract)
     differ -- see ECMA-335 II.23.1.10 */
    static std::string MethodModifiers(uint16_t m_Flags)
    {
        std::string m_Out;
        switch (m_Flags & E_MA_MemberAccessMask)
        {
        case E_MA_Private:     m_Out  = "private ";  break;
        case E_MA_Assem:
        case E_MA_FamAndAssem: m_Out  = "internal "; break;
        case E_MA_Family:      m_Out  = "protected "; break;
        case E_MA_FamOrAssem:  m_Out  = "protected internal "; break;
        case E_MA_Public:      m_Out  = "public ";   break;
        default: break;
        }
        if (m_Flags & E_MA_Static)   m_Out += "static ";
        if (m_Flags & E_MA_Abstract) m_Out += "abstract ";
        else if (m_Flags & E_MA_Final) m_Out += "sealed ";
        else if ((m_Flags & E_MA_Virtual) && !(m_Flags & E_MA_Final)) m_Out += "virtual ";
        return m_Out;
    }
    std::string CDecompiler::VisibilityForType(uint32_t m_Flags) const
    {
        switch (m_Flags & E_TA_VisibilityMask)
        {
        case E_TA_Public:
        case E_TA_NestedPublic:        return "public ";
        case E_TA_NotPublic:
        case E_TA_NestedFamAndAssem:
        case E_TA_NestedAssembly:      return "internal ";
        case E_TA_NestedPrivate:       return "private ";
        case E_TA_NestedFamily:        return "protected ";
        case E_TA_NestedFamOrAssem:    return "protected internal ";
        default:                       return "";
        }
    }

    std::string CDecompiler::VisibilityForMember(uint32_t m_Flags) const
    {
        switch (m_Flags & E_MA_MemberAccessMask)
        {
        case E_MA_Private:      return "private ";
        case E_MA_FamAndAssem:  return "internal protected ";
        case E_MA_Assem:        return "internal ";
        case E_MA_Family:       return "protected ";
        case E_MA_FamOrAssem:   return "protected internal ";
        case E_MA_Public:       return "public ";
        default:                return "";
        }
    }

    bool CDecompiler::Decompile(const SDumpConfig& m_Cfg, const std::string& m_OutDir)
    {
        std::string m_Path = m_OutDir + "dump.cs";
        FILE* p_F = std::fopen(m_Path.c_str(), "wb");
        if (!p_F) return false;

        CMetadata* p_Meta   = p_Exec_->Meta();
        CIl2Cpp*   p_Il2Cpp = p_Exec_->Il2Cpp();


        size_t m_MethodsTotal    = 0;
        size_t m_MethodsWithAddr = 0;
        size_t m_FailLogged      = 0;
        const size_t k_FailLogCap = 8;

        /* Cross-check: log a side-by-side of metadata image names vs the
         codeGenModule names the binary parse came up with. When the two
        don't align, GetMethodPointer fails for every method below */
        
        const auto& m_CgmOrder = p_Il2Cpp->CodeGenModuleOrder();
        IL2D_LOG("Decompile: %zu metadata images, %zu codeGenModules",
                 p_Meta->ImageDefs().size(), m_CgmOrder.size());
        for (size_t i = 0; i < p_Meta->ImageDefs().size() && i < 8; ++i)
        {
            std::string m_MetaName = p_Meta->GetStringFromIndex(p_Meta->ImageDefs()[i].m_NameIndex);
            const std::string m_BinName = (i < m_CgmOrder.size()) ? m_CgmOrder[i] : "<none>";
            IL2D_LOG("  image[%zu]: meta='%s' binary='%s' methodPointers=%zu",
                     i, m_MetaName.c_str(), m_BinName.c_str(),
                     p_Il2Cpp->CodeGenModuleMethodCount(int(i)));
        }

        // Image header
        for (size_t i = 0; i < p_Meta->ImageDefs().size(); ++i)
        {
            const SImageDefinition& m_Img = p_Meta->ImageDefs()[i];
            std::string m_Name = p_Meta->GetStringFromIndex(m_Img.m_NameIndex);
            std::fprintf(p_F, "// Image %zu: %s - %d\n", i, m_Name.c_str(), m_Img.m_TypeStart);
        }

        for (size_t m_ImageIdx = 0; m_ImageIdx < p_Meta->ImageDefs().size(); ++m_ImageIdx)
        {
            const SImageDefinition& m_Img = p_Meta->ImageDefs()[m_ImageIdx];
            std::string m_ImageName = p_Meta->GetStringFromIndex(m_Img.m_NameIndex);
            int m_End = m_Img.m_TypeStart + int(m_Img.m_TypeCount);
            for (int m_TdIdx = m_Img.m_TypeStart; m_TdIdx < m_End; ++m_TdIdx)
            {
                if (m_TdIdx < 0 || size_t(m_TdIdx) >= p_Meta->TypeDefs().size()) continue;
                const STypeDefinition& m_Td = p_Meta->TypeDefs()[size_t(m_TdIdx)];

                std::string m_Ns = p_Meta->GetStringFromIndex(m_Td.m_NamespaceIndex);
                std::fprintf(p_F, "\n// Namespace: %s\n", m_Ns.c_str());

                std::string m_Vis = VisibilityForType(m_Td.m_Flags);
                std::fprintf(p_F, "%s", m_Vis.c_str());

                if ((m_Td.m_Flags & E_TA_Abstract) && (m_Td.m_Flags & E_TA_Sealed))
                {
                    std::fprintf(p_F, "static ");
                }
                else if (!(m_Td.m_Flags & E_TA_Interface) && (m_Td.m_Flags & E_TA_Abstract))
                {
                    std::fprintf(p_F, "abstract ");
                }
                else if (!m_Td.IsValueType() && !m_Td.IsEnum() && (m_Td.m_Flags & E_TA_Sealed))
                {
                    std::fprintf(p_F, "sealed ");
                }

                if (m_Td.m_Flags & E_TA_Interface)      std::fprintf(p_F, "interface ");
                else if (m_Td.IsEnum())                std::fprintf(p_F, "enum ");
                else if (m_Td.IsValueType())           std::fprintf(p_F, "struct ");
                else                                    std::fprintf(p_F, "class ");

                std::string m_TypeName = p_Exec_->GetTypeDefName(m_Td, false, true);
                std::fprintf(p_F, "%s", m_TypeName.c_str());

                // Inheritance
                if (m_Td.m_ParentIndex >= 0 && size_t(m_Td.m_ParentIndex) < p_Il2Cpp->Types().size())
                {
                    const SIl2CppType& m_Parent = p_Il2Cpp->Types()[size_t(m_Td.m_ParentIndex)];
                    std::string m_ParentName = p_Exec_->GetTypeName(m_Parent, false, false);
                    if (!m_Td.IsValueType() && !m_Td.IsEnum() && m_ParentName != "object")
                    {
                        std::fprintf(p_F, " : %s", m_ParentName.c_str());
                    }
                }

                if (m_Cfg.bDumpTypeDefIndex) std::fprintf(p_F, " // TypeDefIndex: %d\n{", m_TdIdx);
                else                          std::fprintf(p_F, "\n{");

                // Fields
                if (m_Cfg.bDumpField && m_Td.m_FieldCount > 0)
                {
                    std::fprintf(p_F, "\n\t// Fields\n");
                    int m_FStart = m_Td.m_FieldStart;
                    int m_FEnd   = m_FStart + int(m_Td.m_FieldCount);

                    for (int i = m_FStart; i < m_FEnd; ++i)
                    {
                        if (i < 0 || size_t(i) >= p_Meta->FieldDefs().size()) continue;
                        const SFieldDefinition& m_Fd = p_Meta->FieldDefs()[size_t(i)];
                        std::string m_FName = p_Meta->GetStringFromIndex(m_Fd.m_NameIndex);

                        /* Field type + its decoded attrs (Il2CppType.attrs is
                         where field visibility / static / const / readonly
                         live, NOT on the FieldDefinition) */
                        
                        std::string m_FType  = "?";
                        uint32_t    m_FAttrs = 0;
                        if (m_Fd.m_TypeIndex >= 0 && size_t(m_Fd.m_TypeIndex) < p_Il2Cpp->Types().size())
                        {
                            const SIl2CppType& m_Ft = p_Il2Cpp->Types()[size_t(m_Fd.m_TypeIndex)];
                            m_FType  = p_Exec_->GetTypeName(m_Ft, false, false);
                            m_FAttrs = m_Ft.m_Attrs;
                        }

                        const bool bIsLiteral  = (m_FAttrs & E_FA_Literal)  != 0;
                        const bool bIsStatic   = (m_FAttrs & E_FA_Static)   != 0;
                        const bool bIsInitOnly = (m_FAttrs & E_FA_InitOnly) != 0;

                        std::fprintf(p_F, "\t%s", FieldAccessText(m_FAttrs));
                        if (bIsLiteral)
                        {
                            std::fprintf(p_F, "const ");
                        }
                        else
                        {
                            if (bIsStatic)   std::fprintf(p_F, "static ");
                            if (bIsInitOnly) std::fprintf(p_F, "readonly ");
                        }
                        std::fprintf(p_F, "%s %s;", m_FType.c_str(), m_FName.c_str());

                        if (m_Cfg.bDumpFieldOffset)
                        {
                            int m_Off = p_Il2Cpp->GetFieldOffsetFromIndex(m_TdIdx, i - m_FStart, i,
                                                                          m_Td.IsValueType(), bIsStatic);
                            if (m_Off >= 0) std::fprintf(p_F, " // 0x%X", m_Off);
                        }
                        std::fprintf(p_F, "\n");
                    }
                }

                // Methods
                if (m_Cfg.bDumpMethod && m_Td.m_MethodCount > 0)
                {
                    std::fprintf(p_F, "\n\t// Methods\n");
                    int m_MStart = m_Td.m_MethodStart;
                    int m_MEnd   = m_MStart + int(m_Td.m_MethodCount);

                    for (int i = m_MStart; i < m_MEnd; ++i)
                    {
                        if (i < 0 || size_t(i) >= p_Meta->MethodDefs().size()) continue;

                        const SMethodDefinition& m_Md = p_Meta->MethodDefs()[size_t(i)];
                        std::string m_MName = p_Meta->GetStringFromIndex(m_Md.m_NameIndex);

                        std::string m_RetName = "void";
                        if (m_Md.m_ReturnType >= 0 && size_t(m_Md.m_ReturnType) < p_Il2Cpp->Types().size())
                        {
                            m_RetName = p_Exec_->GetTypeName(p_Il2Cpp->Types()[size_t(m_Md.m_ReturnType)], false, false);
                        }

                        std::fprintf(p_F, "\t%s%s %s(",
                                     MethodModifiers(m_Md.m_Flags).c_str(),
                                     m_RetName.c_str(), m_MName.c_str());

                        for (int p = 0; p < int(m_Md.m_ParameterCount); ++p)
                        {
                            int m_PIdx = m_Md.m_ParameterStart + p;

                            if (m_PIdx < 0 || size_t(m_PIdx) >= p_Meta->ParameterDefs().size()) continue;

                            const SParameterDefinition& m_Pd = p_Meta->ParameterDefs()[size_t(m_PIdx)];
                            std::string m_PName = p_Meta->GetStringFromIndex(m_Pd.m_NameIndex);
                            std::string m_PType = "?";

                            if (m_Pd.m_TypeIndex >= 0 && size_t(m_Pd.m_TypeIndex) < p_Il2Cpp->Types().size())
                            {
                                m_PType = p_Exec_->GetTypeName(p_Il2Cpp->Types()[size_t(m_Pd.m_TypeIndex)], false, false);
                            }

                            if (p) std::fprintf(p_F, ", ");
                            std::fprintf(p_F, "%s %s", m_PType.c_str(), m_PName.c_str());
                        }
                        std::fprintf(p_F, ") { }");

                        ++m_MethodsTotal;
                        if (m_Cfg.bDumpMethodOffset)
                        {
                        #ifdef IL2CPP_VER_LE_24_1
                            int32_t m_MIdxArg = m_Md.m_MethodIndex;
                        #else
                            int32_t m_MIdxArg = -1;
                        #endif
                            uint64_t m_Va = p_Il2Cpp->GetMethodPointerByImageIndex(int(m_ImageIdx), m_Md.m_Token, m_MIdxArg);

                            if (m_Va == 0 && m_FailLogged < k_FailLogCap)
                            {
                                ++m_FailLogged;
                                IL2D_LOG("  miss: img[%zu]='%s' method='%s' token=0x%08x idx=%u tbl=%zu",
                                         m_ImageIdx, m_ImageName.c_str(), m_MName.c_str(),
                                         m_Md.m_Token, unsigned(m_Md.m_Token & 0x00FFFFFFu),
                                         p_Il2Cpp->CodeGenModuleMethodCount(int(m_ImageIdx)));
                            }
                            if (m_Va)
                            {
                                ++m_MethodsWithAddr;
                                // Address triplet matching the various Il2CppDumper forks:
                                //   RVA     = method pointer - vmaddr base (== scrubbed VA here)
                                //   Offset  = file offset of the code (MapVATR resolution)
                                //   VA      = live process address (RVA + slide)
                                //   Slot    = m_Md.m_Slot when it's a real vtable slot
                                uint64_t m_Rva    = m_Va - p_Il2Cpp->ImageBase();
                                uint64_t m_Offset = p_Il2Cpp->Mach()->MapVATR(m_Va);
                                uint64_t m_Slide  = p_Il2Cpp->Mach()->Slide();
                                uint64_t m_AbsVa  = (m_Slide != 0) ? (m_Slide + m_Va) : m_Va;
                                std::fprintf(p_F,
                                    " // RVA: 0x%llx Offset: 0x%llx VA: 0x%llx",
                                    (unsigned long long)m_Rva,
                                    (unsigned long long)m_Offset,
                                    (unsigned long long)m_AbsVa);
                                if (m_Md.m_Slot != 0xFFFF)
                                {
                                    std::fprintf(p_F, " Slot: %u", unsigned(m_Md.m_Slot));
                                }
                            }
                        }
                        std::fprintf(p_F, "\n");
                    }
                }
                std::fprintf(p_F, "}\n");
            }
        }
        std::fclose(p_F);
        IL2D_LOG("CDecompiler::Decompile: %zu/%zu methods got RVA (%.1f%%)",
                 m_MethodsWithAddr, m_MethodsTotal,
                 m_MethodsTotal ? 100.0 * double(m_MethodsWithAddr) / double(m_MethodsTotal) : 0.0);
        return true;
    }
}
