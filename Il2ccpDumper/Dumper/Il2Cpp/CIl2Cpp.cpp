//
//  CIl2Cpp.cpp
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//

#include "CIl2Cpp.h"
#include "Versions.h"
#include "../Core/Log.h"
#include "../Utils/CSectionHelper.h"

#include <cstdint>

namespace Il2Dumper
{
    /* Pointer arrays in real Il2Cpp targets never reach this size; a
     garbage count from a mis-aligned struct read goes much higher */
    static constexpr uint64_t k_MaxSaneCount = 50ull * 1024 * 1024;

    CIl2Cpp::CIl2Cpp(CMachO64* p_Mach)
        : p_Mach_(p_Mach)
    {
    }

    bool CIl2Cpp::PlusSearch(int m_MethodCount, int m_TypeDefCount, int m_ImageCount)
    {
        std::vector<SSearchSection> m_Data;
        std::vector<SSearchSection> m_Exec;
        std::vector<SSearchSection> m_Bss;
        for (const auto& s : p_Mach_->Sections())
        {
            SSearchSection ss;
            ss.m_Offset     = s.m_Offset;
            ss.m_OffsetEnd  = s.m_Offset + s.m_Size;
            ss.m_Address    = s.m_Addr;
            ss.m_AddressEnd = s.m_Addr + s.m_Size;
            if (s.m_SectName == "__const" || s.m_SectName == "__cstring" || s.m_SectName == "__data")
            {
                m_Data.push_back(ss);
            }
            if (s.m_Flags == 0x80000400u) m_Exec.push_back(ss);
            if (s.m_Flags == 1u)          m_Bss.push_back(ss);
        }
        IL2D_LOG("PlusSearch: target=%g data=%zu exec=%zu bss=%zu",
                 (double)IL2CPP_TARGET_VERSION, m_Data.size(), m_Exec.size(), m_Bss.size());

        CSectionHelper m_Helper(p_Mach_, m_MethodCount, m_TypeDefCount,
                                m_MetadataUsagesCount_, m_ImageCount);
        m_Helper.SetSection(E_SectionKind::E_Exec, m_Exec);
        m_Helper.SetSection(E_SectionKind::E_Data, m_Data);
        m_Helper.SetSection(E_SectionKind::E_Bss,  m_Bss);

        uint64_t m_Cr = m_Helper.FindCodeRegistration();
        uint64_t m_Mr = m_Helper.FindMetadataRegistration();
        IL2D_LOG("PlusSearch: CR=0x%llx MR=0x%llx",
                 (unsigned long long)m_Cr, (unsigned long long)m_Mr);
        if (m_Cr == 0 || m_Mr == 0) return false;
        return Init(m_Cr, m_Mr);
    }

    bool CIl2Cpp::Init(uint64_t m_Cr, uint64_t m_Mr)
    {
        m_CodeReg_ = ReadAtVA<SIl2CppCodeRegistration>(m_Cr);

        /* PlusSearch returns refva3 - 14*ptrSize, which lands on the v29
         struct start. v31 is 16 bytes larger so the real start is two
         pointers earlier; v27.x is one pointer. Detect by sanity-
         checking the first count and back up if it looks like a slid
         pointer rather than a count */
        constexpr uint64_t k_PtrLimit = 0x50000;
    #ifdef IL2CPP_VER_31
        if (m_CodeReg_.m_GenericMethodPointersCount > k_PtrLimit)
        {
            m_Cr -= 16;
            IL2D_LOG("Init: v31 search-offset detected, retrying CR @ 0x%llx",
                     (unsigned long long)m_Cr);
            m_CodeReg_ = ReadAtVA<SIl2CppCodeRegistration>(m_Cr);
        }
    #endif
    #if defined(IL2CPP_VER_29) || defined(IL2CPP_VER_29_1)
        if (m_CodeReg_.m_GenericMethodPointersCount > k_PtrLimit)
        {
            m_Cr -= 16;
            IL2D_LOG("Init: v29 search-offset detected, retrying CR @ 0x%llx",
                     (unsigned long long)m_Cr);
            m_CodeReg_ = ReadAtVA<SIl2CppCodeRegistration>(m_Cr);
        }
    #endif
    #if defined(IL2CPP_VER_27) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27_2)
        if (m_CodeReg_.m_ReversePInvokeWrapperCount > k_PtrLimit)
        {
            m_Cr -= 8;
            IL2D_LOG("Init: v27 search-offset detected, retrying CR @ 0x%llx",
                     (unsigned long long)m_Cr);
            m_CodeReg_ = ReadAtVA<SIl2CppCodeRegistration>(m_Cr);
        }
    #endif

        m_MetaReg_ = ReadAtVA<SIl2CppMetadataRegistration>(m_Mr);

        // Scrub pointer fields we actually consume. Count fields stay as-is.
        auto m_Scrub = [&](uint64_t v) { return p_Mach_->ScrubVa(v); };
        m_MetaReg_.m_Types        = m_Scrub(m_MetaReg_.m_Types);
        m_MetaReg_.m_FieldOffsets = m_Scrub(m_MetaReg_.m_FieldOffsets);
    #ifdef IL2CPP_VER_GE_24_2
        m_CodeReg_.m_CodeGenModules = m_Scrub(m_CodeReg_.m_CodeGenModules);
    #endif

        CBinaryStream& m_S = p_Mach_->Stream();

        auto m_LoadPtrArray = [&](uint64_t m_Va, uint64_t m_Count, std::vector<uint64_t>& m_Out)
        {
            if (m_Count == 0) return;
            if (m_Count > k_MaxSaneCount)
            {
                IL2D_LOG("LoadPtrArray: refusing insane count %llu at VA 0x%llx",
                         (unsigned long long)m_Count, (unsigned long long)m_Va);
                return;
            }
            uint64_t m_Off = (m_Va != 0) ? p_Mach_->MapVATR(m_Va) : 0;
            if (m_Va != 0 && m_Off == 0) return;
            if (m_Off + m_Count * 8 > m_S.Length()) return;
            m_Out.resize(size_t(m_Count));
            m_S.SeekTo(m_Off);
            for (uint64_t i = 0; i < m_Count; ++i) m_Out[i] = p_Mach_->ReadVaPointer();
        };

        /* Field-offset table (pointer-of-pointers from v22 onward, flat
         u32 array on older builds) */
    #ifdef IL2CPP_VER_GE_22
        bFieldOffsetsArePointers_ = true;
    #else
        bFieldOffsetsArePointers_ = false;
    #endif
        if (bFieldOffsetsArePointers_)
        {
            m_LoadPtrArray(m_MetaReg_.m_FieldOffsets,
                           uint64_t(m_MetaReg_.m_FieldOffsetsCount),
                           m_FieldOffsetsPtrTable_);
        }
        else
        {
            m_FieldOffsetsValueTable_.resize(size_t(m_MetaReg_.m_FieldOffsetsCount));
            uint64_t m_Off = p_Mach_->MapVATR(m_MetaReg_.m_FieldOffsets);
            m_S.SeekTo(m_Off);
            for (int64_t i = 0; i < m_MetaReg_.m_FieldOffsetsCount; ++i)
            {
                m_FieldOffsetsValueTable_[size_t(i)] = m_S.ReadU32();
            }
        }

        /* Types -- m_TypeAddresses_[i] is the scrubbed VA for m_Types_[i];
         m_TypeDic_ reverse-maps VA -> index for the executor */
        
        m_LoadPtrArray(m_MetaReg_.m_Types,
                       uint64_t(m_MetaReg_.m_TypesCount),
                       m_TypeAddresses_);
        m_Types_.resize(m_TypeAddresses_.size());
        for (size_t i = 0; i < m_TypeAddresses_.size(); ++i)
        {
            uint64_t m_Off = p_Mach_->MapVATR(m_TypeAddresses_[i]);
            m_S.SeekTo(m_Off);
            m_Types_[i].Read(m_S);
            m_Types_[i].InitDecoded();
            m_TypeDic_[m_TypeAddresses_[i]] = i;
        }

        // codeGenModules (v24.2+ only) -- order matches metadata ImageDefs.
    #ifdef IL2CPP_VER_GE_24_2
        std::vector<uint64_t> m_ModPtrs;
        m_LoadPtrArray(m_CodeReg_.m_CodeGenModules,
                       m_CodeReg_.m_CodeGenModulesCount,
                       m_ModPtrs);
        m_CodeGenModuleOrder_.reserve(m_ModPtrs.size());
        m_CodeGenModulePointers_.reserve(m_ModPtrs.size());
        for (uint64_t m_Va : m_ModPtrs)
        {
            SIl2CppCodeGenModule m_Mod = ReadAtVA<SIl2CppCodeGenModule>(m_Va);
            m_Mod.m_ModuleName     = m_Scrub(m_Mod.m_ModuleName);
            m_Mod.m_MethodPointers = m_Scrub(m_Mod.m_MethodPointers);

            std::string m_Name = m_S.ReadStringToNull(p_Mach_->MapVATR(m_Mod.m_ModuleName));
            std::vector<uint64_t> m_Ptrs;
            m_LoadPtrArray(m_Mod.m_MethodPointers,
                           uint64_t(m_Mod.m_MethodPointerCount),
                           m_Ptrs);
            IL2D_LOG("CodeGenModule[%zu] '%s' methodPointers=%zu",
                     m_CodeGenModuleOrder_.size(), m_Name.c_str(), m_Ptrs.size());
            m_CodeGenModuleOrder_.push_back(std::move(m_Name));
            m_CodeGenModulePointers_.push_back(std::move(m_Ptrs));
        }
    #endif
        return true;
    }

#ifdef IL2CPP_VER_GE_24_2
    static std::string LowerAscii(const std::string& m_In)
    {
        std::string m_Out = m_In;
        for (char& c : m_Out)
        {
            if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
        }
        return m_Out;
    }
#endif

    void CIl2Cpp::BindImages(const std::vector<std::string>& m_ImageNames)
    {
    #ifdef IL2CPP_VER_GE_24_2
        const size_t m_ImgCount = m_ImageNames.size();
        const size_t m_ModCount = m_CodeGenModuleOrder_.size();

        m_ImageToModule_.assign(m_ImgCount, -1);

        std::vector<bool>                       m_Claimed(m_ModCount, false);
        std::unordered_map<std::string, size_t> m_Exact;
        std::unordered_map<std::string, size_t> m_Folded;
        m_Exact.reserve(m_ModCount * 2);
        m_Folded.reserve(m_ModCount * 2);
        for (size_t i = 0; i < m_ModCount; ++i)
        {
            // First writer wins: a duplicate module name means a broken build.
            m_Exact.emplace(m_CodeGenModuleOrder_[i], i);
            m_Folded.emplace(LowerAscii(m_CodeGenModuleOrder_[i]), i);
        }

        // Exact match first, ASCII-case-folded second. SIZE_MAX = no module.
        auto m_Lookup = [&](const std::string& m_Name) -> size_t
        {
            auto it = m_Exact.find(m_Name);
            if (it != m_Exact.end()) return it->second;
            auto it2 = m_Folded.find(LowerAscii(m_Name));
            if (it2 != m_Folded.end()) return it2->second;
            return SIZE_MAX;
        };

        size_t m_ByName = 0;
        for (size_t i = 0; i < m_ImgCount; ++i)
        {
            if (m_ImageNames[i].empty()) continue;

            const size_t m_Mod = m_Lookup(m_ImageNames[i]);
            if (m_Mod == SIZE_MAX || m_Claimed[m_Mod]) continue;

            m_ImageToModule_[i] = int(m_Mod);
            m_Claimed[m_Mod]    = true;
            ++m_ByName;
        }

        /* Obfuscated builds rename modules in the binary but not always in
         metadata (or the other way round). When exactly as many images as
         modules are left over, the only consistent completion is to pair the
         remainders in order -- that is also what the old ordinal-only path
         did, so nothing regresses for a fully-obfuscated target */

        std::vector<size_t> m_FreeImgs, m_FreeMods;
        for (size_t i = 0; i < m_ImgCount; ++i) if (m_ImageToModule_[i] < 0)  m_FreeImgs.push_back(i);
        for (size_t i = 0; i < m_ModCount; ++i) if (!m_Claimed[i])            m_FreeMods.push_back(i);

        if (!m_FreeImgs.empty() && m_FreeImgs.size() == m_FreeMods.size())
        {
            for (size_t k = 0; k < m_FreeImgs.size(); ++k)
            {
                m_ImageToModule_[m_FreeImgs[k]] = int(m_FreeMods[k]);
            }
        }

        IL2D_LOG("BindImages: %zu images, %zu codeGenModules -- %zu by name, %zu by leftover order, %zu unpaired",
                 m_ImgCount, m_ModCount, m_ByName,
                 (m_FreeImgs.size() == m_FreeMods.size()) ? m_FreeImgs.size() : size_t(0),
                 (m_FreeImgs.size() == m_FreeMods.size()) ? size_t(0) : m_FreeImgs.size());

        if (m_FreeImgs.size() != m_FreeMods.size())
        {
            IL2D_LOG("BindImages: %zu images left unpaired (%zu modules free) -- "
                     "their methods will have no RVA", m_FreeImgs.size(), m_FreeMods.size());
        }
    #else
        (void)m_ImageNames;
    #endif
    }

    int CIl2Cpp::ModuleIndexForImage(int m_ImageIdx) const
    {
    #ifdef IL2CPP_VER_GE_24_2
        if (m_ImageIdx < 0) return -1;

        // BindImages() never ran -- keep the historical ordinal behaviour.
        if (m_ImageToModule_.empty())
        {
            return (size_t(m_ImageIdx) < m_CodeGenModulePointers_.size()) ? m_ImageIdx : -1;
        }
        if (size_t(m_ImageIdx) >= m_ImageToModule_.size()) return -1;

        const int m_Mod = m_ImageToModule_[size_t(m_ImageIdx)];
        return (m_Mod >= 0 && size_t(m_Mod) < m_CodeGenModulePointers_.size()) ? m_Mod : -1;
    #else
        (void)m_ImageIdx;
        return -1;
    #endif
    }

    uint64_t CIl2Cpp::GetMethodPointerByImageIndex(int      m_ImageIdx,
                                                   uint32_t m_Token,
                                                   int32_t  m_MethodIndex) const
    {
    #ifdef IL2CPP_VER_GE_24_2
        (void)m_MethodIndex;
        const int m_ModIdx = ModuleIndexForImage(m_ImageIdx);
        if (m_ModIdx < 0) return 0;

        const auto& m_Ptrs = m_CodeGenModulePointers_[size_t(m_ModIdx)];
        uint32_t m_Idx = m_Token & 0x00FFFFFFu;
        if (m_Idx == 0 || m_Idx > uint32_t(m_Ptrs.size())) return 0;
        return m_Ptrs[m_Idx - 1];
    #else
        (void)m_ImageIdx; (void)m_Token; (void)m_MethodIndex;
        return 0;
    #endif
    }

    std::string CIl2Cpp::CodeGenModuleNameForImage(int m_ImageIdx) const
    {
    #ifdef IL2CPP_VER_GE_24_2
        const int m_ModIdx = ModuleIndexForImage(m_ImageIdx);
        if (m_ModIdx < 0) return "<unpaired>";
        return m_CodeGenModuleOrder_[size_t(m_ModIdx)];
    #else
        (void)m_ImageIdx;
        return "<n/a>";
    #endif
    }

    size_t CIl2Cpp::CodeGenModuleMethodCount(int m_ImageIdx) const
    {
    #ifdef IL2CPP_VER_GE_24_2
        const int m_ModIdx = ModuleIndexForImage(m_ImageIdx);
        if (m_ModIdx < 0) return 0;
        return m_CodeGenModulePointers_[size_t(m_ModIdx)].size();
    #else
        (void)m_ImageIdx;
        return 0;
    #endif
    }

    int CIl2Cpp::GetFieldOffsetFromIndex(int m_TypeIndex, int m_FieldIndexInType,
                                         int m_FieldIndex, bool b_ValueType, bool b_Static)
    {
        int m_Offset = -1;
        if (bFieldOffsetsArePointers_)
        {
            if (m_TypeIndex < 0 || size_t(m_TypeIndex) >= m_FieldOffsetsPtrTable_.size()) return -1;
            uint64_t m_Ptr = m_FieldOffsetsPtrTable_[size_t(m_TypeIndex)];
            if (m_Ptr == 0) return -1;
            uint64_t m_Off = p_Mach_->MapVATR(m_Ptr) + 4ull * uint64_t(m_FieldIndexInType);
            CBinaryStream& m_S = p_Mach_->Stream();
            m_S.SeekTo(m_Off);
            m_Offset = m_S.ReadI32();
        }
        else
        {
            if (m_FieldIndex < 0 || size_t(m_FieldIndex) >= m_FieldOffsetsValueTable_.size()) return -1;
            m_Offset = int(m_FieldOffsetsValueTable_[size_t(m_FieldIndex)]);
        }
        if (m_Offset > 0 && b_ValueType && !b_Static)
        {
            m_Offset -= p_Mach_->Stream().Is32Bit() ? 8 : 16;
        }
        return m_Offset;
    }
}
