#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"
#include <filesystem>
#include <string>

// --- EKRAN VE COZUNURLUK YAPISI ---
struct GameResolution {
    int width;
    int height;
    const char* label;
};

// Mevcut secenekler
const GameResolution RES_720P = { 1280, 720, "1280x720" };
const GameResolution RES_900P = { 1600, 900, "1600x900" };
const GameResolution RES_1080P = { 1920, 1080, "1920x1080" };

// --- DINAMIK AYARLAR (Global erisim icin) ---
// Not: Bunlar baslangic degerleridir, oyun icinde degistirilebilir.
inline GameResolution currentRes = RES_900P; 
inline bool isFullscreen = false;

// --- OTOMATIK OLCEKLEME (Scaling) MANTIGI ---
// Tum oyun mantigi bu "Sanal" cozunurluge gore hesaplanacak.
const int VIRTUAL_WIDTH = 1600;
const int VIRTUAL_HEIGHT = 900;

// Haritanin ve objelerin ekrana tam oturmasi icin carpanlar
inline float GetScaleX() { return (float)GetScreenWidth() / VIRTUAL_WIDTH; }
inline float GetScaleY() { return (float)GetScreenHeight() / VIRTUAL_HEIGHT; }

// --- OYUNCU VE SISTEM STATLARI ---
const float PLAYER_STARTING_HEALTH = 100.0f;
const float STARTING_BITCOIN = 500.0f;

// --- RENK TEMASI (CYBERPUNK) ---
const Color CYBER_BG = { 5, 5, 12, 255 }; 
const Color CYBER_PATH = { 0, 255, 150, 60 };
const Color CYBER_ACCENT = { 0, 160, 255, 255 };
const Color CYBER_MALWARE = { 255, 45, 85, 255 };
const Color CYBER_RIVER = { 0, 80, 150, 100 }; // Nehir rengi
const Color CYBER_BRIDGE = { 60, 40, 30, 255 }; // Köprü rengi  

inline std::string GetAssetPath(const std::string& filename) {
    namespace fs = std::filesystem;
    fs::path start = fs::current_path();
    fs::path probe = start;

    for (int i = 0; i < 4; ++i) {
        fs::path candidates[] = {
            probe / "assets" / filename,
            probe / "bin" / "assets" / filename
        };

        for (const auto& p : candidates) {
            if (fs::exists(p)) return p.string();
        }

        if (probe.has_parent_path()) {
            probe = probe.parent_path();
        } else {
            break;
        }
    }

    return (start / filename).string();
}

#endif
