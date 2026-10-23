#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include "constants.h"
#include "enemy.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Projectile {
public:
    Vector2 pos;
    Vector2 direction;
    Vector2 lastTargetPos;
    Enemy* target;
    float speed, damage, width;
    bool active = true;
    int ricochetCount = 0;
    bool isSlowing = false;
    float stunDuration = 0.0f;
    float slowDuration = 1.5f;

    // Data Spike: piercing + multi-target continuous collision
    bool piercing = false;
    int remainingHits = 0;

    bool isArmorPierce = false;
    std::vector<Enemy*> hitEnemies;

    Projectile(
        Vector2 startPos,
        Enemy* targetEnemy,
        float dmg,
        int ricochet = 0,
        bool slowing = false,
        float stun = 0.0f,
        float slowDur = 1.5f,
        int pierceExtra = 0,
        float w = 6.0f,
        bool armor = false
    )
        : pos(startPos),
          lastTargetPos(startPos),
          target(targetEnemy),
          speed(500.0f),
          damage(dmg),
          width(w),
          ricochetCount(ricochet),
          isSlowing(slowing),
          stunDuration(stun),
          slowDuration(slowDur),
          isArmorPierce(armor)
    {
        if (targetEnemy) {
            lastTargetPos = targetEnemy->position;
            Vector2 diff = Vector2Subtract(targetEnemy->position, pos);
            direction = (Vector2Length(diff) > 0.001f) ? Vector2Normalize(diff) : Vector2{1.0f, 0.0f};
        } else {
            direction = {1.0f, 0.0f};
        }

        piercing = (pierceExtra > 0);
        if (piercing) {
            remainingHits = pierceExtra + 1;
            speed = 800.0f;
        }
    }

    // Clears dangling pointers before an Enemy is freed by EntityManager
    void onEnemyRemoved(const Enemy* removed) {
        if (!removed) return;
        if (target == removed) {
            lastTargetPos = removed->position;
            target = nullptr;
        }
        hitEnemies.erase(
            std::remove(hitEnemies.begin(), hitEnemies.end(), removed),
            hitEnemies.end()
        );
    }

private:
    // Swept segment (a -> b) vs circle (center, r) continuous collision check
    static bool SegmentIntersectsCircle(Vector2 a, Vector2 b, Vector2 center, float r) {
        Vector2 d = Vector2Subtract(b, a);
        float len2 = d.x * d.x + d.y * d.y;

        if (len2 < 1e-6f) {
            return CheckCollisionPointCircle(a, center, r);
        }

        Vector2 f = Vector2Subtract(a, center);
        float t = -(f.x * d.x + f.y * d.y) / len2;

        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;

        Vector2 closest = { a.x + d.x * t, a.y + d.y * t };
        return CheckCollisionPointCircle(closest, center, r);
    }

public:
    void update(float deltaTime, std::vector<Enemy*>& enemies) {
        Vector2 prevPos = pos;

        // --- 1) MOVEMENT ---
        if (piercing) {
            if (hitEnemies.empty() && target && target->active && target->health > 0 && !target->reachedEnd) {
                lastTargetPos = target->position;
                Vector2 toT = Vector2Subtract(target->position, pos);
                if (Vector2Length(toT) > 0.001f) direction = Vector2Normalize(toT);
            }

            pos = Vector2Add(pos, Vector2Scale(direction, speed * deltaTime));

            if (pos.x < -100 || pos.x > VIRTUAL_WIDTH + 100 || pos.y < -100 || pos.y > VIRTUAL_HEIGHT + 100) {
                active = false;
                return;
            }
        } else {
            if (!target || !target->active || target->health <= 0 || target->reachedEnd) {
                target = nullptr;
                active = false;
                return;
            }

            lastTargetPos = target->position;
            Vector2 toTarget = Vector2Subtract(target->position, pos);
            Vector2 dir = (Vector2Length(toTarget) > 0.001f) ? Vector2Normalize(toTarget) : direction;
            direction = dir;
            pos = Vector2Add(pos, Vector2Scale(dir, speed * deltaTime));

            if (Vector2Distance(pos, target->position) < 8.0f) {
                target->health -= damage;
                if (isSlowing) target->slowTimer = slowDuration;
                if (stunDuration > 0) target->stunTimer = stunDuration;

                if (ricochetCount > 0) {
                    ricochetCount--;
                    Enemy* newTarget = nullptr;
                    float minNextDist = 200.0f;

                    for (auto e : enemies) {
                        if (e && e != target && e->active && e->health > 0 && !e->reachedEnd) {
                            float d2 = Vector2Distance(pos, e->position);
                            if (d2 < minNextDist) {
                                minNextDist = d2;
                                newTarget = e;
                            }
                        }
                    }

                    if (newTarget) {
                        target = newTarget;
                        lastTargetPos = newTarget->position;
                        return;
                    }
                }

                active = false;
                return;
            }
        }

        // --- 2) SWEPT COLLISION (PIERCING PROJECTILES) ---
        if (piercing) {
            float hitRadius = (width / 2.0f) + 14.0f;

            for (auto e : enemies) {
                if (!e || !e->active || e->health <= 0 || e->reachedEnd) continue;
                if (std::find(hitEnemies.begin(), hitEnemies.end(), e) != hitEnemies.end()) continue;

                if (SegmentIntersectsCircle(prevPos, pos, e->position, hitRadius)) {
                    float finalDamage = (e->armorTimer > 0) ? damage * 1.3f : damage;
                    e->health -= finalDamage;

                    if (isArmorPierce) e->armorTimer = 3.0f;
                    if (isSlowing) e->slowTimer = slowDuration;
                    if (stunDuration > 0) e->stunTimer = stunDuration;

                    hitEnemies.push_back(e);

                    remainingHits--;
                    if (remainingHits <= 0) {
                        active = false;
                        return;
                    }
                }
            }
        }
    }

    void draw() {
        float sx = GetScaleX();
        float sy = GetScaleY();
        Color pColor = isArmorPierce ? GOLD : (isSlowing ? SKYBLUE : CYBER_ACCENT);

        if (piercing) {
            DrawLineEx(
                {pos.x * sx, pos.y * sy},
                {(pos.x - direction.x * 25.0f) * sx, (pos.y - direction.y * 25.0f) * sy},
                width * sx,
                pColor
            );
        } else {
            DrawRectangleV({pos.x * sx - 3*sx, pos.y * sy - 3*sy}, {6*sx, 6*sy}, pColor);
        }
    }
};

#endif
