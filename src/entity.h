#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

class Entity {
public:
    Vector2 position;
    bool active;

    Entity(Vector2 pos) : position(pos), active(true) {}
    virtual ~Entity() {}

    // BURAYI GUNCELLE: Fonksiyon isimleri kucuk harf olmali
    virtual void update(float deltaTime) = 0; 
    virtual void draw() = 0;                 
};

#endif