//
//  CScriptJson.mm
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CScriptJson.h"

#include <cstdio>

namespace Il2Dumper
{
    static std::string JsonEscape(const std::string& m_In)
    {
        std::string m_Out;
        m_Out.reserve(m_In.size() + 2);
        for (char c : m_In)
        {
            switch (c)
            {
            case '"':  m_Out += "\\\""; break;
            case '\\': m_Out += "\\\\"; break;
            case '\n': m_Out += "\\n";  break;
            case '\r': m_Out += "\\r";  break;
            case '\t': m_Out += "\\t";  break;
            default:
                if (uint8_t(c) < 0x20)
                {
                    char m_Buf[8];
                    std::snprintf(m_Buf, sizeof(m_Buf), "\\u%04x", c);
                    m_Out += m_Buf;
                }
                else
                {
                    m_Out += c;
                }
            }
        }
        return m_Out;
    }

    bool CScriptJson::Write(const std::string& m_OutDir)
    {
        std::string m_Path = m_OutDir + "script.json";
        FILE* p_F = std::fopen(m_Path.c_str(), "wb");
        if (!p_F) return false;

        CMetadata* p_Meta   = p_Exec_->Meta();
        CIl2Cpp*   p_Il2Cpp = p_Exec_->Il2Cpp();

        std::fprintf(p_F, "{\n  \"ScriptMethod\": [\n");
        bool b_First = true;
        for (size_t m_ImageIdx = 0; m_ImageIdx < p_Meta->ImageDefs().size(); ++m_ImageIdx)
        {
            const SImageDefinition& m_Img = p_Meta->ImageDefs()[m_ImageIdx];
            int m_End = m_Img.m_TypeStart + int(m_Img.m_TypeCount);

            for (int m_TdIdx = m_Img.m_TypeStart; m_TdIdx < m_End; ++m_TdIdx)
            {
                if (m_TdIdx < 0 || size_t(m_TdIdx) >= p_Meta->TypeDefs().size()) continue;

                const STypeDefinition& m_Td = p_Meta->TypeDefs()[size_t(m_TdIdx)];
                int m_MStart = m_Td.m_MethodStart;
                int m_MEnd   = m_MStart + int(m_Td.m_MethodCount);

                for (int i = m_MStart; i < m_MEnd; ++i)
                {
                    if (i < 0 || size_t(i) >= p_Meta->MethodDefs().size()) continue;

                    const SMethodDefinition& m_Md = p_Meta->MethodDefs()[size_t(i)];
                #ifdef IL2CPP_VER_LE_24_1
                    uint64_t m_Va = p_Il2Cpp->GetMethodPointerByImageIndex(int(m_ImageIdx), m_Md.m_Token, m_Md.m_MethodIndex);
                #else
                    uint64_t m_Va = p_Il2Cpp->GetMethodPointerByImageIndex(int(m_ImageIdx), m_Md.m_Token, -1);
                #endif
                    
                    if (!m_Va) continue;
                    
                    uint64_t m_Rva = m_Va - p_Il2Cpp->ImageBase();

                    std::string m_TypeName = p_Exec_->GetTypeDefName(m_Td, true, false);
                    std::string m_Name     = p_Meta->GetStringFromIndex(m_Md.m_NameIndex);
                    std::string m_Full     = m_TypeName + "$$" + m_Name;

                    if (!b_First) std::fprintf(p_F, ",\n");
                    
                    b_First = false;
                    std::fprintf(p_F,
                        "    { \"Address\": %llu, \"Name\": \"%s\", \"Signature\": \"%s\" }",
                        (unsigned long long)m_Rva,
                        JsonEscape(m_Full).c_str(),
                        JsonEscape(m_Name).c_str());
                }
            }
        }
        std::fprintf(p_F, "\n  ]\n}\n");
        std::fclose(p_F);
        return true;
    }
}
