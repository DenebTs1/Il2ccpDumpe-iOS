//
//  CSectionHelper.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "../ExecutableFormats/SMachoTypes.h"
#include "../ExecutableFormats/CMachO64.h"

#include <vector>

namespace Il2Dumper
{
    /* Finds CodeRegistration / MetadataRegistration by walking the data,
     exec and bss sections of the binary. Targets v24.2+ via the
     "mscorlib.dll" reference chain; legacy targets fall back to the older
     signature-counting search */

    class CSectionHelper
    {
    public:
        CSectionHelper(CMachO64* p_Mach,
                       int     m_MethodCount,
                       int     m_TypeDefCount,
                       int64_t m_MetadataUsagesCount,
                       int     m_ImageCount);

        void SetSection(E_SectionKind m_Kind, const std::vector<SSearchSection>& m_Secs);

        // Returns 0 on failure.
        uint64_t FindCodeRegistration();
        uint64_t FindMetadataRegistration();

    private:
        uint64_t FindCodeRegistrationOld();
        uint64_t FindCodeRegistration2019(const std::vector<SSearchSection>& m_Secs);
        uint64_t FindMetadataRegistrationV21();
        uint64_t FindMetadataRegistrationOld();

        bool CheckPointerRangeDataRa(uint64_t m_Ptr) const;
        bool CheckPointerRangeExecVa(const std::vector<uint64_t>& m_Ptrs) const;
        bool CheckPointerRangeDataVa(const std::vector<uint64_t>& m_Ptrs) const;
        bool CheckPointerRangeBssVa (const std::vector<uint64_t>& m_Ptrs) const;

        // Appends all VAs in `data` whose stored uintptr == m_Addr.
        void FindReference(uint64_t m_Addr, std::vector<uint64_t>& m_Out);

        CMachO64* p_Mach_;
        int       m_MethodCount_;
        int       m_TypeDefCount_;
        int64_t   m_MetadataUsagesCount_;
        int       m_ImageCount_;
        bool      bPointerInExec_ { false };

        std::vector<SSearchSection> m_Exec_;
        std::vector<SSearchSection> m_Data_;
        std::vector<SSearchSection> m_Bss_;
    };
}
