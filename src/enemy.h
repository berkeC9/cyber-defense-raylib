#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include "raylib.h"
#include "raymath.h"
#include "animated_sprite.h"
#include <vector>

// 1. Düşman Tiplerini Tanımlıyoruz
enum class EnemyType { WORM, SPYWARE, TROJAN, BOSS };

class Enemy : public Entity {
public:
    EnemyType type;
    float health, maxHealth, speed, damage;
    int reward;
    float slowTimer = 0.0f; // %50 yavaşlatma süresi
    float stunTimer = 0.0f; // Seviye 4 için sersemletme süresi
    float armorTimer = 0.0f; // Lvl 4 Armor Pierce için

    int targetIndex = 1;
    std::vector<Vector2> waypoints;
    bool reachedEnd = false;

    // --- TEXTURE REFERANSLARI (main'de set edilecek) ---
    static Texture2D* texWorm;
    static Texture2D* texSpyware;
    static Texture2D* texTrojan;

    static void SetTextures(Texture2D* worm, Texture2D* spyware, Texture2D* trojan) {
        texWorm = worm;
        texSpyware = spyware;
        texTrojan = trojan;
    }

    // --- ANİMASYON REFERANSLARI (opsiyonel) ---
    static AnimatedSprite* wormAnim;
    static AnimatedSprite* spywareAnim;
    static AnimatedSprite* trojanAnim;

    static void SetWormAnim(AnimatedSprite* a) { wormAnim = a; }
    static void SetSpywareAnim(AnimatedSprite* a) { spywareAnim = a; }
    static void SetTrojanAnim(AnimatedSprite* a) { trojanAnim = a; }

    Enemy(std::vector<Vector2> path, EnemyType t, int wave) : Entity(path[0]), waypoints(path), type(t) {
    // --- KADEMELIİ ZORLUK MANTIĞI ---
    
    // 1. Her 5 dalgada bir %10 artış (Küçük adım)
    int tier5 = (wave - 1) / 5;
    float smallBoost = tier5 * 0.10f;

    // 2. Her 10 dalgada bir EKSTRA %25 artış (Büyük sıçrama)
    int tier10 = (wave - 1) / 10;
    float largeBoost = tier10 * 0.25f;

    // Toplam çarpan: Temel (1.0) + Küçük Artışlar + Büyük Artışlar
    float multiplier = 1.0f + smallBoost + largeBoost;

    switch (type) {
        case EnemyType::WORM:    
            health = 20.0f * multiplier;  speed = 150.0f; damage = 5.0f;  reward = 10;  
            break;
        case EnemyType::SPYWARE: 
            health = 60.0f * multiplier;  speed = 100.0f; damage = 15.0f; reward = 25;  
            break;
        case EnemyType::TROJAN:  
            health = 250.0f * multiplier; speed = 50.0f;  damage = 40.0f; reward = 100; 
            break;
        case EnemyType::BOSS:    
            // Bosslar için çarpanı daha da agresif hale getirebilirsin
            health = 1500.0f * multiplier * (1.0f + tier10 * 0.5f); 
            speed = 40.0f; damage = 50.0f; reward = 500; 
            break;
    }
    maxHealth = health;
}

    void update(float deltaTime) override {
    if (!active || reachedEnd) return;

        if (targetIndex >= (int)waypoints.size()) {
            reachedEnd = true;
            active = false;
            return;
        }
        // --- HIZ HESAPLAMA ---
        if (armorTimer > 0) armorTimer -= deltaTime;
        float currentSpeed = speed;
        if (stunTimer > 0) {
            currentSpeed = 0; // Seviye 4: Tamamen durma
            stunTimer -= deltaTime;
        } else if (slowTimer > 0) {
            currentSpeed *= 0.5f; // %50 Yavaşlatma
            slowTimer -= deltaTime;
        }

        
    Vector2 target = waypoints[targetIndex];
    float distanceToTarget = Vector2Distance(position, target);

    // DÜZELTME: 2.0f olan değeri 10.0f veya hıza bağlı bir değere yükseltiyoruz
    // Hızlandırma (timeScale) aktifken düşmanın noktayı ıskalamasını önler
    if (distanceToTarget < (currentSpeed * deltaTime + 5.0f)) { 
        targetIndex++;
    } else {
        Vector2 direction = Vector2Normalize(Vector2Subtract(target, position));
        position =
            Vector2Add(position, Vector2Scale(direction, currentSpeed * deltaTime));
    }

    if (health <= 0) active = false;
}

    void draw() override {
    if (!active) return;

    // 1) Ölçek çarpanlarını al
    float sx = GetScaleX();
    float sy = GetScaleY();
    
    // Ölçeklenmiş pozisyon (Ekran üzerindeki gerçek piksel konumu)
    Vector2 sPos = { position.x * sx, position.y * sy };

    // --- 1) Animasyon Çizimi (WORM, SPYWARE, TROJAN) ---
    if (type == EnemyType::WORM && wormAnim && wormAnim->loaded) {
        float size = 64.0f * sx; // Boyutu da ölçekle
        wormAnim->DrawCentered(sPos, size, WHITE);
    } 
    else if (type == EnemyType::SPYWARE && spywareAnim && spywareAnim->loaded) {
        float size = 80.0f * sx;
        spywareAnim->DrawCentered(sPos, size, WHITE);
    }
    else if (type == EnemyType::TROJAN && trojanAnim && trojanAnim->loaded) {
        float size = 80.0f * sx;
        trojanAnim->DrawCentered(sPos, size, WHITE);
    }
    // --- 2) Standart Texture Çizimi (Eğer animasyon yoksa) ---
    else {
        Texture2D* tex = nullptr;
        float size = 0.0f;

        switch (type) {
            case EnemyType::WORM:    tex = texWorm;    size = 64.0f; break;
            case EnemyType::SPYWARE: tex = texSpyware; size = 80.0f; break;
            case EnemyType::TROJAN:  tex = texTrojan;  size = 110.0f; break;
            case EnemyType::BOSS:    size = 150.0f; break; // Boss için varsayılan boyut
        }

        if (tex != nullptr && tex->id != 0) {
            float w = size * sx;
            float h = (size * ((float)tex->height / (float)tex->width)) * sy;
            
            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            Rectangle dst = { sPos.x, sPos.y, w, h };
            Vector2 origin = { w / 2.0f, h / 2.0f }; // Merkeze hizala
            DrawTexturePro(*tex, src, dst, origin, 0.0f, WHITE);
        } else {
            // Fallback: Texture yoksa renkli kare çiz
            Color bodyColor = RED;
            if (type == EnemyType::SPYWARE) bodyColor = ORANGE;
            if (type == EnemyType::TROJAN) bodyColor = PURPLE;
            if (type == EnemyType::BOSS) bodyColor = DARKPURPLE;

            DrawRectangle(sPos.x - 16 * sx, sPos.y - 16 * sy, 32 * sx, 32 * sy, bodyColor);
            DrawRectangleLines(sPos.x - 16 * sx, sPos.y - 16 * sy, 32 * sx, 32 * sy, MAROON);
        }
    }

    // --- 3) Can Barı Çizimi ---
    // Can barı boyutunu ve konumunu ölçekle
    float currentSize = (type == EnemyType::BOSS ? 150.0f : (type == EnemyType::TROJAN ? 110.0f : 80.0f));
    float barWidth = (currentSize + 10.0f) * sx;
    float barY = sPos.y - (currentSize * sy / 2.0f + 10.0f * sy);
    
    DrawRectangle(sPos.x - barWidth / 2.0f, barY, barWidth, 4 * sy, DARKGRAY);
    DrawRectangle(sPos.x - barWidth / 2.0f, barY, barWidth * (health / maxHealth), 4 * sy, GREEN);

    // --- 4) Efekt Belirteçleri ---
    float effectRadius = (currentSize * 0.8f) * sx;
    if (stunTimer > 0) {
        DrawCircleLines(sPos.x, sPos.y, effectRadius, GOLD); // Sersemleme halkası
    } else if (slowTimer > 0) {
        DrawRectangleLinesEx({sPos.x - effectRadius, sPos.y - effectRadius, effectRadius * 2, effectRadius * 2}, 2 * sx, SKYBLUE);
    } else if (armorTimer > 0) {
        DrawCircleLines(sPos.x, sPos.y, effectRadius + 4.0f * sx, WHITE); // Zırh kırma
    }
}
    
};

// static değişken tanımları (header-only için)
inline Texture2D* Enemy::texWorm = nullptr;
inline Texture2D* Enemy::texSpyware = nullptr;
inline Texture2D* Enemy::texTrojan = nullptr;

inline AnimatedSprite* Enemy::wormAnim = nullptr;
inline AnimatedSprite* Enemy::spywareAnim = nullptr;
inline AnimatedSprite* Enemy::trojanAnim = nullptr;

#endif