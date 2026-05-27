//
//  CStringLiteralJson.mm
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CStringLiteralJson.h"

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

    bool CStringLiteralJson::Write(const std::string& m_OutDir)
    {
        std::string m_Path = m_OutDir + "stringliteral.json";
        FILE* p_F = std::fopen(m_Path.c_str(), "wb");
        if (!p_F) return false;

        const auto& m_Lits = p_Meta_->StringLiterals();
        std::fprintf(p_F, "[\n");
        
        for (size_t i = 0; i < m_Lits.size(); ++i)
        {
            std::string m_S = p_Meta_->GetStringLiteralFromIndex(uint32_t(i));
            std::fprintf(p_F, "  { \"index\": %zu, \"value\": \"%s\" }%s\n",
                         i, JsonEscape(m_S).c_str(),
                         (i + 1 < m_Lits.size()) ? "," : "");
        }
        
        std::fprintf(p_F, "]\n");
        std::fclose(p_F);
        return true;
    }
}
