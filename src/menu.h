#ifndef MENU_H
#define MENU_H

#include "constants.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>

// --- OYUN DURUMLARI ---
enum class GameState { MENU, SETTINGS, PLAYING, PAUSED, GAME_OVER };

// --- MENU BUTON YAPISI ---
struct MenuItem {
  Rectangle bounds;
  const char *text;
  bool isHovered;
  float hoverScale;
  Color baseColor;
  Color hoverColor;

  MenuItem(float x, float y, float w, float h, const char *txt) {
    bounds = {x, y, w, h};
    text = txt;
    isHovered = false;
    hoverScale = 1.0f;
    baseColor = {0, 100, 150, 200};
    hoverColor = {0, 200, 255, 255};
  }

  void update() {
    float sx = GetScaleX();
    float sy = GetScaleY();
    Rectangle scaledBounds = {bounds.x * sx, bounds.y * sy, bounds.width * sx,
                              bounds.height * sy};

    isHovered = CheckCollisionPointRec(GetMousePosition(), scaledBounds);

    // Smooth hover animation
    float targetScale = isHovered ? 1.1f : 1.0f;
    hoverScale += (targetScale - hoverScale) * 0.15f;
  }

  void draw() {
    float sx = GetScaleX();
    float sy = GetScaleY();

    float scaledW = bounds.width * sx * hoverScale;
    float scaledH = bounds.height * sy * hoverScale;
    float scaledX = bounds.x * sx - (scaledW - bounds.width * sx) / 2;
    float scaledY = bounds.y * sy - (scaledH - bounds.height * sy) / 2;

    Rectangle drawBounds = {scaledX, scaledY, scaledW, scaledH};

    // Background with gradient effect
    Color bgColor = isHovered ? hoverColor : baseColor;
    DrawRectangleRounded(drawBounds, 0.3f, 8, Fade(bgColor, 0.8f));

    // Border glow effect
    Color borderColor = isHovered ? CYBER_ACCENT : Fade(CYBER_ACCENT, 0.5f);
    DrawRectangleRoundedLinesEx(drawBounds, 0.3f, 8, 3, borderColor);

    // Text
    int fontSize = (int)(30 * sx * hoverScale);
    int textWidth = MeasureText(text, fontSize);
    float textX = scaledX + (scaledW - textWidth) / 2;
    float textY = scaledY + (scaledH - fontSize) / 2;

    Color textColor = isHovered ? WHITE : Fade(WHITE, 0.9f);
    DrawText(text, (int)textX, (int)textY, fontSize, textColor);
  }

  bool isClicked() {
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  }
};

// --- ANA MENUÜ SINIFI ---
class Menu {
public:
  Texture2D background = {0};
  std::vector<MenuItem> items;
  float titleGlow;
  float particleTime;
  bool backgroundLoaded;

  Menu(const Menu &) = delete;
  Menu &operator=(const Menu &) = delete;

  Menu() {
    titleGlow = 0.0f;
    particleTime = 0.0f;
    backgroundLoaded = false;

    // Butonları oluştur (sanal çözünürlük koordinatları)
    float centerX = VIRTUAL_WIDTH / 2 - 150;
    float startY = 400;
    float spacing = 90;

    items.push_back(MenuItem(centerX, startY, 300, 70, "NEW GAME"));
    items.push_back(MenuItem(centerX, startY + spacing, 300, 70, "SETTINGS"));
    items.push_back(MenuItem(centerX, startY + spacing * 2, 300, 70, "EXIT"));
  }

  void loadBackground() {
    if (!backgroundLoaded) {
      std::string path = GetAssetPath("menu/menu_background.png");
      if (FileExists(path.c_str())) {
        background = LoadTexture(path.c_str());
      }
      backgroundLoaded = true;
    }
  }

  void unloadBackground() {
    if (backgroundLoaded) {
      if (background.id != 0) {
        UnloadTexture(background);
      }
      background = {0};
      backgroundLoaded = false;
    }
  }

  void update() {
    float dt = GetFrameTime();
    titleGlow += dt * 3.0f;
    particleTime += dt;

    for (auto &item : items) {
      item.update();
    }
  }

  void draw() {
    float sx = GetScaleX();
    float sy = GetScaleY();

    // Arka plan
    if (backgroundLoaded && background.id != 0) {
      Rectangle source = {0, 0, (float)background.width,
                          (float)background.height};
      Rectangle dest = {0, 0, (float)GetScreenWidth(),
                        (float)GetScreenHeight()};
      DrawTexturePro(background, source, dest, {0, 0}, 0.0f, WHITE);
    } else {
      ClearBackground(CYBER_BG);
    }

    // Overlay for better text visibility
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.3f));

    // Title with glow effect
    const char *title = "CYBER DEFENSE";
    int titleSize = (int)(70 * sx);
    int titleWidth = MeasureText(title, titleSize);
    float titleX = (GetScreenWidth() - titleWidth) / 2;
    float titleY = 120 * sy;

    // Glow layers
    float glowIntensity = (sinf(titleGlow) + 1.0f) / 2.0f * 0.5f + 0.5f;
    DrawText(title, (int)titleX + 3, (int)titleY + 3, titleSize,
             Fade(BLACK, 0.5f));
    DrawText(title, (int)titleX, (int)titleY, titleSize,
             Fade(CYBER_ACCENT, glowIntensity));

    // Subtitle
    const char *subtitle = "TOWER DEFENSE";
    int subSize = (int)(35 * sx);
    int subWidth = MeasureText(subtitle, subSize);
    float subX = (GetScreenWidth() - subWidth) / 2;
    float subY = titleY + titleSize + 10 * sy;
    DrawText(subtitle, (int)subX, (int)subY, subSize, Fade(WHITE, 0.7f));

    // Draw circuit decoration lines
    drawCircuitLines();

    // Draw menu items
    for (auto &item : items) {
      item.draw();
    }

    // Footer
    const char *footer = "Press ESC to exit";
    int footerSize = (int)(18 * sx);
    int footerWidth = MeasureText(footer, footerSize);
    DrawText(footer, (GetScreenWidth() - footerWidth) / 2,
             GetScreenHeight() - 40 * sy, footerSize, Fade(WHITE, 0.4f));
  }

  void drawCircuitLines() {
    float sx = GetScaleX();
    float sy = GetScaleY();
    float time = GetTime();

    // Animated circuit lines on sides
    for (int i = 0; i < 5; i++) {
      float offset = sinf(time * 2.0f + i * 0.5f) * 10.0f;

      // Left side
      DrawLineEx({50 * sx, (200 + i * 80 + offset) * sy},
                 {150 * sx, (250 + i * 80 + offset) * sy}, 2,
                 Fade(CYBER_ACCENT, 0.3f));

      // Right side
      DrawLineEx({(VIRTUAL_WIDTH - 50) * sx, (200 + i * 80 - offset) * sy},
                 {(VIRTUAL_WIDTH - 150) * sx, (250 + i * 80 - offset) * sy}, 2,
                 Fade(CYBER_ACCENT, 0.3f));
    }
  }

  // Returns: 0 = nothing, 1 = new game, 2 = settings, 3 = exit
  int handleInput() {
    if (items[0].isClicked())
      return 1; // New Game
    if (items[1].isClicked())
      return 2; // Settings
    if (items[2].isClicked())
      return 3; // Exit
    return 0;
  }
};

// --- AYARLAR MENÜSÜ SINIFI ---
class SettingsMenu {
public:
  std::vector<MenuItem> items;
  int selectedResolution;
  bool fullscreenToggle;

  SettingsMenu() {
    selectedResolution = 1; // Default: 900p
    fullscreenToggle = false;

    float centerX = VIRTUAL_WIDTH / 2 - 175;
    float startY = 250;
    float spacing = 80;

    items.push_back(MenuItem(centerX, startY, 350, 60, "1280 x 720"));
    items.push_back(MenuItem(centerX, startY + spacing, 350, 60, "1600 x 900"));
    items.push_back(
        MenuItem(centerX, startY + spacing * 2, 350, 60, "1920 x 1080"));
    items.push_back(
        MenuItem(centerX, startY + spacing * 3, 350, 60, "FULLSCREEN"));
    items.push_back(
        MenuItem(centerX, startY + spacing * 4 + 30, 350, 60, "BACK"));

    updateButtonColors();
  }

  void updateButtonColors() {
    // Resolution buttons
    for (int i = 0; i < 3; i++) {
      if (i == selectedResolution) {
        items[i].baseColor = {0, 180, 100, 200}; // Green for selected
      } else {
        items[i].baseColor = {0, 100, 150, 200}; // Default blue
      }
    }

    // Fullscreen toggle
    if (fullscreenToggle) {
      items[3].baseColor = {0, 180, 100, 200};
    } else {
      items[3].baseColor = {0, 100, 150, 200};
    }
  }

  void update() {
    for (auto &item : items) {
      item.update();
    }
  }

  void draw() {
    float sx = GetScaleX();
    float sy = GetScaleY();

    // Dark background
    ClearBackground(CYBER_BG);

    // Title
    const char *title = "SETTINGS";
    int titleSize = (int)(60 * sx);
    int titleWidth = MeasureText(title, titleSize);
    float titleX = (GetScreenWidth() - titleWidth) / 2;
    DrawText(title, (int)titleX, (int)(100 * sy), titleSize, CYBER_ACCENT);

    // Section label
    const char *resLabel = "RESOLUTION";
    int labelSize = (int)(25 * sx);
    int labelWidth = MeasureText(resLabel, labelSize);
    DrawText(resLabel, (GetScreenWidth() - labelWidth) / 2, (int)(200 * sy),
             labelSize, Fade(WHITE, 0.7f));

    // Draw items
    for (auto &item : items) {
      item.draw();
    }

    // Info text
    const char *info = "Changes apply immediately";
    int infoSize = (int)(16 * sx);
    int infoWidth = MeasureText(info, infoSize);
    DrawText(info, (GetScreenWidth() - infoWidth) / 2,
             GetScreenHeight() - 60 * sy, infoSize, Fade(WHITE, 0.4f));
  }

  // Returns: 0 = nothing, -1 = back, 1-3 = resolution changed, 4 = fullscreen
  // toggled
  int handleInput() {
    // Resolution buttons
    for (int i = 0; i < 3; i++) {
      if (items[i].isClicked()) {
        selectedResolution = i;
        applyResolution(i);
        updateButtonColors();
        return i + 1;
      }
    }

    // Fullscreen toggle
    if (items[3].isClicked()) {
      fullscreenToggle = !fullscreenToggle;
      ToggleFullscreen();
      updateButtonColors();
      return 4;
    }

    // Back button
    if (items[4].isClicked()) {
      return -1;
    }

    return 0;
  }

  void applyResolution(int index) {
    switch (index) {
    case 0:
      SetWindowSize(1280, 720);
      currentRes = RES_720P;
      break;
    case 1:
      SetWindowSize(1600, 900);
      currentRes = RES_900P;
      break;
    case 2:
      SetWindowSize(1920, 1080);
      currentRes = RES_1080P;
      break;
    }
  }
};

#endif
