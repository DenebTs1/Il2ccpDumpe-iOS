#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Il2Dumper
{
    struct SMachoSection64
    {
        std::string m_SectName;
        uint64_t    m_Addr   { 0 };
        uint64_t    m_Size   { 0 };
        uint64_t    m_Offset { 0 };
        uint32_t    m_Flags  { 0 };
    };

    struct SFatSlice
    {
        uint32_t m_Offset { 0 };
        uint32_t m_Size   { 0 };
        uint32_t m_Magic  { 0 };
    };

    struct SSearchSection
    {
        uint64_t m_Offset     { 0 };
        uint64_t m_OffsetEnd  { 0 };
        uint64_t m_Address    { 0 };
        uint64_t m_AddressEnd { 0 };
    };

    enum class E_SectionKind : int
    {
        E_Exec = 0,
        E_Data = 1,
        E_Bss  = 2
    };
}
