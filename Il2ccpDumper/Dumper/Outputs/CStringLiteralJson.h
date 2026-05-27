//
//  CStringLiteralJson.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "../Il2Cpp/CMetadata.h"

#include <string>

namespace Il2Dumper
{
    class CStringLiteralJson
    {
    public:
        explicit CStringLiteralJson(CMetadata* p_Meta) : p_Meta_(p_Meta) {}

        bool Write(const std::string& m_OutDir);

    private:
        CMetadata* p_Meta_;
    };
}
