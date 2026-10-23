#ifndef ANIMATED_SPRITE_H
#define ANIMATED_SPRITE_H

#include "raylib.h"
#include <cstdio>
#include <string>
#include <vector>

// Loads numbered frames until the first missing file.
// Example pattern: "assets/enemies/worm/worm_frames/worm_%03d.png"
class AnimatedSprite {
public:
    std::vector<Texture2D> frames;
    int current = 0;
    float fps = 15.0f;
    float acc = 0.0f;
    bool loaded = false;

    AnimatedSprite() = default;
    AnimatedSprite(const AnimatedSprite&) = delete;
    AnimatedSprite& operator=(const AnimatedSprite&) = delete;

    bool LoadSequence(const std::string& pattern, int startIndex = 1, int maxFrames = 1000) {
        Unload();

        for (int i = startIndex; i < startIndex + maxFrames; i++) {
            char path[512];
            std::snprintf(path, sizeof(path), pattern.c_str(), i);

            if (!FileExists(path)) {
                break; // stop at first missing file
            }

            Texture2D t = LoadTexture(path);
            if (t.id == 0) {
                break; // file exists but failed to load
            }
            frames.push_back(t);
        }

        loaded = !frames.empty();
        current = 0;
        acc = 0.0f;
        return loaded;
    }

    void SetFPS(float f) { fps = (f <= 0.0f ? 15.0f : f); }

    // IMPORTANT: call ONCE per frame (not per enemy)
    void Update(float dt) {
        if (!loaded || frames.size() <= 1) return;

        acc += dt;
        const float spf = 1.0f / fps;

        // Prevent spiral if dt spikes
        if (acc > 0.25f) acc = 0.25f;

        while (acc >= spf) {
            acc -= spf;
            current = (current + 1) % (int)frames.size();
        }
    }

    void DrawCentered(Vector2 pos, float width, Color tint = WHITE) const {
        if (!loaded) return;

        const Texture2D& tex = frames[current];
        float aspect = (float)tex.height / (float)tex.width;
        float height = width * aspect;

        Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
        Rectangle dst = { pos.x, pos.y, width, height };
        Vector2 origin = { width / 2.0f, height / 2.0f };

        DrawTexturePro(tex, src, dst, origin, 0.0f, tint);
    }

    void Unload() {
        for (auto& t : frames) {
            if (t.id != 0) UnloadTexture(t);
        }
        frames.clear();
        loaded = false;
        current = 0;
        acc = 0.0f;
    }

    ~AnimatedSprite() { Unload(); }
};

#endif
