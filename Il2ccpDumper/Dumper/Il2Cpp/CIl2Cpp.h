//
//  CIl2Cpp.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#pragma once

#include "Versions.h"
#include "Il2CppTypes.h"
#include "../ExecutableFormats/CMachO64.h"
#include "../Core/Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Il2Dumper
{
    class CSectionHelper;

    /* Wraps the parsed Il2Cpp Mach-O image and reads Code/Metadata
     Registration after a search has located them. Version is compile-
     time selected via IL2CPP_VER_*; only the metadata-usage count comes
     in at runtime */

    class CIl2Cpp
    {
    public:
        explicit CIl2Cpp(CMachO64* p_Mach);

        void SetMetadataUsagesCount(int64_t v) { m_MetadataUsagesCount_ = v; }

        bool PlusSearch(int m_MethodCount, int m_TypeDefCount, int m_ImageCount);
        bool Init(uint64_t m_CodeRegistration, uint64_t m_MetadataRegistration);

        static constexpr double Version()  { return IL2CPP_TARGET_VERSION; }
        uint64_t                ImageBase() const { return p_Mach_->VmAddr(); }
        CMachO64*               Mach()       { return p_Mach_; }
        const CMachO64*         Mach() const { return p_Mach_; }

        const std::vector<SIl2CppType>& Types()         const { return m_Types_; }
        const std::vector<uint64_t>&    TypeAddresses() const { return m_TypeAddresses_; }

        /* Returns the parsed SIl2CppType pointed at by a scrubbed VA, or
         nullptr when the VA isn't in MetadataRegistration.types[] */
        
        const SIl2CppType* GetIl2CppType(uint64_t m_Va) const
        {
            auto it = m_TypeDic_.find(m_Va);
            if (it == m_TypeDic_.end()) return nullptr;
            return &m_Types_[it->second];
        }

        /* Address a codeGenModule by its ordinal (metadata image index ==
         binary codeGenModule index for any unmodified Unity build)
         Returns the resolved method pointer or 0 on miss */
        
        uint64_t GetMethodPointerByImageIndex(int      m_ImageIdx,
                                              uint32_t m_Token,
                                              int32_t  m_MethodIndex) const;

        // Per-module table size, used by the decompiler's diagnostic log.
        const std::vector<std::string>& CodeGenModuleOrder() const { return m_CodeGenModuleOrder_; }
        size_t                          CodeGenModuleMethodCount(int m_ImageIdx) const;

        /* Resolve a field offset. b_ValueType + !b_Static adjusts for the
        VTable header in instance value types */
        
        int GetFieldOffsetFromIndex(int m_TypeIndex, int m_FieldIndexInType,
                                    int m_FieldIndex, bool b_ValueType, bool b_Static);

        // VA-aware POD read: translate VA -> file offset, then T::Read(stream).
        template <typename T> T ReadAtVA(uint64_t m_Va);

    private:
        CMachO64*                   p_Mach_;
        int64_t                     m_MetadataUsagesCount_ { 0 };

        SIl2CppCodeRegistration     m_CodeReg_ {};
        SIl2CppMetadataRegistration m_MetaReg_ {};

        std::vector<uint64_t>                m_FieldOffsetsPtrTable_;
        std::vector<uint32_t>                m_FieldOffsetsValueTable_;
        bool                                 bFieldOffsetsArePointers_ { true };

        std::vector<SIl2CppType>             m_Types_;
        std::vector<uint64_t>                m_TypeAddresses_;
        std::unordered_map<uint64_t, size_t> m_TypeDic_;

    #ifdef IL2CPP_VER_GE_24_2
        /* Parallel vectors keyed by binary codeGenModule ordinal -- no
         name lookups (metadata names can be obfuscated, the ordinal
         alignment is the only thing we can rely on) */
        
        std::vector<std::string>           m_CodeGenModuleOrder_;
        std::vector<std::vector<uint64_t>> m_CodeGenModulePointers_;
    #endif
    };

    template <typename T>
    T CIl2Cpp::ReadAtVA(uint64_t m_Va)
    {
        uint64_t m_Off = p_Mach_->MapVATR(m_Va);
        CBinaryStream& m_S = p_Mach_->Stream();
        m_S.SeekTo(m_Off);
        T m_V{};
        m_V.Read(m_S);
        return m_V;
    }
}
