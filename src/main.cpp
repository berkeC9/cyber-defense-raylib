// 1. WINDOWS VE RAYLIB ÇAKIŞMALARINI ÖNLEME
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

// Raylib fonksiyonlarını korumak için Windows makrolarını temizle
#undef CloseWindow
#undef ShowCursor
#undef PlaySound
#undef DrawText
#undef LoadImage
#undef DrawTextEx

#include "constants.h"
#include "entityManager.h"
#include "hud.h"
#include "map.h"
#include "menu.h"
#include "music_manager.h"
#include "raylib.h"
#include "waveManager.h"
#include "animated_sprite.h"

#include <math.h>
#include <string>

// -----------------------
// Helpers
// -----------------------
static std::string ReplaceTail(const std::string& s, const std::string& from, const std::string& to) {
  std::string out = s;
  size_t pos = out.rfind(from);
  if (pos != std::string::npos) out.replace(pos, from.size(), to);
  return out;
}

static Texture2D LoadTowerTextureAnyExt(const std::string& baseNoExt) {
  // baseNoExt örn: "towers/firewall/level_5/level_5"
  std::string p1 = GetAssetPath(baseNoExt + ".png");
  if (FileExists(p1.c_str())) return LoadTexture(p1.c_str());

  std::string p2 = GetAssetPath(baseNoExt + ".jpg");
  if (FileExists(p2.c_str())) return LoadTexture(p2.c_str());

  std::string p3 = GetAssetPath(baseNoExt + ".jpeg");
  if (FileExists(p3.c_str())) return LoadTexture(p3.c_str());

  Texture2D empty = {0};
  return empty;
}

static void DrawTextureCenteredInRect(Texture2D* tex, Rectangle rect, float targetW, Color tint) {
  if (!tex || tex->id == 0) return;

  float w = targetW;
  float aspect = (float)tex->height / (float)tex->width;
  float h = w * aspect;

  Vector2 center = { rect.x + rect.width * 0.18f, rect.y + rect.height * 0.52f }; // kartın sol-orta kısmı
  Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
  Rectangle dst = {center.x, center.y, w, h};
  Vector2 origin = {w / 2.0f, h / 2.0f};

  DrawTexturePro(*tex, src, dst, origin, 0.0f, tint);
}

int main() {
  InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Cyber Defense - Tower Defense Game");
  SetTargetFPS(60);
  SetExitKey(0);
  InitAudioDevice();

  // --- MUSIC SYSTEM ---
  MusicManager musicManager;
  musicManager.load();
  musicManager.playMenuMusic();

  // --- MENU SYSTEM ---
  GameState gameState = GameState::MENU;
  Menu mainMenu;
  SettingsMenu settingsMenu;
  mainMenu.loadBackground();

  // --- HUD ASSETS ---
  HUDAssets hudAssets;
  hudAssets.load();

  // --- GAME STATE ---
  Map gameMap;
  EntityManager manager;
  WaveManager waveSystem;

  // =========================================================
  // ENEMY ASSETS (Texture + Optional Animation)
  // =========================================================
  AnimatedSprite wormAnim;
  AnimatedSprite spywareAnim;
  AnimatedSprite trojanAnim;

  std::string wormFirst    = GetAssetPath("enemies/worm/worm_frames/worm_000.png");
  std::string spywareFirst = GetAssetPath("enemies/spyware/spyware_frames/spyware_000.png");
  std::string trojanFirst  = GetAssetPath("enemies/trojan/trojan_frames/trojan_000.png");

  std::string wormPattern    = ReplaceTail(wormFirst,    "worm_000.png",    "worm_%03d.png");
  std::string spywarePattern = ReplaceTail(spywareFirst, "spyware_000.png", "spyware_%03d.png");
  std::string trojanPattern  = ReplaceTail(trojanFirst,  "trojan_000.png",  "trojan_%03d.png");

  wormAnim.LoadSequence(wormPattern, 0, 200);
  spywareAnim.LoadSequence(spywarePattern, 0, 200);
  trojanAnim.LoadSequence(trojanPattern, 0, 200);

  wormAnim.SetFPS(24.0f);
  spywareAnim.SetFPS(24.0f);
  trojanAnim.SetFPS(24.0f);

  Enemy::SetWormAnim(&wormAnim);
  Enemy::SetSpywareAnim(&spywareAnim);
  Enemy::SetTrojanAnim(&trojanAnim);

  Texture2D wormTex = {0};
  Texture2D spywareTex = {0};
  Texture2D trojanTex = {0};

  if (FileExists(wormFirst.c_str()))    wormTex = LoadTexture(wormFirst.c_str());
  if (FileExists(spywareFirst.c_str())) spywareTex = LoadTexture(spywareFirst.c_str());
  if (FileExists(trojanFirst.c_str()))  trojanTex = LoadTexture(trojanFirst.c_str());

  Enemy::SetTextures(&wormTex, &spywareTex, &trojanTex);

  // =========================================================
  // TOWER ASSETS (Texture ONLY)  -> Cooler artık frame çekmiyor
  // =========================================================
  Texture2D firewallTex[6] = {};
  Texture2D logicBombTex[6] = {};
  Texture2D defragTex[6] = {};
  Texture2D dataSpikeTex[6] = {};

  for (int lvl = 1; lvl <= 6; lvl++) {
    int idx = lvl - 1;

    firewallTex[idx] = LoadTowerTextureAnyExt("towers/firewall/level_" + std::to_string(lvl) + "/level_" + std::to_string(lvl));
    Tower::SetFirewallTexture(idx, &firewallTex[idx]);

    logicBombTex[idx] = LoadTowerTextureAnyExt("towers/logic_bomb/level_" + std::to_string(lvl) + "/level_" + std::to_string(lvl));
    Tower::SetLogicBombTexture(idx, &logicBombTex[idx]);

    // COOLER (Defrag) artık direkt: bin/assets/towers/cooler/level_X/level_X.png
    defragTex[idx] = LoadTowerTextureAnyExt("towers/cooler/level_" + std::to_string(lvl) + "/level_" + std::to_string(lvl));
    Tower::SetDefragTexture(idx, &defragTex[idx]);

    dataSpikeTex[idx] = LoadTowerTextureAnyExt("towers/data_spike/level_" + std::to_string(lvl) + "/level_" + std::to_string(lvl));
    Tower::SetDataSpikeTexture(idx, &dataSpikeTex[idx]);

    // Cooler anim/frame tamamen kapalı
    Tower::SetDefragAnim(idx, nullptr);
  }

  // --- GAME VARS ---
  float health = PLAYER_STARTING_HEALTH;
  int bitcoin = (int)STARTING_BITCOIN;
  float autoStartTimer = 10.0f;
  float timeScale = 1.0f;

  TowerType selectedTowerType = TowerType::FIREWALL;
  Tower *uiSelectedTower = nullptr;
  GameState previousState = GameState::MENU;

  auto resetGame = [&]() {
    health = PLAYER_STARTING_HEALTH;
    bitcoin = (int)STARTING_BITCOIN;
    autoStartTimer = 10.0f;
    timeScale = 1.0f;
    waveSystem = WaveManager();
    manager.clear();
    gameMap.reset();
    uiSelectedTower = nullptr;
    selectedTowerType = TowerType::FIREWALL;
  };

  bool shouldExit = false;

  while (!WindowShouldClose() && !shouldExit) {
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.016f;

    if (IsKeyPressed(KEY_T)) {
      timeScale += 1.0f;
      if (timeScale > 4.0f) timeScale = 1.0f;
    }

    float finalDt = dt * timeScale;

    // --- ANIM UPDATE (once per frame) ---
    if (gameState == GameState::PLAYING) {
      wormAnim.Update(finalDt);
      spywareAnim.Update(finalDt);
      trojanAnim.Update(finalDt);
    }

    musicManager.update(waveSystem.currentWave);

    switch (gameState) {

    // ==================== MENU ====================
    case GameState::MENU: {
      mainMenu.update();

      int menuResult = mainMenu.handleInput();
      if (menuResult == 1) {
        resetGame();
        musicManager.startGameMusic(waveSystem.currentWave);
        gameState = GameState::PLAYING;
      } else if (menuResult == 2) {
        previousState = GameState::MENU;
        gameState = GameState::SETTINGS;
      } else if (menuResult == 3) {
        // exit after draw
      }

      BeginDrawing();
      mainMenu.draw();
      EndDrawing();

      if (menuResult == 3) {
        shouldExit = true;
      }
      break;
    }

    // ==================== SETTINGS ====================
    case GameState::SETTINGS: {
      settingsMenu.update();
      int settingsResult = settingsMenu.handleInput();
      if (settingsResult == -1) {
        gameState = previousState;
      }

      BeginDrawing();
      settingsMenu.draw();
      EndDrawing();
      break;
    }

    // ==================== PLAYING ====================
    case GameState::PLAYING: {
      if (IsKeyPressed(KEY_ESCAPE)) {
        musicManager.pauseGameMusicAndPlayPause();
        gameState = GameState::PAUSED;
      }

      if (IsKeyPressed(KEY_F)) ToggleFullscreen();

      if (IsKeyPressed(KEY_ONE))   selectedTowerType = TowerType::FIREWALL;
      if (IsKeyPressed(KEY_TWO))   selectedTowerType = TowerType::LOGIC_BOMB;
      if (IsKeyPressed(KEY_THREE)) selectedTowerType = TowerType::DEFRAGMENTER;
      if (IsKeyPressed(KEY_FOUR)) selectedTowerType = TowerType::DATA_SPIKE;

      // Wave control
      if (!waveSystem.waveActive) {
        autoStartTimer -= finalDt;
        if (IsKeyPressed(KEY_SPACE) || autoStartTimer <= 0) {
          waveSystem.startNextWave();
          autoStartTimer = 10.0f;
        }
      } else {
        autoStartTimer = 10.0f;
      }

      waveSystem.update(finalDt, manager, gameMap);

      // ----------------------------
      // INPUT: BUILD / SELECT / UPGRADE / SELL
      // ----------------------------
      float sx = GetScaleX(); float sy = GetScaleY();

      // ---------- BUY UI CARD LAYOUT (4 cards) ----------
      float cardWidth = 170 * sx;
      float cardHeight = 70 * sy;
      float cardSpacing = 15 * sx;
      float totalCardWidth = (cardWidth * 4) + (cardSpacing * 3);
      float cardStartX = (GetScreenWidth() - totalCardWidth) / 2;
      float cardY = 10 * sy;

      Rectangle cardFirewall = { cardStartX + (0 * (cardWidth + cardSpacing)), cardY, cardWidth, cardHeight };
      Rectangle cardLogic    = { cardStartX + (1 * (cardWidth + cardSpacing)), cardY, cardWidth, cardHeight };
      Rectangle cardDefrag   = { cardStartX + (2 * (cardWidth + cardSpacing)), cardY, cardWidth, cardHeight };
      Rectangle cardDataSpike = { cardStartX + (3 * (cardWidth + cardSpacing)), cardY, cardWidth, cardHeight };

      auto getUpgradePanelRect = [&](const Tower *tower) {
        return Rectangle{(tower->pos.x * sx) + 40 * sx,
                         (tower->pos.y * sy) + 15 * sy, 140 * sx, 75 * sy};
      };

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool uiAction = false;

        // 1) BUY CARD CLICK (4 cards)
        if (CheckCollisionPointRec(GetMousePosition(), cardFirewall)) {
          selectedTowerType = TowerType::FIREWALL;
          uiAction = true;
        } else if (CheckCollisionPointRec(GetMousePosition(), cardLogic)) {
          selectedTowerType = TowerType::LOGIC_BOMB;
          uiAction = true;
        } else if (CheckCollisionPointRec(GetMousePosition(), cardDefrag)) {
          selectedTowerType = TowerType::DEFRAGMENTER;
          uiAction = true;
        } else if (CheckCollisionPointRec(GetMousePosition(), cardDataSpike)) {
          selectedTowerType = TowerType::DATA_SPIKE;
          uiAction = true;
        }

        // 2) Upgrade click
        if (!uiAction && uiSelectedTower != nullptr) {
          Rectangle upgradeBtn = getUpgradePanelRect(uiSelectedTower);
          if (CheckCollisionPointRec(GetMousePosition(), upgradeBtn)) {
            int uCost = uiSelectedTower->getUpgradeCost();
            if (bitcoin >= uCost && uiSelectedTower->level < 6) {
              bitcoin -= uCost;
              uiSelectedTower->upgrade();
            }
            uiAction = true;
          }
        }

        // 3) Select existing tower
        if (!uiAction) {
          bool towerFound = false;
          for (auto t : manager.towers) {
            if (t->isMouseOver()) {
              uiSelectedTower = t;
              towerFound = true;
              uiAction = true;
              break;
            }
          }
          if (!towerFound) uiSelectedTower = nullptr;
        }

        // 4) Build on socket
        if (!uiAction) {
          for (auto& s : gameMap.sockets) {
            int currentCost =
              (selectedTowerType == TowerType::FIREWALL) ? 100 :
              (selectedTowerType == TowerType::LOGIC_BOMB) ? 250 :
              (selectedTowerType == TowerType::DEFRAGMENTER) ? 150 :
              (selectedTowerType == TowerType::DATA_SPIKE) ? 300 : 999999;

            if (!s.isOccupied && s.isMouseOver() && bitcoin >= currentCost) {
              bitcoin -= currentCost;
              s.isOccupied = true;
              manager.towers.push_back(new Tower(s.position, selectedTowerType));
              break;
            }
          }
        }
      }

      // Right click sell
      if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        for (auto& s : gameMap.sockets) {
          if (s.isOccupied && s.isMouseOver()) {
            bitcoin += 75;
            s.isOccupied = false;
            for (int i = (int)manager.towers.size() - 1; i >= 0; i--) {
              if (manager.towers[i]->pos.x == s.position.x && manager.towers[i]->pos.y == s.position.y) {
                if (uiSelectedTower == manager.towers[i]) {
                  uiSelectedTower = nullptr;
                }
                delete manager.towers[i];
                manager.towers.erase(manager.towers.begin() + i);
                break;
              }
            }
          }
        }
      }

      // Systems update
      int rewardThisFrame = 0;
      float frameDamage = manager.updateAll(finalDt, rewardThisFrame);
      health -= frameDamage;
      bitcoin += rewardThisFrame;

      if (health <= 0) gameState = GameState::GAME_OVER;

      // ----------------------------
      // DRAW
      // ----------------------------
      BeginDrawing();
      ClearBackground(CYBER_BG);

      gameMap.draw();
      manager.drawAll();

      // Ghost range preview
      for (auto &s : gameMap.sockets) {
        if (!s.isOccupied && s.isMouseOver()) {
          float previewRange =
            (selectedTowerType == TowerType::FIREWALL) ? 200.0f :
            (selectedTowerType == TowerType::LOGIC_BOMB) ? 180.0f :
            (selectedTowerType == TowerType::DEFRAGMENTER) ? 140.0f : 300.0f;

          Color previewColor =
            (selectedTowerType == TowerType::FIREWALL) ? CYBER_ACCENT :
            (selectedTowerType == TowerType::LOGIC_BOMB) ? PURPLE :
            (selectedTowerType == TowerType::DATA_SPIKE) ? GOLD : SKYBLUE;

          Vector2 screenPos = {s.position.x * sx, s.position.y * sy};

          DrawCircleLines(screenPos.x, screenPos.y, previewRange * sx, Fade(previewColor, 0.4f));
          DrawCircle(screenPos.x, screenPos.y, previewRange * sx, Fade(previewColor, 0.1f));
          DrawRectangleV({screenPos.x - 22 * sx, screenPos.y - 22 * sy}, {44 * sx, 44 * sy}, Fade(DARKGRAY, 0.5f));
          DrawCircleV(screenPos, 12 * sx, Fade(previewColor, 0.6f));
          break;
        }
      }

      // Entry point warnings
      float blink = (sinf(GetTime() * 10.0f) + 1.0f) / 2.0f;
      Vector2 spawns[4] = {gameMap.pathTop[0], gameMap.pathBottom[0], gameMap.pathBottom2[0], gameMap.pathBottom3[0]};
      for (int i = 0; i < 4; i++) {
        DrawText("!", spawns[i].x * sx - 10, spawns[i].y * sy - 20, 30, Fade(RED, blink));
      }

      // ==================== HUD ====================
      DrawCyberPanel(10, 10, 200, 100);

      float iconSize = 28 * sx;
      if (hudAssets.shieldIcon.id != 0) {
        Rectangle srcIcon = {0, 0, (float)hudAssets.shieldIcon.width, (float)hudAssets.shieldIcon.height};
        Rectangle dstIcon = {20 * sx, 18 * sy, iconSize, iconSize};
        DrawTexturePro(hudAssets.shieldIcon, srcIcon, dstIcon, {0, 0}, 0, WHITE);
      }
      DrawText("INTEGRITY", 55 * sx, 22 * sy, 12 * sx, Fade(WHITE, 0.7f));

      float healthPercent = health / PLAYER_STARTING_HEALTH;
      if (healthPercent < 0) healthPercent = 0;
      if (healthPercent > 1) healthPercent = 1;
      DrawHealthBar(20, 48, 180, 22, healthPercent, sx, sy);
      DrawText(TextFormat("%.0f%%", health), 95 * sx, 52 * sy, 14 * sx, WHITE);

      if (hudAssets.bitcoinIcon.id != 0) {
        Rectangle srcIcon = {0, 0, (float)hudAssets.bitcoinIcon.width, (float)hudAssets.bitcoinIcon.height};
        Rectangle dstIcon = {20 * sx, 78 * sy, 22 * sx, 22 * sy};
        DrawTexturePro(hudAssets.bitcoinIcon, srcIcon, dstIcon, {0, 0}, 0, WHITE);
      }
      DrawText(TextFormat("%d BC", bitcoin), 50 * sx, 80 * sy, 18 * sx, CYBER_ACCENT);

      // ---------- BUY CARDS (with correct costs) ----------
      DrawTowerCard(cardFirewall.x, cardFirewall.y, cardFirewall.width, cardFirewall.height,
                    "FIREWALL", 100, 1, hudAssets.firewallIcon, CYBER_ACCENT,
                    selectedTowerType == TowerType::FIREWALL, bitcoin >= 100, sx, sy);

      DrawTowerCard(cardLogic.x, cardLogic.y, cardLogic.width, cardLogic.height,
                    "LOGIC BOMB", 250, 2, hudAssets.logicBombIcon, PURPLE,
                    selectedTowerType == TowerType::LOGIC_BOMB, bitcoin >= 250, sx, sy);

      DrawTowerCard(cardDefrag.x, cardDefrag.y, cardDefrag.width, cardDefrag.height,
                    "DEFRAGMENTER", 150, 3, hudAssets.defragmenterIcon, SKYBLUE,
                    selectedTowerType == TowerType::DEFRAGMENTER, bitcoin >= 150, sx, sy);

      DrawTowerCard(cardDataSpike.x, cardDataSpike.y, cardDataSpike.width, cardDataSpike.height,
                    "DATA SPIKE", 300, 4, hudAssets.dataSpikeIcon, GOLD,
                    selectedTowerType == TowerType::DATA_SPIKE, bitcoin >= 300, sx, sy);

      // ---------- BUY CARD: ASSET PREVIEW OVERLAY ----------
      DrawTextureCenteredInRect(&firewallTex[0], cardFirewall, 40 * sx, WHITE);
      DrawTextureCenteredInRect(&logicBombTex[0], cardLogic, 40 * sx, WHITE);

      // COOLER artık anim değil, direkt texture preview
      DrawTextureCenteredInRect(&defragTex[0], cardDefrag, 40 * sx, WHITE);

      DrawTextureCenteredInRect(&dataSpikeTex[0], cardDataSpike, 40 * sx, WHITE);

      DrawGameStatusPanel(VIRTUAL_WIDTH - 170, 10, waveSystem.currentWave, timeScale, sx, sy);

      // Upgrade UI
      if (uiSelectedTower != nullptr) {
        Vector2 sPos = {uiSelectedTower->pos.x * sx, uiSelectedTower->pos.y * sy};

        DrawCircleLines(sPos.x, sPos.y, uiSelectedTower->range * sx, Fade(CYBER_ACCENT, 0.4f));
        DrawCircle(sPos.x, sPos.y, uiSelectedTower->range * sx, Fade(CYBER_ACCENT, 0.08f));

        Rectangle upgradePanel = getUpgradePanelRect(uiSelectedTower);
        DrawRectangleRounded(upgradePanel, 0.2f, 8, Fade(BLACK, 0.9f));
        DrawRectangleRoundedLinesEx(upgradePanel, 0.2f, 8, 2, CYBER_ACCENT);

        if (uiSelectedTower->level < 6) {
          DrawText(TextFormat("LEVEL %d", uiSelectedTower->level),
                   upgradePanel.x + 10 * sx, upgradePanel.y + 10 * sy, 14 * sx, WHITE);
          DrawText(TextFormat("UPGRADE: %d BC", uiSelectedTower->getUpgradeCost()),
                   upgradePanel.x + 10 * sx, upgradePanel.y + 30 * sy, 12 * sx,
                   (bitcoin >= uiSelectedTower->getUpgradeCost() ? GREEN : RED));
          DrawText("[CLICK TO UPGRADE]", upgradePanel.x + 10 * sx,
                   upgradePanel.y + 50 * sy, 10 * sx, Fade(CYBER_ACCENT, 0.7f));
        } else {
          DrawText("MAX LEVEL", upgradePanel.x + 20 * sx, upgradePanel.y + 25 * sy, 16 * sx, GOLD);
          DrawText("FULLY UPGRADED", upgradePanel.x + 15 * sx, upgradePanel.y + 48 * sy, 10 * sx, Fade(WHITE, 0.5f));
        }
      }

      DrawBottomBar(autoStartTimer, waveSystem.waveActive, sx, sy);

      EndDrawing();
      break;
    }

    // ==================== PAUSED ====================
    case GameState::PAUSED: {
      if (IsKeyPressed(KEY_ESCAPE)) {
        musicManager.resumeGameMusic();
        gameState = GameState::PLAYING;
      }

      BeginDrawing();
      ClearBackground(CYBER_BG);

      float sx = GetScaleX();
      float sy = GetScaleY();

      gameMap.draw();
      manager.drawAll();

      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.75f));

      const char *pauseTitle = "GAME PAUSED";
      int titleSize = (int)(50 * sx);
      int titleWidth = MeasureText(pauseTitle, titleSize);
      float titleX = (GetScreenWidth() - titleWidth) / 2;
      float titleY = 150 * sy;

      DrawText(pauseTitle, (int)titleX + 2, (int)titleY + 2, titleSize, Fade(BLACK, 0.5f));
      DrawText(pauseTitle, (int)titleX, (int)titleY, titleSize, CYBER_ACCENT);

      float btnWidth = 300 * sx;
      float btnHeight = 60 * sy;
      float btnX = (GetScreenWidth() - btnWidth) / 2;
      float btnSpacing = 80 * sy;
      float btnStartY = 280 * sy;

      Rectangle resumeBtn = {btnX, btnStartY, btnWidth, btnHeight};
      bool resumeHover = CheckCollisionPointRec(GetMousePosition(), resumeBtn);
      DrawRectangleRounded(resumeBtn, 0.3f, 8, Fade(resumeHover ? CYBER_ACCENT : DARKGRAY, 0.8f));
      DrawRectangleRoundedLinesEx(resumeBtn, 0.3f, 8, 2, CYBER_ACCENT);
      const char *resumeText = "RESUME";
      int resumeTextWidth = MeasureText(resumeText, (int)(25 * sx));
      DrawText(resumeText,
               (int)(resumeBtn.x + (btnWidth - resumeTextWidth) / 2),
               (int)(resumeBtn.y + (btnHeight - 25 * sx) / 2),
               (int)(25 * sx), WHITE);

      Rectangle settingsBtn = {btnX, btnStartY + btnSpacing, btnWidth, btnHeight};
      bool settingsHover = CheckCollisionPointRec(GetMousePosition(), settingsBtn);
      DrawRectangleRounded(settingsBtn, 0.3f, 8, Fade(settingsHover ? CYBER_ACCENT : DARKGRAY, 0.8f));
      DrawRectangleRoundedLinesEx(settingsBtn, 0.3f, 8, 2, CYBER_ACCENT);
      const char *settingsText = "SETTINGS";
      int settingsTextWidth = MeasureText(settingsText, (int)(25 * sx));
      DrawText(settingsText,
               (int)(settingsBtn.x + (btnWidth - settingsTextWidth) / 2),
               (int)(settingsBtn.y + (btnHeight - 25 * sx) / 2),
               (int)(25 * sx), WHITE);

      Rectangle menuBtn = {btnX, btnStartY + btnSpacing * 2, btnWidth, btnHeight};
      bool menuHover = CheckCollisionPointRec(GetMousePosition(), menuBtn);
      DrawRectangleRounded(menuBtn, 0.3f, 8, Fade(menuHover ? RED : DARKGRAY, 0.8f));
      DrawRectangleRoundedLinesEx(menuBtn, 0.3f, 8, 2, RED);
      const char *menuText = "RETURN TO MAIN MENU";
      int menuTextWidth = MeasureText(menuText, (int)(25 * sx));
      DrawText(menuText,
               (int)(menuBtn.x + (btnWidth - menuTextWidth) / 2),
               (int)(menuBtn.y + (btnHeight - 25 * sx) / 2),
               (int)(25 * sx), WHITE);

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (resumeHover) {
          musicManager.resumeGameMusic();
          gameState = GameState::PLAYING;
        } else if (settingsHover) {
          previousState = GameState::PAUSED;
          gameState = GameState::SETTINGS;
        } else if (menuHover) {
          musicManager.stopAllMusic();
          musicManager.playMenuMusic();
          gameState = GameState::MENU;
        }
      }

      const char *escInfo = "[ESC] Resume";
      int escInfoWidth = MeasureText(escInfo, (int)(16 * sx));
      DrawText(escInfo, (GetScreenWidth() - escInfoWidth) / 2,
               GetScreenHeight() - 50 * sy, (int)(16 * sx), Fade(WHITE, 0.5f));

      EndDrawing();
      break;
    }

    // ==================== GAME OVER ====================
    case GameState::GAME_OVER: {
      BeginDrawing();
      ClearBackground(CYBER_BG);

      float sx = GetScaleX();
      float sy = GetScaleY();

      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));

      const char *gameOverText = "SYSTEM CRASHED";
      int goSize = (int)(60 * sx);
      int goWidth = MeasureText(gameOverText, goSize);
      float goX = (GetScreenWidth() - goWidth) / 2;
      float goY = GetScreenHeight() / 2 - 80 * sy;

      DrawText(gameOverText, (int)goX + 3, (int)goY + 3, goSize, Fade(BLACK, 0.5f));
      DrawText(gameOverText, (int)goX, (int)goY, goSize, RED);

      const char *waveInfo = TextFormat("Last Wave: %d", waveSystem.currentWave);
      int wiSize = (int)(30 * sx);
      int wiWidth = MeasureText(waveInfo, wiSize);
      DrawText(waveInfo, (GetScreenWidth() - wiWidth) / 2,
               (int)(goY + goSize + 30 * sy), wiSize, WHITE);

      Rectangle menuBtn = {GetScreenWidth() / 2 - 150 * sx,
                           GetScreenHeight() / 2 + 50 * sy, 300 * sx, 60 * sy};

      bool btnHover = CheckCollisionPointRec(GetMousePosition(), menuBtn);
      Color btnColor = btnHover ? CYBER_ACCENT : Fade(CYBER_ACCENT, 0.6f);

      DrawRectangleRounded(menuBtn, 0.3f, 8, Fade(btnColor, 0.3f));
      DrawRectangleRoundedLinesEx(menuBtn, 0.3f, 8, 3, btnColor);

      const char *btnText = "RETURN TO MAIN MENU";
      int btnTextSize = (int)(25 * sx);
      int btnTextWidth = MeasureText(btnText, btnTextSize);
      DrawText(btnText, (int)(menuBtn.x + (menuBtn.width - btnTextWidth) / 2),
               (int)(menuBtn.y + (menuBtn.height - btnTextSize) / 2),
               btnTextSize, WHITE);

      if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        musicManager.stopAllMusic();
        musicManager.playMenuMusic();
        gameState = GameState::MENU;
      }

      if (IsKeyPressed(KEY_ESCAPE)) {
        musicManager.stopAllMusic();
        musicManager.playMenuMusic();
        gameState = GameState::MENU;
      }

      EndDrawing();
      break;
    }

    } // switch
  }   // while

  // ----------------------------
  // UNLOAD
  // ----------------------------
  manager.clear();
  gameMap.unload();
  mainMenu.unloadBackground();
  hudAssets.unload();
  musicManager.unload();

  // Enemy textures + anim
  if (wormTex.id != 0) UnloadTexture(wormTex);
  if (spywareTex.id != 0) UnloadTexture(spywareTex);
  if (trojanTex.id != 0) UnloadTexture(trojanTex);

  wormAnim.Unload();
  spywareAnim.Unload();
  trojanAnim.Unload();

  // Tower textures
  for (int i = 0; i < 6; i++) {
    if (firewallTex[i].id != 0) UnloadTexture(firewallTex[i]);
    if (logicBombTex[i].id != 0) UnloadTexture(logicBombTex[i]);
    if (defragTex[i].id != 0) UnloadTexture(defragTex[i]);
    if (dataSpikeTex[i].id != 0) UnloadTexture(dataSpikeTex[i]);
  }

  CloseAudioDevice();
  CloseWindow();
  return 0;
}
