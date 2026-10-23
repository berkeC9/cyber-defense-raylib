#ifndef WAVEMANAGER_H
#define WAVEMANAGER_H

#include "raylib.h"
#include "entityManager.h"
#include "map.h"

class WaveManager {
public:
    int currentWave = 0; //
    int enemiesLeftInWave = 0; //
    float spawnTimer = 0; //
    float spawnInterval = 1.0f; //
    bool waveActive = false; //

    void startNextWave() {
        currentWave++; //
        
        // --- BOSS DALGASI KONTROLÜ (Her 10 Dalga) ---
        if (currentWave % 10 == 0) {
            enemiesLeftInWave = 1; // Sadece 1 Boss
            spawnInterval = 0.5f; 
        } else {
            // Normal dalga hesaplaması
            enemiesLeftInWave = 5 + (currentWave * 2); 
            spawnInterval = fmax(0.3f, 1.2f - (currentWave * 0.1f)); 
        }
        
        waveActive = true; //
    }

    void update(float dt, EntityManager& manager, const Map& gameMap) {
        if (!waveActive) return; //

        spawnTimer += dt; //
        if (spawnTimer >= spawnInterval && enemiesLeftInWave > 0) {
            int pathIndex = GetRandomValue(0, 3);
            const std::vector<Vector2>* path = &gameMap.pathTop;
            if (pathIndex == 1) path = &gameMap.pathBottom;
            else if (pathIndex == 2) path = &gameMap.pathBottom2;
            else if (pathIndex == 3) path = &gameMap.pathBottom3;
            
            EnemyType typeToSpawn = EnemyType::WORM; // Varsayılan tip
            
            // --- DÜŞMAN TİPİ BELİRLEME ---
            if (currentWave % 10 == 0) {
                // Boss dalgasında sadece Boss gönder
                typeToSpawn = EnemyType::BOSS; 
            } else {
                // Normal Dalga Mantığı
                if (currentWave >= 5) { 
                    int rand = GetRandomValue(0, 10);
                    if (rand > 8) typeToSpawn = EnemyType::TROJAN;
                    else if (rand > 5) typeToSpawn = EnemyType::SPYWARE;
                } 
                else if (currentWave >= 3) { 
                    if (GetRandomValue(0, 10) > 6) typeToSpawn = EnemyType::SPYWARE;
                }
            }

            manager.enemies.push_back(new Enemy(*path, typeToSpawn, currentWave));
            
            enemiesLeftInWave--; //
            spawnTimer = 0; //
        }

        if (enemiesLeftInWave <= 0 && manager.enemies.empty()) {
            waveActive = false; //
        }
    }
};

#endif
