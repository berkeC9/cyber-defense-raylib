#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include "constants.h"
#include "raylib.h"
#include <string>
#include <vector>

// ==================== MUSIC MANAGER SYSTEM ====================
// Handles dynamic royalty-free synth music based on game state and wave phase.
// Guards every Raylib audio operation so missing files or unavailable audio
// devices never cause crashes or undefined behavior.

enum class MusicPhase { EARLY, MID, LATE };

class MusicManager {
public:
  Music menuMusic = {0};
  Music pauseMusic = {0};
  std::vector<Music> earlyGameMusic;
  std::vector<Music> midGameMusic;
  std::vector<Music> lateGameMusic;

  Music *currentGameMusic = nullptr;
  int currentTrackIndex = 0;
  float savedPosition = 0.0f;
  float masterVolume = 0.75f;
  bool musicLoaded = false;
  bool isGameMusicPaused = false;
  MusicPhase currentPhase = MusicPhase::EARLY;

  MusicManager() = default;
  MusicManager(const MusicManager &) = delete;
  MusicManager &operator=(const MusicManager &) = delete;

  ~MusicManager() { unload(); }

private:
  static Music LoadSafeTrack(const std::string &relativeAssetPath,
                             bool loop = true) {
    Music stream = {0};
    if (!IsAudioDeviceReady()) {
      return stream;
    }

    std::string resolved = GetAssetPath(relativeAssetPath);
    if (!FileExists(resolved.c_str())) {
      return stream;
    }

    stream = LoadMusicStream(resolved.c_str());
    if (IsMusicValid(stream)) {
      stream.looping = loop;
    } else {
      stream = {0};
    }
    return stream;
  }

  static void SafePlay(Music &m) {
    if (IsAudioDeviceReady() && IsMusicValid(m)) {
      PlayMusicStream(m);
    }
  }

  static void SafeStop(Music &m) {
    if (IsAudioDeviceReady() && IsMusicValid(m)) {
      StopMusicStream(m);
    }
  }

  static void SafePause(Music &m) {
    if (IsAudioDeviceReady() && IsMusicValid(m)) {
      PauseMusicStream(m);
    }
  }

  static void SafeResume(Music &m) {
    if (IsAudioDeviceReady() && IsMusicValid(m)) {
      ResumeMusicStream(m);
    }
  }

  static void SafeUpdate(Music &m) {
    if (IsAudioDeviceReady() && IsMusicValid(m) && IsMusicStreamPlaying(m)) {
      UpdateMusicStream(m);
    }
  }

  static void SafeUnload(Music &m) {
    if (IsMusicValid(m)) {
      UnloadMusicStream(m);
    }
    m = {0};
  }

  static void SafeVolume(Music &m, float volume) {
    if (IsAudioDeviceReady() && IsMusicValid(m)) {
      SetMusicVolume(m, volume);
    }
  }

  static MusicPhase ComputePhase(int currentWave) {
    if (currentWave <= 5) {
      return MusicPhase::EARLY;
    }
    if (currentWave <= 10) {
      return MusicPhase::MID;
    }
    return MusicPhase::LATE;
  }

public:
  void load() {
    if (musicLoaded) {
      return;
    }

    menuMusic = LoadSafeTrack("songs/menu_theme.wav", true);
    pauseMusic = LoadSafeTrack("songs/menu_theme.wav", true);

    Music earlyTrack = LoadSafeTrack("songs/early_wave.wav", true);
    if (IsMusicValid(earlyTrack)) {
      earlyGameMusic.push_back(earlyTrack);
    }

    Music midTrack = LoadSafeTrack("songs/mid_wave.wav", true);
    if (IsMusicValid(midTrack)) {
      midGameMusic.push_back(midTrack);
    }

    Music lateTrack = LoadSafeTrack("songs/late_wave.wav", true);
    if (IsMusicValid(lateTrack)) {
      lateGameMusic.push_back(lateTrack);
    }

    setMasterVolume(masterVolume);
    musicLoaded = true;
  }

  void unload() {
    if (!musicLoaded) {
      return;
    }

    stopAllMusic();

    SafeUnload(menuMusic);
    SafeUnload(pauseMusic);

    for (auto &m : earlyGameMusic) {
      SafeUnload(m);
    }
    for (auto &m : midGameMusic) {
      SafeUnload(m);
    }
    for (auto &m : lateGameMusic) {
      SafeUnload(m);
    }

    earlyGameMusic.clear();
    midGameMusic.clear();
    lateGameMusic.clear();
    currentGameMusic = nullptr;
    musicLoaded = false;
  }

  // ==================== MENU MUSIC ====================
  void playMenuMusic() {
    stopAllMusic();
    SafePlay(menuMusic);
  }

  void stopMenuMusic() { SafeStop(menuMusic); }

  // ==================== PAUSE MUSIC ====================
  void pauseGameMusicAndPlayPause() {
    if (currentGameMusic != nullptr && IsAudioDeviceReady() &&
        IsMusicValid(*currentGameMusic) &&
        IsMusicStreamPlaying(*currentGameMusic)) {
      savedPosition = GetMusicTimePlayed(*currentGameMusic);
      SafePause(*currentGameMusic);
      isGameMusicPaused = true;
    }

    SafeStop(menuMusic);
    SafePlay(pauseMusic);
  }

  void resumeGameMusic() {
    SafeStop(pauseMusic);

    if (currentGameMusic != nullptr && isGameMusicPaused) {
      SafeResume(*currentGameMusic);
      isGameMusicPaused = false;
    }
  }

  // ==================== IN-GAME MUSIC ====================
  void startGameMusic(int currentWave) {
    stopAllMusic();
    updatePhase(currentWave);
    currentTrackIndex = 0;
    playCurrentPhaseMusic();
  }

  void updatePhase(int currentWave) { currentPhase = ComputePhase(currentWave); }

  void playCurrentPhaseMusic() {
    std::vector<Music> *musicList = nullptr;

    switch (currentPhase) {
    case MusicPhase::EARLY:
      musicList = &earlyGameMusic;
      break;
    case MusicPhase::MID:
      musicList = !midGameMusic.empty() ? &midGameMusic : &earlyGameMusic;
      break;
    case MusicPhase::LATE:
      musicList = !lateGameMusic.empty() ? &lateGameMusic : &earlyGameMusic;
      break;
    }

    if (musicList != nullptr && !musicList->empty()) {
      if (currentTrackIndex >= (int)musicList->size()) {
        currentTrackIndex = 0;
      }
      currentGameMusic = &(*musicList)[currentTrackIndex];
      SafePlay(*currentGameMusic);
    } else {
      currentGameMusic = nullptr;
    }
  }

  void update(int currentWave) {
    if (!musicLoaded || !IsAudioDeviceReady()) {
      return;
    }

    SafeUpdate(menuMusic);
    SafeUpdate(pauseMusic);

    for (auto &m : earlyGameMusic) {
      SafeUpdate(m);
    }
    for (auto &m : midGameMusic) {
      SafeUpdate(m);
    }
    for (auto &m : lateGameMusic) {
      SafeUpdate(m);
    }

    if (currentGameMusic != nullptr && !isGameMusicPaused &&
        IsMusicValid(*currentGameMusic)) {
      // Transition seamlessly when wave progresses into a new intensity phase
      MusicPhase targetPhase = ComputePhase(currentWave);
      if (targetPhase != currentPhase) {
        SafeStop(*currentGameMusic);
        currentPhase = targetPhase;
        currentTrackIndex = 0;
        playCurrentPhaseMusic();
        return;
      }

      // Advance non-looping playlists if applicable
      if (!currentGameMusic->looping) {
        float timePlayed = GetMusicTimePlayed(*currentGameMusic);
        float timeLength = GetMusicTimeLength(*currentGameMusic);
        if (timeLength > 0.0f && timePlayed >= timeLength - 0.1f) {
          currentTrackIndex++;
          playCurrentPhaseMusic();
        }
      }
    }
  }

  void stopAllMusic() {
    SafeStop(menuMusic);
    SafeStop(pauseMusic);

    for (auto &m : earlyGameMusic) {
      SafeStop(m);
    }
    for (auto &m : midGameMusic) {
      SafeStop(m);
    }
    for (auto &m : lateGameMusic) {
      SafeStop(m);
    }

    currentGameMusic = nullptr;
    isGameMusicPaused = false;
  }

  void setMasterVolume(float volume) {
    if (volume < 0.0f) {
      volume = 0.0f;
    } else if (volume > 1.0f) {
      volume = 1.0f;
    }
    masterVolume = volume;

    SafeVolume(menuMusic, volume);
    SafeVolume(pauseMusic, volume * 0.55f);

    for (auto &m : earlyGameMusic) {
      SafeVolume(m, volume);
    }
    for (auto &m : midGameMusic) {
      SafeVolume(m, volume);
    }
    for (auto &m : lateGameMusic) {
      SafeVolume(m, volume);
    }
  }
};

#endif // MUSIC_MANAGER_H
