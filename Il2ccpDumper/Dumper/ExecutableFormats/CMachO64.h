//
//  CMachO64.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "SMachoTypes.h"
#include "../IO/IDataSource.h"
#include "../IO/CBinaryStream.h"

#include <vector>

namespace Il2Dumper
{
    /* Parses a 64-bit Mach-O image and exposes the helpers Il2Cpp needs:
    virtual-address <-> file-offset translation, and section grouping
    for the structured registration search (CSectionHelper). */

    class CMachO64
    {
    public:
        explicit CMachO64(const IDataSource& m_Src);

        bool Ok() const { return bOk_; }
        bool Parse();

        uint64_t                            VmAddr()   const { return m_VmAddr_; }
        const std::vector<SMachoSection64>& Sections() const { return m_Sections_; }

        // VA -> file offset. Returns 0 if not mappable.
        uint64_t MapVATR(uint64_t m_Addr) const;

        CBinaryStream&       Stream()       { return m_Stream_; }
        const CBinaryStream& Stream() const { return m_Stream_; }

        void     SetSlide(uint64_t v) { m_Slide_ = v; }
        uint64_t Slide() const        { return m_Slide_; }

        uint64_t ReadVaPointer();

        uint64_t ScrubVa(uint64_t m_Ptr) const;

    private:
        CBinaryStream                m_Stream_;
        std::vector<SMachoSection64> m_Sections_;
        uint64_t                     m_VmAddr_ { 0 };
        uint64_t                     m_Slide_  { 0 };
        bool                         bOk_      { false };
    };
}
