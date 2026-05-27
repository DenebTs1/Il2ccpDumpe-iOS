#pragma once

#include <CoreGraphics/CoreGraphics.h> // For CGPoint
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <string>

struct Vec2
{
  float x, y;
  Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

class Settings {
public:
    Vec2 MenuSize;
    Vec2 MenuPos;
    
    // variables related to hooking
    bool hpHackON = true;
    bool hpPhysicalShield_HackON = true;
    bool hpEnergyShield_HackON = true;
    
    // hide all
    bool StreamerMode = false;

    // ESP settings
    bool esp_line = false;
    bool esp_box_2d = false;
    bool esp_distance = false;
    bool esp_skeleton = false;
    bool esp_health = false;
    float esp_line_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float esp_box_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float esp_line_thickness = 1.0f;
    float esp_box_thickness = 1.0f;

    // Aimbot settings
    bool  aimbot            = false;
    bool  aimbot_fov_draw   = true;
    float aimbot_fov_radius = 150.0f;
    int   aimbot_mode       = 0;     // 0=FOV (crosshair), 1=Closest (distance), 2=Mid (balanced)
    int   aimbot_bone       = 0;     // 0=Head, 1=Neck, 2=Chest
    float aimbot_smooth     = 3.0f;  // 1=instant snap, higher=smoother
    bool  aimbot_vis_check  = true;
    float aimbot_fov_color[4] = { 1.0f, 1.0f, 1.0f, 0.8f };

    // Camera / FOV changer
    bool  fov_enabled   = false;
    float game_fov      = 90.0f;
    
    char Username[64];
    char Password[64];
    
public:
    static Settings &GetInstance()
    {
        static Settings Instance{};
        return Instance;
    }
    
    static inline std::filesystem::path FilePath = std::filesystem::path(std::string(getenv("HOME")) + "/Documents/Settings.txt");
    
    void Save()
    {
        std::ofstream OutFile(FilePath, std::ios::binary | std::ios::trunc);
        OutFile.write(reinterpret_cast<const char *>(this), sizeof(Settings));
        OutFile.close();
    }
    
    void Load()
    {
        if (std::filesystem::exists(FilePath)) {
            std::ifstream InFile(FilePath, std::ios::binary);
            InFile.read(reinterpret_cast<char *>(this), sizeof(Settings));
            InFile.close();
        }
    }
    
    void Reset()
    {
        *this = Settings{};
        return Save();
    }
    
private:
    Settings() : MenuSize(Vec2(390, 370)), MenuPos(Vec2(1200, 500)) {
        memset(Username, 0, 64);
        memset(Password, 0, 64);
    };
    ~Settings() {};
};
