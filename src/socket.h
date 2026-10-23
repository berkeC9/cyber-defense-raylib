#ifndef SOCKET_H
#define SOCKET_H

#include "raylib.h"
#include "constants.h"

class Socket {
public:
    Vector2 position;
    bool isOccupied;      // Üzerinde kule var mı?
    float radius;         // Tıklama alanı büyüklüğü

    Socket(Vector2 pos) : position(pos), isOccupied(false), radius(25.0f) {}

    // Farenin soket üzerinde olup olmadığını kontrol eder
    bool isMouseOver() {
    float sx = GetScaleX();
    float sy = GetScaleY();
    
    // Virtual (1600x900) koordinatları gerçek ekran koordinatlarına çevir
    Vector2 screenPos = { position.x * sx, position.y * sy };
    
    // Tıklama alanını (hitbox) ölçeklendir. 
    // Kare objeler için 25-30 arası bir yarıçap idealdir.
    float scaledRadius = 25.0f * sx; 

    return CheckCollisionPointCircle(GetMousePosition(), screenPos, scaledRadius);
}
    

    void draw() {
        float scaleX = GetScaleX();
        float scaleY = GetScaleY();
        Vector2 screenPos = { position.x * scaleX, position.y * scaleY };

        if (isOccupied) {
            // Kule varsa soketi farklı çiz veya hiç çizme
            DrawCircleLinesV(screenPos, radius * scaleX, Fade(CYBER_ACCENT, 0.3f));
        } else {
            // Boş soket: Mouse üzerindeyse parla
            Color color = isMouseOver() ? CYBER_ACCENT : Fade(CYBER_ACCENT, 0.5f);
            DrawCircleLinesV(screenPos, radius * scaleX, color);
            DrawCircleV(screenPos, 4.0f * scaleX, color); // Merkez noktası
        }
    }
};

#endif