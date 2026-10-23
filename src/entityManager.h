#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "enemy.h"
#include "tower.h"
#include "projectile.h"
#include <vector>

class EntityManager {
public:
    std::vector<Enemy*> enemies;
    std::vector<Tower*> towers;
    std::vector<Projectile*> projectiles;
    std::vector<DamageZone> damageZones;

    EntityManager() = default;
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    ~EntityManager() {
        clear();
    }

    void clear() {
        for (auto e : enemies) delete e;
        for (auto t : towers) delete t;
        for (auto p : projectiles) delete p;

        enemies.clear();
        towers.clear();
        projectiles.clear();
        damageZones.clear();
    }

private:
    void notifyEnemyRemoved(const Enemy* removed) {
        for (auto p : projectiles) {
            if (p) {
                p->onEnemyRemoved(removed);
            }
        }
    }

public:
    float updateAll(float dt, int& bitcoinReward) {
        float totalCPU_Damage = 0.0f;
        bitcoinReward = 0;

        for (auto t : towers) {
            t->update(dt, enemies, projectiles, damageZones);
        }

        for (int i = (int)damageZones.size() - 1; i >= 0; i--) {
            damageZones[i].duration -= dt;
            for (auto e : enemies) {
                if (e && e->active &&
                    CheckCollisionPointCircle(e->position, damageZones[i].position, damageZones[i].radius)) {
                    e->health -= damageZones[i].dps * dt;
                }
            }
            if (damageZones[i].duration <= 0) {
                damageZones.erase(damageZones.begin() + i);
            }
        }

        for (int i = (int)enemies.size() - 1; i >= 0; i--) {
            enemies[i]->update(dt);
            if (enemies[i]->reachedEnd) {
                totalCPU_Damage += enemies[i]->damage;
                notifyEnemyRemoved(enemies[i]);
                delete enemies[i];
                enemies.erase(enemies.begin() + i);
            } else if (enemies[i]->health <= 0) {
                bitcoinReward += enemies[i]->reward;
                notifyEnemyRemoved(enemies[i]);
                delete enemies[i];
                enemies.erase(enemies.begin() + i);
            }
        }

        for (int i = (int)projectiles.size() - 1; i >= 0; i--) {
            projectiles[i]->update(dt, enemies);
            if (!projectiles[i]->active) {
                delete projectiles[i];
                projectiles.erase(projectiles.begin() + i);
            }
        }
        return totalCPU_Damage;
    }

    void drawAll() {
        float sx = GetScaleX();
        for (auto& zone : damageZones) {
            DrawCircleGradient(
                (int)(zone.position.x * sx),
                (int)(zone.position.y * sx),
                zone.radius * sx,
                Fade(ORANGE, 0.4f),
                Fade(RED, 0.0f)
            );
        }
        for (auto t : towers) t->draw();
        for (auto e : enemies) e->draw();
        for (auto p : projectiles) p->draw();
    }
};

#endif
