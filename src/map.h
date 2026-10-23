#ifndef MAP_H
#define MAP_H

#include "constants.h"
#include "socket.h"
#include <string>
#include <vector>

struct Map {
    std::vector<Vector2> pathTop;     // Top data trace
    std::vector<Vector2> pathBottom;  // Mid-upper data trace
    std::vector<Vector2> pathBottom2; // Mid-lower data trace
    std::vector<Vector2> pathBottom3; // Bottom data trace
    std::vector<Socket> sockets;

    Texture2D mapTexture = {0};
    bool mapTextureLoaded = false;

    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    Map() {
        // Copper traces aligned to the motherboard PCB layout
        pathTop = {
            {240, 74},  {240, 140}, {620, 140},  {620, 240},  {815, 240},
            {815, 135}, {1045, 135}, {1045, 197}, {1410, 188}, {1410, 303},
        };

        pathBottom = {
            {66, 270},   {260, 270}, {260, 427}, {374, 427}, {374, 244},
            {526, 242},  {526, 395}, {728, 396}, {728, 334}, {936, 334},
            {936, 400},  {1094, 400}, {1104, 338}, {1330, 338}
        };

        pathBottom2 = {
            {70, 530},  {236, 530}, {236, 682}, {462, 682}, {462, 448},
            {794, 448}, {794, 568}, {628, 568}, {628, 652}, {982, 652},
            {982, 472}, {1322, 472}
        };

        pathBottom3 = {
            {212, 834},  {222, 802},  {486, 802},  {486, 752},  {696, 752},
            {696, 802},  {914, 802},  {914, 690},  {1134, 690}, {1134, 768},
            {1294, 768}, {1294, 602}, {1448, 602}, {1448, 536}
        };

        // 18 tower sockets aligned to PCB component pads
        sockets.push_back(Socket({450, 90}));   sockets.push_back(Socket({724, 190}));
        sockets.push_back(Socket({1134, 142})); sockets.push_back(Socket({1362, 240}));
        sockets.push_back(Socket({320, 380}));  sockets.push_back(Socket({450, 290}));
        sockets.push_back(Socket({678, 352}));  sockets.push_back(Socket({1018, 350}));
        sockets.push_back(Socket({1276, 398}));
        sockets.push_back(Socket({350, 638}));  sockets.push_back(Socket({672, 614}));
        sockets.push_back(Socket({1030, 512})); sockets.push_back(Socket({1306, 538}));
        sockets.push_back(Socket({406, 758}));  sockets.push_back(Socket({600, 796}));
        sockets.push_back(Socket({818, 754}));  sockets.push_back(Socket({1026, 728}));
        sockets.push_back(Socket({1216, 710}));
    }

    ~Map() {
        unload();
    }

    void reset() {
        for (auto& s : sockets) {
            s.isOccupied = false;
        }
    }

    void unload() {
        if (mapTextureLoaded) {
            if (mapTexture.id != 0) {
                UnloadTexture(mapTexture);
            }
            mapTexture = {0};
            mapTextureLoaded = false;
        }
    }

    void draw() {
        if (!mapTextureLoaded) {
            std::string path = GetAssetPath("map.jpg");
            if (FileExists(path.c_str())) {
                mapTexture = LoadTexture(path.c_str());
                if (mapTexture.id != 0) {
                    SetTextureFilter(mapTexture, TEXTURE_FILTER_BILINEAR);
                }
            }
            mapTextureLoaded = true;
        }

        float sx = GetScaleX();
        float sy = GetScaleY();

        if (mapTexture.id != 0) {
            Rectangle src = {0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
            Rectangle dst = {0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight()};
            DrawTexturePro(mapTexture, src, dst, {0, 0}, 0.0f, WHITE);
        }

        float thickness = 6.0f * ((sx + sy) * 0.5f);

        auto drawPath = [&](const std::vector<Vector2>& path) {
            if (path.size() < 2) return;
            for (size_t i = 0; i + 1 < path.size(); ++i) {
                Vector2 a = {path[i].x * sx, path[i].y * sy};
                Vector2 b = {path[i + 1].x * sx, path[i + 1].y * sy};
                DrawLineEx(a, b, thickness, CYBER_PATH);
            }
        };

        drawPath(pathTop);
        drawPath(pathBottom);
        drawPath(pathBottom2);
        drawPath(pathBottom3);

        for (auto& s : sockets) {
            s.draw();
        }
    }
};

#endif
