#define IMGUI_DEFINE_MATH_OPERATORS
#include "UserMenu.h"
#import "../MenuLoad/ImGuiDrawView.h"

#include "../Dumper/Il2Dumper.h"
#include "../Dumper/Il2Cpp/Versions.h"

#import <mach-o/dyld.h>
#import <mach-o/loader.h>

#include <atomic>
#include <string>
#include <sys/stat.h>

extern ImFont *medium;
extern ImFont *boldFont;
extern ImFont *tab_icons;
extern ImFont *logo;
extern ImFont *tab_title;
extern ImFont *tab_title_icon;
extern ImFont *subtab_title;
extern ImFont *combo_arrow;

extern ImTextureID gMenuLogoTexture;

static bool showMainWindow = true;

using namespace ImGui;


void AddRectFilledMultiColorRounded(ImDrawList *dl,const ImVec2 &p_min, const ImVec2 &p_max, ImU32 bg_color, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left,
        float rounding, ImDrawFlags rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(p_max.x - p_min.x) * (((rounding_corners & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) ||
             ((rounding_corners & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom)
             ? 0.5f : 1.0f) - 1.0f);
    rounding = ImMin(rounding, ImFabs(p_max.y - p_min.y) * (((rounding_corners & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((rounding_corners & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        dl->AddRectFilledMultiColor(p_min, p_max,
            col_upr_left, col_upr_right, col_bot_right, col_bot_left);
        return;
    }

    const float rounding_tl = (rounding_corners & ImDrawFlags_RoundCornersTopLeft)     ? rounding : 0.0f;
    const float rounding_tr = (rounding_corners & ImDrawFlags_RoundCornersTopRight)    ? rounding : 0.0f;
    const float rounding_br = (rounding_corners & ImDrawFlags_RoundCornersBottomRight) ? rounding : 0.0f;
    const float rounding_bl = (rounding_corners & ImDrawFlags_RoundCornersBottomLeft)  ? rounding : 0.0f;

    const ImVec2 uv = dl->_Data->TexUvWhitePixel;
    dl->PrimReserve(6, 4);
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx));
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx + 1));
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx + 2));
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx));
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx + 2));
    dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx + 3));
    dl->PrimWriteVtx(p_min, uv, col_upr_left);
    dl->PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    dl->PrimWriteVtx(p_max, uv, col_bot_right);
    dl->PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);

    dl->PathLineTo(p_min);
    dl->PathArcTo(ImVec2(p_min.x + rounding_tl, p_min.y + rounding_tl), rounding_tl, 4.820f, 3.100f);
    dl->PathFillConvex(bg_color);
    dl->PathLineTo(ImVec2(p_max.x, p_min.y));
    dl->PathArcTo(ImVec2(p_max.x - rounding_tr, p_min.y + rounding_tr),
                      rounding_tr, 6.3400f, 4.620f);
    dl->PathFillConvex(bg_color);
    dl->PathLineTo(ImVec2(p_max.x, p_max.y));
    dl->PathArcTo(ImVec2(p_max.x - rounding_br, p_max.y - rounding_br),
                      rounding_br, 7.960f, 6.240f);
    dl->PathFillConvex(bg_color);

    dl->PathLineTo(ImVec2(p_min.x, p_max.y));
    dl->PathArcTo(ImVec2(p_min.x + rounding_bl, p_max.y - rounding_bl),
                      rounding_bl, 9.5f, 7.770f);
    dl->PathFillConvex(bg_color);
}

struct tab_element { float element_opacity, rect_opacity, text_opacity; };

bool elem_tab(const char *name, bool boolean, bool has_active = false)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(name);
    const ImVec2 label_size = tab_icons->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, name);
    ImVec2 pos = window->DC.CursorPos;

    const ImRect rect(pos, ImVec2(pos.x + 31, pos.y + 31));
    ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y + 5), style.FramePadding.y);
    if (!ItemAdd(rect, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held, 0);

    static std::map<ImGuiID, tab_element> anim;
    auto it = anim.find(id);
    if (it == anim.end()) { anim.insert({id, {0,0,0}}); it = anim.find(id); }

    it->second.element_opacity = ImLerp(it->second.element_opacity,
            (boolean ? 0.04f : hovered ? 0.01f : 0.0f),
            0.07f * (1.0f - GetIO().DeltaTime));
    it->second.rect_opacity = ImLerp(it->second.rect_opacity,
            (boolean ? 1.0f : 0.0f),
            0.15f * (1.0f - GetIO().DeltaTime));
    it->second.text_opacity = ImLerp(it->second.text_opacity,
            (boolean ? 1.0f : hovered ? 0.5f : 0.3f),
            0.07f * (1.0f - GetIO().DeltaTime));

    window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(1.0f, 1.0f, 1.0f, it->second.element_opacity), 3.0f);
    window->DrawList->AddText(tab_icons, 15.0f, (rect.Min + rect.Max - label_size) / 2, ImColor(1.0f, 1.0f, 1.0f, it->second.text_opacity), name);

    window->DrawList->AddRectFilled(
            ImVec2(rect.Max.x + 4, rect.Min.y + 6),
            ImVec2(rect.Max.x + 8, rect.Max.y - 6),
            ImColor(75 / 255.0f, 195 / 255.0f, 220 / 255.0f, it->second.rect_opacity), 7.0f, ImDrawFlags_RoundCornersLeft);
    if (has_active && !boolean)
    {
        window->DrawList->AddCircleFilled(
            ImVec2(rect.Max.x - 4.5f, rect.Min.y + 4.5f), 2.4f,
            ImColor(75, 195, 220, 235));
    }

    return pressed;
}

static void section_header(const char *text)
{
    ImGui::Dummy(ImVec2(0, 4));
    ImVec2 p = ImGui::GetCursorScreenPos();
    auto  *dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + 3, p.y + 14), ImColor(75, 195, 220), 1.5f);
    ImGui::Dummy(ImVec2(10, 0));
    ImGui::SameLine();
    ImGui::PushFont(boldFont);
    ImGui::TextColored(ImColor(190, 225, 240, 255).Value, "%s", text);
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 2));
}

static void hint_text(const char *text)
{
    ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.45f), "%s", text);
}

bool elem_begin_child_ex(const char *name, ImGuiID id, const ImVec2 &size_arg,
                         bool border, ImGuiWindowFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *parent_window = g.CurrentWindow;

    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
    flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);

    const ImVec2 content_avail = GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises =
        ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) |
        ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
    if (size.x <= 0.0f) size.x = ImMax(content_avail.x + size.x, 4.0f);
    if (size.y <= 0.0f) size.y = ImMax(content_avail.y + size.y, 4.0f);

    SetNextWindowPos(ImVec2(parent_window->DC.CursorPos.x,
                            parent_window->DC.CursorPos.y + 34.0f));
    SetNextWindowSize(size - ImVec2(0, 36));

    parent_window->DrawList->AddRectFilled(parent_window->DC.CursorPos,
        parent_window->DC.CursorPos + ImVec2(size.x, size.y),
        ImColor(24, 24, 26), 4.0f);
    parent_window->DrawList->AddRect(parent_window->DC.CursorPos,
        parent_window->DC.CursorPos + ImVec2(size.x, size.y),
        ImColor(1.0f, 1.0f, 1.0f, 0.03f), 4.0f);
    parent_window->DrawList->AddLine(
        parent_window->DC.CursorPos + ImVec2(1, 32),
        parent_window->DC.CursorPos + ImVec2(size.x - 1, 32),
        ImColor(1.0f, 1.0f, 1.0f, 0.03f));

    AddRectFilledMultiColorRounded(parent_window->DrawList,
        parent_window->DC.CursorPos,
        parent_window->DC.CursorPos + ImVec2(size.x - 115, 32),
        ImColor(24, 24, 26),
        ImColor(75, 195, 220, 50), ImColor(75, 195, 220, 0),
        ImColor(75, 195, 220, 0),  ImColor(75, 195, 220, 50),
        4.0f, ImDrawFlags_RoundCornersTopLeft);

    parent_window->DrawList->AddText(boldFont, 15.0f,
        parent_window->DC.CursorPos + ImVec2(16, 9),
        ImColor(75, 195, 220), name);

    const char *temp_window_name;
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL,
            "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL,
            "%s/%08X", parent_window->Name, id);

    const float backup_border_size = g.Style.ChildBorderSize;
    if (!border) g.Style.ChildBorderSize = 0.0f;
    bool ret = Begin(temp_window_name, NULL, flags);
    g.Style.ChildBorderSize = backup_border_size;

    ImGuiWindow *child_window = g.CurrentWindow;
    child_window->ChildId = id;

    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) &&
        (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(id + 1, child_window);
        g.ActiveIdSource = g.NavInputSource;
    }
    return ret;
}

bool elem_begin_child(const char *str_id, const ImVec2 &size_arg = ImVec2(0,0),
                      bool border = false, ImGuiWindowFlags extra_flags = 0)
{
    ImGuiWindow *window = GetCurrentWindow();
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 11));
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
    return elem_begin_child_ex(str_id, window->GetID(str_id), size_arg, border,
        extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoMove);
}

void elem_end_child()
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    IM_ASSERT(g.WithinEndChild == false);
    IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);

    g.WithinEndChild = true;
    if (window->BeginCount > 1)
    {
        End();
    }
    else
    {
        ImVec2 sz = window->Size;
        End();

        ImGuiWindow *parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
        ItemSize(sz);
        if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavWindowHasScrollY) &&
            !(window->Flags & ImGuiWindowFlags_NavFlattened))
        {
            ItemAdd(bb, window->ChildId);
            RenderNavHighlight(bb, window->ChildId);
            if (window->DC.NavLayersActiveMask == 0 && window == g.NavWindow)
                RenderNavHighlight(ImRect(bb.Min - ImVec2(2,2), bb.Max + ImVec2(2,2)),
                    g.NavId, ImGuiNavHighlightFlags_TypeThin);
        }
        else
        {
            ItemAdd(bb, 0);
        }
        if (g.HoveredWindow == window)
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
    }
    PopStyleVar(2);
    g.WithinEndChild = false;
    g.LogLinePosY = -FLT_MAX;
}

enum class E_DumpState : int
{
    E_Idle    = 0,
    E_Running = 1,
    E_Ok      = 2,
    E_Failed  = 3
};

enum class E_DumpSource : int
{
    E_Memory = 0,
    E_Files  = 1
};

struct SDumpController
{
    std::atomic<int>    State    { int(E_DumpState::E_Idle) };
    std::atomic<int>    LastCode { 0 };
    std::string         OutputDir;
    std::string         LastError;
    E_DumpSource        Source { E_DumpSource::E_Memory };

    char Il2cppPathBuf[256]   { 0 };
    char MetadataPathBuf[256] { 0 };
};

static SDumpController& Dumper()
{
    static SDumpController s_State;
    return s_State;
}

static bool FindUnityFrameworkImage(const void*& p_OutHeader, size_t& m_OutSize)
{
    const uint32_t m_Count = _dyld_image_count();
    for (uint32_t i = 0; i < m_Count; ++i)
    {
        const char* p_Name = _dyld_get_image_name(i);
        if (!p_Name) continue;
        if (!strstr(p_Name, "UnityFramework") && !strstr(p_Name, "libil2cpp")) continue;

        const struct mach_header_64* p_Hdr =
            reinterpret_cast<const struct mach_header_64*>(_dyld_get_image_header(i));
        if (!p_Hdr) continue;

        // Sum every LC_SEGMENT_64 vmsize to get the image's mapped span.
        const uint8_t* p_Cmd = reinterpret_cast<const uint8_t*>(p_Hdr) + sizeof(struct mach_header_64);
        size_t m_Total = sizeof(struct mach_header_64);
        for (uint32_t c = 0; c < p_Hdr->ncmds; ++c)
        {
            const struct load_command* p_Lc =
                reinterpret_cast<const struct load_command*>(p_Cmd);
            if (p_Lc->cmd == LC_SEGMENT_64)
            {
                const struct segment_command_64* p_Seg =
                    reinterpret_cast<const struct segment_command_64*>(p_Lc);
                m_Total = std::max<size_t>(m_Total, (size_t)p_Seg->fileoff + (size_t)p_Seg->filesize);
            }
            p_Cmd += p_Lc->cmdsize;
        }
        p_OutHeader = p_Hdr;
        m_OutSize   = m_Total;
        return true;
    }
    return false;
}

// Try to find global-metadata.dat under the app bundle's Data/Managed/Metadata.
static NSString* FindMetadataPathInBundle()
{
    NSString* p_Default = [[NSBundle mainBundle]
        pathForResource:@"global-metadata"
                 ofType:@"dat"
            inDirectory:@"Data/Managed/Metadata"];
    if (p_Default) return p_Default;

    // Fallback: scan UnityFramework.framework if it lives in the bundle.
    NSURL* p_FwUrl = [[NSBundle mainBundle] privateFrameworksURL];
    NSString* p_Try = [[p_FwUrl URLByAppendingPathComponent:
        @"UnityFramework.framework/Data/Managed/Metadata/global-metadata.dat"] path];
    if ([[NSFileManager defaultManager] fileExistsAtPath:p_Try])
    {
        return p_Try;
    }
    return nil;
}

static void StartDumpAsync()
{
    SDumpController& m_Dump = Dumper();
    m_Dump.State.store(int(E_DumpState::E_Running));
    m_Dump.LastError.clear();

    const E_DumpSource m_Src = m_Dump.Source;
    std::string m_Il2cppPath = m_Dump.Il2cppPathBuf;
    std::string m_MetaPath   = m_Dump.MetadataPathBuf;

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        Il2Dumper::SDumpSources m_DumpSrc;
        Il2Dumper::SDumpConfig  m_Cfg;

        if (m_Src == E_DumpSource::E_Memory)
        {
            const void* p_Hdr = nullptr;
            size_t      m_Sz  = 0;
            if (!FindUnityFrameworkImage(p_Hdr, m_Sz))
            {
                m_Dump.State.store(int(E_DumpState::E_Failed));
                m_Dump.LastError = "UnityFramework not found in dyld image list";
                return;
            }
            m_DumpSrc.p_Il2cppMemory = p_Hdr;
            m_DumpSrc.m_Il2cppSize   = m_Sz;
            m_DumpSrc.m_ImageBase    = (uint64_t)p_Hdr;

            NSString* p_MetaNs = FindMetadataPathInBundle();
            if (!p_MetaNs)
            {
                m_Dump.State.store(int(E_DumpState::E_Failed));
                m_Dump.LastError = "global-metadata.dat not found in app bundle";
                return;
            }
            m_DumpSrc.m_MetadataPath = [p_MetaNs UTF8String];
        }
        else
        {
            if (m_Il2cppPath.empty() || m_MetaPath.empty())
            {
                m_Dump.State.store(int(E_DumpState::E_Failed));
                m_Dump.LastError = "fill both file paths first";
                return;
            }
            m_DumpSrc.m_Il2cppPath   = m_Il2cppPath;
            m_DumpSrc.m_MetadataPath = m_MetaPath;
        }

        Il2Dumper::E_Status m_St = Il2Dumper::Run(m_DumpSrc, m_Cfg);
        m_Dump.LastCode.store(int(m_St));
        m_Dump.OutputDir = Il2Dumper::DefaultOutputDir();
        m_Dump.State.store(m_St == Il2Dumper::E_Status::E_Ok
                             ? int(E_DumpState::E_Ok)
                             : int(E_DumpState::E_Failed));
    });
}


enum heads { DUMP, INFO, OUTPUTS, SETTING };


void UserMenu::RenderMenu()
{
    if (!showMainWindow) return;

    Settings &S = Settings::GetInstance();
    SDumpController& m_Dump = Dumper();

    static heads tab { DUMP };

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 init_size(730, 460);
    if (init_size.x > io.DisplaySize.x - 16) init_size.x = io.DisplaySize.x - 16;
    if (init_size.y > io.DisplaySize.y - 16) init_size.y = io.DisplaySize.y - 16;

    ImGui::PushStyleColor(ImGuiCol_WindowBg,             ImColor(24,  24,  26         ).Value);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,              ImColor(24,  24,  26         ).Value);
    ImGui::PushStyleColor(ImGuiCol_PopupBg,              ImColor(28,  28,  30         ).Value);
    ImGui::PushStyleColor(ImGuiCol_Border,               ImColor(1.f, 1.f, 1.f, 0.06f).Value);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow,         ImColor(0,   0,   0,  0      ).Value);
    ImGui::PushStyleColor(ImGuiCol_Text,                 ImColor(1.f, 1.f, 1.f        ).Value);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled,         ImColor(1.f, 1.f, 1.f, 0.30f).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,              ImColor(32,  32,  36         ).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,       ImColor(40,  40,  46         ).Value);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,        ImColor(50,  50,  56         ).Value);
    ImGui::PushStyleColor(ImGuiCol_CheckMark,            ImColor(75,  195, 220        ).Value);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,           ImColor(75,  195, 220        ).Value);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive,     ImColor(120, 215, 235        ).Value);
    ImGui::PushStyleColor(ImGuiCol_Button,               ImColor(32,  32,  36         ).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,        ImColor(75,  195, 220, 50    ).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,         ImColor(75,  195, 220, 110   ).Value);
    ImGui::PushStyleColor(ImGuiCol_Header,               ImColor(75,  195, 220, 40    ).Value);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,        ImColor(75,  195, 220, 60    ).Value);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,         ImColor(75,  195, 220, 100   ).Value);
    ImGui::PushStyleColor(ImGuiCol_Separator,            ImColor(1.f, 1.f, 1.f, 0.06f).Value);
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered,     ImColor(75,  195, 220, 120   ).Value);
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive,      ImColor(75,  195, 220        ).Value);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,          ImColor(24,  24,  26, 0      ).Value);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,        ImColor(75,  195, 220, 70    ).Value);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImColor(75,  195, 220, 100   ).Value);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,  ImColor(75,  195, 220        ).Value);
    ImGui::PushStyleColor(ImGuiCol_TitleBg,              ImColor(24,  24,  26         ).Value);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,        ImColor(24,  24,  26         ).Value);
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed,     ImColor(24,  24,  26         ).Value);
    ImGui::PushStyleColor(ImGuiCol_NavHighlight,         ImColor(75,  195, 220        ).Value);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,     4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding,      4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,   0.0f);

    ImGui::SetNextWindowSize(init_size, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - init_size.x) * 0.5f,
                                   (io.DisplaySize.y - init_size.y) * 0.5f),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(380, 280),
                                        ImVec2(io.DisplaySize.x - 16,
                                               io.DisplaySize.y - 16));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (ImGui::Begin("Il2cpp Dumper", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar))
    {
        auto draw  = ImGui::GetWindowDrawList();
        auto pos   = ImGui::GetWindowPos();
        auto size  = ImGui::GetWindowSize();
        ImGuiStyle style = ImGui::GetStyle();

        draw->AddRectFilled(pos, ImVec2(pos.x + 47, pos.y + size.y),
                            ImColor(24, 24, 26), style.WindowRounding,
                            ImDrawFlags_RoundCornersLeft);
        draw->AddLine(ImVec2(pos.x + 47, pos.y + 2),
                      ImVec2(pos.x + 47, pos.y + size.y - 2),
                      ImColor(1.0f, 1.0f, 1.0f, 0.03f));
        draw->AddLine(ImVec2(pos.x + 2, pos.y + 47),
                      ImVec2(pos.x + 47, pos.y + 47),
                      ImColor(1.0f, 1.0f, 1.0f, 0.03f));

        if (gMenuLogoTexture)
        {
            const float iconSz = 30.0f;
            ImVec2 iconMin(pos.x + 8, pos.y + 8);
            ImVec2 iconMax(pos.x + 8 + iconSz, pos.y + 8 + iconSz);
            draw->AddImageRounded(gMenuLogoTexture, iconMin, iconMax,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 6.0f);
        }
        else
        {
            draw->AddText(boldFont, 18.0f, ImVec2(pos.x + 10, pos.y + 14),
                          ImColor(75, 195, 220), "EX");
        }

        draw->AddRect(pos + ImVec2(1, 1), pos + size - ImVec2(1, 1),
                      ImColor(1.0f, 1.0f, 1.0f, 0.03f), style.WindowRounding);

        ImGui::SetCursorPos(ImVec2(8, 56));
        ImGui::BeginGroup();
        {
            const int  m_St       = m_Dump.State.load();
            const bool b_Running  = (m_St == int(E_DumpState::E_Running));
            const bool b_HasOut   = (m_St == int(E_DumpState::E_Ok));
            const bool b_HasErr   = (m_St == int(E_DumpState::E_Failed));

            if (elem_tab("D", tab == DUMP,    b_Running))         { tab = DUMP;    }
            if (elem_tab("B", tab == INFO,    b_HasOut))          { tab = INFO;    }
            if (elem_tab("A", tab == OUTPUTS, b_HasOut))          { tab = OUTPUTS; }
            if (elem_tab("E", tab == SETTING, S.StreamerMode))    { tab = SETTING; }
            (void)b_HasErr;
        }
        ImGui::EndGroup();

        const float  card_x = 52.0f;
        const ImVec2 card_size(size.x - card_x - 8.0f, size.y - 16.0f);

        switch (tab)
        {
        case DUMP:
            ImGui::SetCursorPos(ImVec2(card_x, 8));
            elem_begin_child("Dump", card_size);
            {
                section_header("Source");

                int m_SrcInt = int(m_Dump.Source);
                static const char* k_Sources[] = { "Memory (auto-detect)", "Files (custom paths)" };
                if (ImGui::Combo("Mode", &m_SrcInt, k_Sources, IM_ARRAYSIZE(k_Sources)))
                {
                    m_Dump.Source = E_DumpSource(m_SrcInt);
                }

                if (m_Dump.Source == E_DumpSource::E_Memory)
                {
                    hint_text("Scans dyld for UnityFramework / libil2cpp and uses the app bundle's global-metadata.dat.");
                }
                else
                {
                    ImGui::InputText("il2cpp",   m_Dump.Il2cppPathBuf,   sizeof(m_Dump.Il2cppPathBuf));
                    ImGui::InputText("metadata", m_Dump.MetadataPathBuf, sizeof(m_Dump.MetadataPathBuf));
                    hint_text("Absolute paths on the device, e.g. /var/containers/...");
                }

                section_header("Action");

                const int m_St = m_Dump.State.load();
                const bool b_Running = (m_St == int(E_DumpState::E_Running));

                ImGui::BeginDisabled(b_Running);
                if (ImGui::Button(b_Running ? "Dumping..." : "Run Dump", ImVec2(160, 32)))
                {
                    StartDumpAsync();
                }
                ImGui::EndDisabled();

                ImGui::SameLine();
                switch (m_St)
                {
                case int(E_DumpState::E_Idle):
                    ImGui::TextColored(ImColor(1.f, 1.f, 1.f, 0.5f).Value, "  idle");
                    break;
                case int(E_DumpState::E_Running):
                    ImGui::TextColored(ImColor(75, 195, 220).Value, "  running...");
                    break;
                case int(E_DumpState::E_Ok):
                    ImGui::TextColored(ImColor(120, 215, 150).Value, "  ok");
                    break;
                case int(E_DumpState::E_Failed):
                    ImGui::TextColored(ImColor(235, 110, 110).Value, "  failed (code %d)", m_Dump.LastCode.load());
                    break;
                }

                if (!m_Dump.LastError.empty())
                {
                    ImGui::Dummy(ImVec2(0, 4));
                    ImGui::TextColored(ImColor(235, 110, 110).Value, "%s", m_Dump.LastError.c_str());
                }
                if (m_St == int(E_DumpState::E_Ok) && !m_Dump.OutputDir.empty())
                {
                    ImGui::Dummy(ImVec2(0, 4));
                    section_header("Output");
                    ImGui::TextWrapped("%s", m_Dump.OutputDir.c_str());
                    hint_text("Pull files via Xcode Devices > Download Container, or Files.app if UIFileSharingEnabled.");
                }
            }
            elem_end_child();
            break;

        case INFO:
            ImGui::SetCursorPos(ImVec2(card_x, 8));
            elem_begin_child("Info", card_size);
            {
                section_header("Target");
                ImGui::Text("Compile-time Il2Cpp version : %g", (double)IL2CPP_TARGET_VERSION);

                const void* p_Hdr = nullptr;
                size_t      m_Sz  = 0;
                if (FindUnityFrameworkImage(p_Hdr, m_Sz))
                {
                    ImGui::Text("UnityFramework base         : %p", p_Hdr);
                    ImGui::Text("UnityFramework span         : %.2f MiB", double(m_Sz) / (1024.0 * 1024.0));
                }
                else
                {
                    hint_text("UnityFramework not detected in dyld image list.");
                }

                NSString* p_Meta = FindMetadataPathInBundle();
                if (p_Meta)
                {
                    ImGui::Spacing();
                    section_header("Metadata file");
                    ImGui::TextWrapped("%s", [p_Meta UTF8String]);
                }
            }
            elem_end_child();
            break;

        case OUTPUTS:
            ImGui::SetCursorPos(ImVec2(card_x, 8));
            elem_begin_child("Outputs", card_size);
            {
                section_header("Generated files");
                if (m_Dump.OutputDir.empty())
                {
                    hint_text("Run a dump first to see generated files.");
                }
                else
                {
                    static const char* k_Files[] = {
                        "dump.cs",
                        "il2cpp.h",
                        "script.json",
                        "stringliteral.json"
                    };
                    for (const char* p_F : k_Files)
                    {
                        std::string m_Full = m_Dump.OutputDir + p_F;
                        struct stat m_St;
                        if (::stat(m_Full.c_str(), &m_St) == 0)
                        {
                            ImGui::Text("%-22s  %.1f KB", p_F, double(m_St.st_size) / 1024.0);
                        }
                        else
                        {
                            ImGui::TextColored(ImColor(1.f, 1.f, 1.f, 0.4f).Value,
                                               "%-22s  (missing)", p_F);
                        }
                    }
                    ImGui::Dummy(ImVec2(0, 4));
                    section_header("Output directory");
                    ImGui::TextWrapped("%s", m_Dump.OutputDir.c_str());
                }
            }
            elem_end_child();
            break;

        case SETTING:
            ImGui::SetCursorPos(ImVec2(card_x, 8));
            elem_begin_child("Settings", card_size);
            {
                section_header("Privacy");
                ImGui::Checkbox("Streamer Mode", &S.StreamerMode);
                hint_text("Hides identifying overlays while streaming.");

                section_header("Configuration");
                if (ImGui::Button("Save", ImVec2(90, 0)))  S.Save();  ImGui::SameLine();
                if (ImGui::Button("Load", ImVec2(90, 0)))  S.Load();  ImGui::SameLine();
                if (ImGui::Button("Reset", ImVec2(90, 0))) S.Reset();
            }
            elem_end_child();
            break;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(30);

    if (!showMainWindow && [ImGuiDrawView isMenuShowing])
        [ImGuiDrawView showChange:NO];
}

void UserMenu::SetVisible(bool visible) { showMainWindow = visible; }
bool UserMenu::IsVisible() const        { return showMainWindow; }
