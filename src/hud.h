#ifndef HUD_H
#define HUD_H

#include "constants.h"
#include "raylib.h"

// ==================== PROFESSIONAL HUD SYSTEM ====================

// UI Textures (loaded in main)
struct HUDAssets {
  Texture2D bitcoinIcon = {0};
  Texture2D shieldIcon = {0};
  Texture2D firewallIcon = {0};
  Texture2D logicBombIcon = {0};
  Texture2D defragmenterIcon = {0};
  Texture2D dataSpikeIcon = {0};

  bool loaded = false;

  HUDAssets() = default;
  HUDAssets(const HUDAssets &) = delete;
  HUDAssets &operator=(const HUDAssets &) = delete;

  void load() {
    if (!loaded) {
      // Drawn vector fallback icons used in cards for clean scaling
      bitcoinIcon = {0};
      shieldIcon = {0};
      firewallIcon = {0};
      logicBombIcon = {0};
      defragmenterIcon = {0};
      dataSpikeIcon = {0};
      loaded = true;
    }
  }

  void unload() {
    if (loaded) {
      if (bitcoinIcon.id != 0) UnloadTexture(bitcoinIcon);
      if (shieldIcon.id != 0) UnloadTexture(shieldIcon);
      if (firewallIcon.id != 0) UnloadTexture(firewallIcon);
      if (logicBombIcon.id != 0) UnloadTexture(logicBombIcon);
      if (defragmenterIcon.id != 0) UnloadTexture(defragmenterIcon);
      if (dataSpikeIcon.id != 0) UnloadTexture(dataSpikeIcon);
      bitcoinIcon = {0};
      shieldIcon = {0};
      firewallIcon = {0};
      logicBombIcon = {0};
      defragmenterIcon = {0};
      dataSpikeIcon = {0};
      loaded = false;
    }
  }
};

// ==================== DRAWING FUNCTIONS ====================

// Draw a panel with cyber styling
inline void DrawCyberPanel(float x, float y, float width, float height,
                           float alpha = 0.85f) {
  float sx = GetScaleX();
  float sy = GetScaleY();

  Rectangle panel = {x * sx, y * sy, width * sx, height * sy};

  // Background
  DrawRectangleRounded(panel, 0.15f, 8, Fade(BLACK, alpha));

  // Border glow
  DrawRectangleRoundedLinesEx(panel, 0.15f, 8, 2, Fade(CYBER_ACCENT, 0.8f));

  // Corner accents
  float cornerSize = 8 * sx;
  Color accent = Fade(CYBER_ACCENT, 0.6f);

  // Top-left corner
  DrawLineEx({panel.x, panel.y + cornerSize}, {panel.x, panel.y}, 2, accent);
  DrawLineEx({panel.x, panel.y}, {panel.x + cornerSize, panel.y}, 2, accent);

  // Top-right corner
  DrawLineEx({panel.x + panel.width - cornerSize, panel.y},
             {panel.x + panel.width, panel.y}, 2, accent);
  DrawLineEx({panel.x + panel.width, panel.y},
             {panel.x + panel.width, panel.y + cornerSize}, 2, accent);

  // Bottom-left corner
  DrawLineEx({panel.x, panel.y + panel.height - cornerSize},
             {panel.x, panel.y + panel.height}, 2, accent);
  DrawLineEx({panel.x, panel.y + panel.height},
             {panel.x + cornerSize, panel.y + panel.height}, 2, accent);

  // Bottom-right corner
  DrawLineEx({panel.x + panel.width - cornerSize, panel.y + panel.height},
             {panel.x + panel.width, panel.y + panel.height}, 2, accent);
  DrawLineEx({panel.x + panel.width, panel.y + panel.height - cornerSize},
             {panel.x + panel.width, panel.y + panel.height}, 2, accent);
}

// Draw animated health bar with gradient
inline void DrawHealthBar(float x, float y, float width, float height,
                          float percent, float sx, float sy) {
  Rectangle bg = {x * sx, y * sy, width * sx, height * sy};

  // Background
  DrawRectangleRounded(bg, 0.3f, 8, Fade(BLACK, 0.7f));

  // Determine color based on health
  Color healthColor;
  Color healthColorDark;
  if (percent > 0.6f) {
    healthColor = {0, 255, 100, 255}; // Green
    healthColorDark = {0, 180, 70, 255};
  } else if (percent > 0.3f) {
    healthColor = {255, 220, 0, 255}; // Yellow
    healthColorDark = {200, 170, 0, 255};
  } else {
    healthColor = {255, 60, 60, 255}; // Red
    healthColorDark = {180, 40, 40, 255};
  }

  // Health fill
  float padding = 3 * sx;
  float fillWidth = (width * sx - padding * 2) * percent;
  if (fillWidth > 0) {
    Rectangle fill = {bg.x + padding, bg.y + padding, fillWidth,
                      bg.height - padding * 2};

    // Gradient effect using multiple rectangles
    DrawRectangleRounded(fill, 0.3f, 8, healthColor);

    // Shine effect on top
    Rectangle shine = {fill.x, fill.y, fill.width, fill.height * 0.4f};
    DrawRectangleRounded(shine, 0.3f, 8, Fade(WHITE, 0.2f));
  }

  // Border
  DrawRectangleRoundedLinesEx(bg, 0.3f, 8, 2, Fade(CYBER_ACCENT, 0.9f));

  // Tick marks
  for (int i = 1; i < 4; i++) {
    float tickX = bg.x + (bg.width * i / 4.0f);
    DrawLineEx({tickX, bg.y + 2}, {tickX, bg.y + bg.height - 2}, 1,
               Fade(WHITE, 0.15f));
  }
}

// Draw tower selection card
inline void DrawTowerCard(float x, float y, float width, float height,
                          const char *name, int cost, int hotkey,
                          Texture2D icon, Color themeColor, bool isSelected,
                          bool canAfford, float sx, float sy) {
  Rectangle card = {x * sx, y * sy, width * sx, height * sy};

  // Check hover
  bool isHovered = CheckCollisionPointRec(GetMousePosition(), card);

  // Background with selection/hover effect
  Color bgColor = Fade(BLACK, 0.85f);
  if (isSelected) {
    bgColor = Fade(themeColor, 0.25f);
  } else if (isHovered) {
    bgColor = Fade(themeColor, 0.15f);
  }
  DrawRectangleRounded(card, 0.2f, 8, bgColor);

  // Border
  float borderWidth = isSelected ? 3 : (isHovered ? 2 : 1);
  Color borderColor =
      isSelected ? themeColor
                 : (isHovered ? Fade(themeColor, 0.8f) : Fade(DARKGRAY, 0.8f));
  DrawRectangleRoundedLinesEx(card, 0.2f, 8, borderWidth, borderColor);

  // Icon
  float iconSize = 40 * sx;
  float iconX = card.x + 10 * sx;
  float iconY = card.y + (card.height - iconSize) / 2;

  if (icon.id != 0) {
    Rectangle srcIcon = {0, 0, (float)icon.width, (float)icon.height};
    Rectangle dstIcon = {iconX, iconY, iconSize, iconSize};
    DrawTexturePro(icon, srcIcon, dstIcon, {0, 0}, 0, WHITE);
  } else {
    // Fallback: draw a simple icon
    DrawRectangleV({iconX, iconY}, {iconSize, iconSize},
                   Fade(themeColor, 0.3f));
    DrawCircleV({iconX + iconSize / 2, iconY + iconSize / 2}, iconSize * 0.3f,
                themeColor);
  }

  // Hotkey badge
  float badgeSize = 18 * sx;
  DrawCircleV({card.x + badgeSize, card.y + badgeSize}, badgeSize * 0.8f,
              Fade(themeColor, 0.9f));
  DrawText(TextFormat("%d", hotkey), card.x + badgeSize - 5 * sx,
           card.y + badgeSize - 8 * sx, 14 * sx, WHITE);

  // Tower name
  float textX = iconX + iconSize + 10 * sx;
  float textY = card.y + 12 * sy;
  DrawText(name, textX, textY, 16 * sx, WHITE);

  // Cost
  Color costColor = canAfford ? GREEN : RED;
  DrawText(TextFormat("%d BC", cost), textX, textY + 22 * sy, 14 * sx,
           costColor);

  // Selected indicator
  if (isSelected) {
    DrawText("SELECTED", textX, textY + 42 * sy, 10 * sx,
             Fade(themeColor, 0.8f));
  }
}

// Draw wave/speed info panel
inline void DrawGameStatusPanel(float x, float y, int wave, float timeScale,
                                float sx, float sy) {
  DrawCyberPanel(x, y, 160, 100);

  float px = x * sx;
  float py = y * sy;

  // Wave number
  DrawText("WAVE", px + 15 * sx, py + 12 * sy, 14 * sx, Fade(WHITE, 0.7f));
  DrawText(TextFormat("%d", wave), px + 75 * sx, py + 8 * sy, 32 * sx,
           CYBER_ACCENT);

  // Speed indicator
  Color speedColor = WHITE;
  if (timeScale == 2.0f)
    speedColor = YELLOW;
  else if (timeScale == 3.0f)
    speedColor = ORANGE;
  else if (timeScale >= 4.0f)
    speedColor = RED;

  DrawText("SPEED", px + 15 * sx, py + 52 * sy, 12 * sx, Fade(WHITE, 0.6f));
  DrawText(TextFormat("%.0fx", timeScale), px + 75 * sx, py + 48 * sy, 24 * sx,
           speedColor);

  // Hint
  DrawText("[T] Speed", px + 15 * sx, py + 78 * sy, 10 * sx, Fade(WHITE, 0.4f));
}

// Draw bottom info bar
inline void DrawBottomBar(float autoStartTimer, bool waveActive, float sx,
                          float sy) {
  float barHeight = 40;
  float barY = VIRTUAL_HEIGHT - barHeight - 10;

  // Semi-transparent bar
  Rectangle bar = {10 * sx, barY * sy, (VIRTUAL_WIDTH - 20) * sx,
                   barHeight * sy};
  DrawRectangleRounded(bar, 0.3f, 8, Fade(BLACK, 0.6f));
  DrawRectangleRoundedLinesEx(bar, 0.3f, 8, 1, Fade(CYBER_ACCENT, 0.3f));

  float centerX = GetScreenWidth() / 2;
  float textY = bar.y + (bar.height - 14 * sx) / 2;

  if (!waveActive) {
    // Countdown
    const char *countdown = TextFormat("AUTO-START: %.1fs", autoStartTimer);
    int countWidth = MeasureText(countdown, 16 * sx);
    DrawText(countdown, centerX - countWidth / 2, textY, 16 * sx, YELLOW);
  }

  // Left hints
  DrawText("[ESC] Pause", bar.x + 20 * sx, textY, 12 * sx, Fade(WHITE, 0.5f));
  DrawText("[F] Fullscreen", bar.x + 120 * sx, textY, 12 * sx,
           Fade(WHITE, 0.5f));

  // Right hints
  DrawText("[SPACE] Start Wave", bar.x + bar.width - 160 * sx, textY, 12 * sx,
           Fade(WHITE, 0.5f));
}

#endif // HUD_H
