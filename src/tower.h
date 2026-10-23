#ifndef TOWER_H
#define TOWER_H

#include "raylib.h"
#include "constants.h"
#include "enemy.h"
#include "projectile.h"
#include "animated_sprite.h"
#include <vector>

enum class TowerType { FIREWALL, LOGIC_BOMB, DEFRAGMENTER, DATA_SPIKE }; // Yeni tip eklendi

struct DamageZone {
    Vector2 position; float radius; float duration; float dps;
};

class Tower {
public:
    Vector2 pos;
    TowerType type;
    float range, fireRate, timer, damage;
    int cost, level = 1;
    float burstTimer = 0; // Seviye 6 Firewall için

    // ---------------------------
    // ASSET REFERANSLARI (main.cpp set eder)
    // ---------------------------
    static Texture2D* firewallTex[6];
    static Texture2D* logicBombTex[6];
    static Texture2D* defragTex[6];
    static Texture2D* dataSpikeTex[6];

    static AnimatedSprite* firewallAnim[6];
    static AnimatedSprite* logicBombAnim[6];
    static AnimatedSprite* defragAnim[6];
    static AnimatedSprite* dataSpikeAnim[6];

    static void SetFirewallTexture(int idx, Texture2D* t)     { if (idx >= 0 && idx < 6) firewallTex[idx] = t; }
    static void SetLogicBombTexture(int idx, Texture2D* t)    { if (idx >= 0 && idx < 6) logicBombTex[idx] = t; }
    static void SetDefragTexture(int idx, Texture2D* t)       { if (idx >= 0 && idx < 6) defragTex[idx] = t; }
    static void SetDataSpikeTexture(int idx, Texture2D* t)     { if (idx >= 0 && idx < 6) dataSpikeTex[idx] = t; }

    static void SetFirewallAnim(int idx, AnimatedSprite* a)   { if (idx >= 0 && idx < 6) firewallAnim[idx] = a; }
    static void SetLogicBombAnim(int idx, AnimatedSprite* a)  { if (idx >= 0 && idx < 6) logicBombAnim[idx] = a; }
    static void SetDefragAnim(int idx, AnimatedSprite* a)     { if (idx >= 0 && idx < 6) defragAnim[idx] = a; }
    static void SetDataSpikeAnim(int idx, AnimatedSprite* a)   { if (idx >= 0 && idx < 6) dataSpikeAnim[idx] = a; }

    Tower(Vector2 position, TowerType t) : pos(position), type(t), timer(0) {
        setupStats();
    }

    void setupStats() {
        if (type == TowerType::FIREWALL) {
            cost = 100; damage = 10.0f + (level >= 3 ? 10.0f : 0.0f);
            range = 200.0f + (level >= 3 ? 50.0f : 0.0f); fireRate = 1.0f + (level >= 5 ? 0.8f : 0.0f);
        } else if (type == TowerType::LOGIC_BOMB) {
            cost = 250; damage = 40.0f + (level >= 3 ? 30.0f : 0.0f);
            range = 180.0f + (level >= 3 ? 60.0f : 0.0f); fireRate = 0.4f + (level >= 5 ? 0.3f : 0.0f);
        } else if (type == TowerType::DEFRAGMENTER) {
            cost = 150; damage = 2.0f + (level * 1.5f); // DoT hasarı
            // Lvl 2: Menzil ve FireRate artışı
            range = 140.0f + (level >= 2 ? 60.0f : 0.0f);
            fireRate = 2.0f + (level >= 2 ? 0.8f : 0.0f);
        }
            else if (type == TowerType::DATA_SPIKE) {
            cost = 300;
            damage = 40.0f + (level * 15.0f); 
            range = 300.0f + (level >= 3 ? 150.0f : 0.0f); // Lvl 3: Menzil
            fireRate = 0.3f + (level >= 5 ? 0.2f : 0.0f);  // Lvl 5: Fire rate
        }
    }

    void upgrade() { if (level < 6) { level++; setupStats(); } }
    int getUpgradeCost() { return cost * level; }

    void update(float dt, std::vector<Enemy*>& enemies, std::vector<Projectile*>& projectiles, std::vector<DamageZone>& zones) {
        timer += dt;
        if (timer >= 1.0f / fireRate) {
            if (type == TowerType::FIREWALL) updateFirewall(enemies, projectiles);
            else if (type == TowerType::LOGIC_BOMB) updateLogicBomb(enemies, zones, projectiles);
            else if (type == TowerType::DEFRAGMENTER) updateDefragmenter(enemies, projectiles);
            else if (type == TowerType::DATA_SPIKE) updateDataSpike(enemies, projectiles);
            timer = 0;
        }
    }

    void updateFirewall(std::vector<Enemy*>& enemies, std::vector<Projectile*>& projectiles) {
        Enemy* target = nullptr;
        for (auto e : enemies) {
            if (Vector2Distance(pos, e->position) <= range) { target = e; break; }
        }

        if (target) {
            int ricochet = (level >= 4) ? 1 : 0;

            // Seviye 2: Yan yana 2 mermi
            if (level >= 2 && level < 6) {
                projectiles.push_back(new Projectile({pos.x - 5, pos.y}, target, damage, ricochet));
                projectiles.push_back(new Projectile({pos.x + 5, pos.y}, target, damage, ricochet));
            }
            // Seviye 6: Arka arkaya 2 atış (simüle edilmiş burst)
            else if (level == 6) {
                projectiles.push_back(new Projectile(pos, target, damage, ricochet, false, 0.0f, 1.5f));
                projectiles.push_back(new Projectile({pos.x, pos.y - 12.0f}, target, damage, ricochet, false, 0.0f, 1.5f));
            }
            else {
                projectiles.push_back(new Projectile(pos, target, damage, ricochet));
            }
        }
    }

    void updateLogicBomb(std::vector<Enemy*>& enemies, std::vector<DamageZone>& zones, std::vector<Projectile*>& projectiles) {
        std::vector<Enemy*> targets;
        for (auto e : enemies) {
            if (Vector2Distance(pos, e->position) <= range) targets.push_back(e);
        }

        if (!targets.empty()) {
            // Seviye 2: En yakın ve en uzak
            if (level >= 2) {
                Enemy* closest = targets[0]; Enemy* furthest = targets[0];
                for (auto e : targets) {
                    if (Vector2Distance(pos, e->position) < Vector2Distance(pos, closest->position)) closest = e;
                    if (Vector2Distance(pos, e->position) > Vector2Distance(pos, furthest->position)) furthest = e;
                }
                fireBomb(closest, zones, projectiles);
                if (closest != furthest) fireBomb(furthest, zones, projectiles);
            } else {
                fireBomb(targets[0], zones, projectiles);
            }
        }
    }

    void updateDefragmenter(std::vector<Enemy*>& enemies, std::vector<Projectile*>& projectiles) {
        for (auto e : enemies) {
            if (Vector2Distance(pos, e->position) <= range) {
                int ricochet = (level >= 3) ? 1 : 0;          // Lvl 3: Sekme
                float stun = (level >= 4) ? 0.2f : 0.0f;      // Lvl 4: 0.2s Stun
                float slowDur = (level >= 5) ? 3.5f : 1.5f;   // Lvl 5: Etki süresi

                projectiles.push_back(new Projectile(pos, e, damage, ricochet, true, stun, slowDur));
                if (level == 6) {
                    projectiles.push_back(new Projectile({pos.x - 5, pos.y}, e, damage, ricochet, true, stun, slowDur));
                    projectiles.push_back(new Projectile({pos.x + 5, pos.y}, e, damage, ricochet, true, stun, slowDur));
                }
                break;
            }
        }
    }

    void updateDataSpike(std::vector<Enemy*>& enemies, std::vector<Projectile*>& projectiles) {
    for (auto e : enemies) {
        if (Vector2Distance(pos, e->position) <= range) {
            // Lvl 1: 1 pierce (2 hedef), Lvl 2: 2 pierce (3 hedef)
            int pierce = (level >= 2) ? 2 : 1; 
            
            // Lvl 2: Genişleme (6px -> 15px)
            float w = (level >= 2) ? 15.0f : 6.0f; 
            
            // Lvl 4: Armor Pierce (Zırh Kırma) aktif mi?
            bool armor = (level >= 4); 

            // Mermi oluşturma: (pos, hedef, hasar, ricochet=0, pierce, genişlik, armor)
            projectiles.push_back(new Projectile(pos, e, damage, 0, false, 0.0f, 1.5f, pierce, w, armor));

            // Lvl 6: Overpenetration Beam (Arka arkaya 2 delici mermi)
            if (level == 6) {
                projectiles.push_back(new Projectile({pos.x, pos.y - 10.0f}, e, damage, 0, false, 0.0f, 1.5f, pierce, w, armor));
            }
            break; 
        }
    }
}

    void fireBomb(Enemy* target, std::vector<DamageZone>& zones, std::vector<Projectile*>& projectiles) {
        projectiles.push_back(new Projectile(pos, target, damage));
        if (level >= 4) zones.push_back({target->position, 90.0f, 3.5f, damage * 0.3f});
    }

    bool isMouseOver() {
    float sx = GetScaleX();
    float sy = GetScaleY();
    
    // Kule pozisyonunu ölçekle
    Vector2 screenPos = { pos.x * sx, pos.y * sy };
    
    // Kuleler biraz daha büyük olduğu için yarıçapı 30 yapabilirsin
    float scaledRadius = 30.0f * sx; 

    return CheckCollisionPointCircle(GetMousePosition(), screenPos, scaledRadius);
}

    void draw() {
        float sx = GetScaleX();
        float sy = GetScaleY();
        Vector2 sPos = {pos.x * sx, pos.y * sy};

        int idx = level - 1;
        if (idx < 0) idx = 0;
        if (idx > 5) idx = 5;

        AnimatedSprite* anim = nullptr;
        Texture2D* tex = nullptr;

        if (type == TowerType::FIREWALL) {
            anim = firewallAnim[idx];
            tex  = firewallTex[idx];
        } else if (type == TowerType::LOGIC_BOMB) {
            anim = logicBombAnim[idx];
            tex  = logicBombTex[idx];
        } else if (type == TowerType::DEFRAGMENTER) {
            anim = defragAnim[idx];
            tex  = defragTex[idx];
        }else if (type == TowerType::DATA_SPIKE) {
            anim = dataSpikeAnim[idx];
            tex  = dataSpikeTex[idx];
        }

        // --- 1) Animasyon varsa onu çiz ---
        if (anim && anim->loaded) {
            float baseW = 56.0f;
            anim->DrawCentered(sPos, baseW * sx, WHITE);
            return;
        }

        // --- 2) Texture varsa onu çiz ---
        if (tex && tex->id != 0) {
            float baseW = 56.0f;
            float w = baseW * sx;
            float aspect = (float)tex->height / (float)tex->width;
            float h = w * aspect;

            Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
            Rectangle dst = {sPos.x, sPos.y, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};

            DrawTexturePro(*tex, src, dst, origin, 0.0f, WHITE);
            return;
        }

        // --- 3) Fallback: eski çizim ---
        Color c = (type == TowerType::FIREWALL)
                      ? CYBER_ACCENT
                      : (type == TowerType::LOGIC_BOMB
                             ? PURPLE
                             : (type == TowerType::DATA_SPIKE ? GOLD : SKYBLUE));
        DrawRectangleV({sPos.x - 22*sx, sPos.y - 22*sy}, {44*sx, 44*sy}, DARKGRAY);
        DrawCircleV(sPos, 12*sx, c);
        for (int i = 0; i < level; i++)
            DrawCircle(sPos.x - 15*sx + (i*6*sx), sPos.y + 28*sy, 2.5f*sx, (level == 6 ? GOLD : GREEN));
    }
};

// header-only static tanımları
inline Texture2D* Tower::firewallTex[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline Texture2D* Tower::logicBombTex[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline Texture2D* Tower::defragTex[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline Texture2D* Tower::dataSpikeTex[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

inline AnimatedSprite* Tower::firewallAnim[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline AnimatedSprite* Tower::logicBombAnim[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline AnimatedSprite* Tower::defragAnim[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
inline AnimatedSprite* Tower::dataSpikeAnim[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

#endif
