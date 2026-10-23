# Game Design Document — Cyber Defense

- **Version:** 1.1 (Portfolio Release)
- **Platform:** Windows Desktop (C++20 / Raylib 5.5)
- **Genre:** Tower Defense / Real-Time Tactical Strategy
- **Theme:** Hardware Architecture / Cyberpunk PCB Defense

---

## 1. Executive Summary

**Cyber Defense** is a hardware-themed tower defense strategy game that places the player in the role of a **System Administrator** defending a motherboard from escalating malware intrusions. Hostile payloads—Worms, Spyware, Trojans, and Boss rootkits—enter through peripheral data buses and travel along four copper PCB traces toward the **Central Processing Unit (CPU)**. Players earn **Bitcoin (`BC`)** by neutralizing threats and strategically deploy and upgrade four specialized hardware security modules across **18 component sockets** to preserve CPU integrity.

---

## 2. Core Gameplay Loop & Controls

### Core Loop
1. **Monitor Threat Vectors:** Observe incoming malware spawns across four independent PCB copper traces.
2. **Deploy Security Modules:** Spend `BC` to mount defensive towers onto unoccupied PCB component sockets (`18` total).
3. **Neutralize & Harvest:** Destroy malware before it reaches the CPU to earn `BC` rewards while protecting system integrity (`100%` starting health).
4. **Upgrade & Adapt:** Invest `BC` into **6-tier tower upgrades** unlocking multi-shot bursts, chain ricochets, micro-stuns, armor penetration, and persistent data-corruption fields.

### Controls
| Input | Action |
| :--- | :--- |
| **Left Mouse Click** | Select tower cards, place towers on sockets, or inspect/upgrade an existing tower |
| **Right Mouse Click** | Sell an existing tower on a socket (`+75 BC` refund) |
| **`1` / `2` / `3` / `4`** | Quick-select **Firewall**, **Logic Bomb**, **Defragmenter**, or **Data Spike** |
| **`SPACE`** | Trigger the next wave immediately (bypasses the 10s auto-start countdown) |
| **`T`** | Cycle simulation time scale (`1x` -> `2x` -> `3x` -> `4x`) |
| **`F`** | Toggle fullscreen mode |
| **`ESC`** | Pause/resume gameplay or return to the Main Menu |

---

## 3. Game Elements

### 3.1 Security Modules (Towers)

All four tower archetypes support **Levels 1–6** (`upgradeCost = baseCost * currentLevel`) with milestone-driven mechanical upgrades:

| Module | Base Cost | Base Damage | Base Range | Fire Rate | Tactical Role & Upgrade Milestones |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Firewall** | `100 BC` | `10` | `200 px` | `1.0 /s` | Rapid single-target defense. **Lv2:** Twin parallel shots; **Lv3:** `+10` dmg & `+50` range; **Lv4:** `1x` ricochet; **Lv5:** `+0.8 /s` fire rate; **Lv6:** Rapid burst volley. |
| **Logic Bomb** | `250 BC` | `40` | `180 px` | `0.4 /s` | Heavy ordinance & area denial. **Lv2:** Simultaneously targets closest & furthest enemies in range; **Lv3:** `+30` dmg & `+60` range; **Lv4:** Spawns a `3.5s` residual **Data Corruption** DoT zone (`90 px` radius, `30%` DPS); **Lv5:** `+0.3 /s` fire rate. |
| **Defragmenter** | `150 BC` | `3.5` | `140 px` | `2.0 /s` | Crowd-control cryocooler. Applies `50%` movement slow (`1.5s`). **Lv2:** `+60` range & `+0.8 /s` fire rate; **Lv3:** `1x` chain ricochet; **Lv4:** `0.2s` micro-stun; **Lv5:** `3.5s` slow duration; **Lv6:** Triple fan spread. |
| **Data Spike** | `300 BC` | `55` | `300 px` | `0.3 /s` | Long-range railgun sniper. High-velocity piercing beam (`800 px/s`). **Lv1:** Pierces `2` targets; **Lv2:** Pierces `3` targets with wider beam (`15 px`); **Lv3:** `+150` range; **Lv4:** **Armor Pierce** (`+30%` vulnerability debuff for `3s`); **Lv5:** `+0.2 /s` fire rate; **Lv6:** Twin overpenetration beams. |

### 3.2 Malware Payloads (Enemies)

| Malware Type | Base HP | Speed | CPU Damage | Reward | Behavior |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Worm** | `20` | `150 px/s` | `5` | `10 BC` | Fast, low-health swarm unit introduced from Wave 1. |
| **Spyware** | `60` | `100 px/s` | `15` | `25 BC` | Balanced mid-tier infiltrator introduced from Wave 3. |
| **Trojan** | `250` | `50 px/s` | `40` | `100 BC` | Heavily armored payload carrier introduced from Wave 5. |
| **Boss Rootkit** | `1500+` | `40 px/s` | `50` | `500 BC` | High-threat siege unit deployed every 10th wave (`Wave 10, 20, ...`). |

---

## 4. Technical Architecture & Mathematical Models

### Core Subsystems
- **`EntityManager` (`src/entityManager.h`):** Owns active `Enemy`, `Tower`, `Projectile`, and `DamageZone` collections with deterministic cleanup (`clear()`), non-copyable RAII semantics, and pre-destruction observer notification (`Projectile::onEnemyRemoved`) so in-flight projectiles never retain dangling enemy pointers.
- **Swept Segment-Circle Continuous Collision (`src/projectile.h`):** High-speed piercing beams (`800 px/s`, up to `3200 px/s` at `4x` simulation speed) project the frame displacement segment $\overline{P_{t-1}P_t}$ onto each enemy's bounding circle (`SegmentIntersectsCircle`) to eliminate frame-skipping tunneling artifacts.
- **Resolution-Independent Virtual Coordinate Scaling (`src/constants.h`):** All gameplay simulation, socket hitboxes, and PCB waypoint traces operate in a canonical `1600x900` coordinate space and scale dynamically to `1280x720`, `1600x900`, `1920x1080`, or fullscreen via `GetScaleX()` and `GetScaleY()`.
- **Adaptive Wave Music System (`src/music_manager.h`):** Streams synthesized, seamless-looping retro-synth `.wav` tracks that transition automatically across **Early** (`Waves 1–5`), **Mid** (`Waves 6–10`), and **Late** (`Wave 11+`) combat phases, with full `IsAudioDeviceReady()` and `IsMusicValid()` safety guards.

### Mathematical Formulas
- **Tiered Wave HP Scaling:**
  $$M(w) = 1.0 + 0.10 \cdot \left\lfloor \frac{w - 1}{5} \right\rfloor + 0.25 \cdot \left\lfloor \frac{w - 1}{10} \right\rfloor$$
  *(Boss units apply an additional $\left(1.0 + 0.50 \cdot \lfloor (w-1)/10 \rfloor\right)$ siege multiplier.)*
- **Continuous Segment-Circle Projection:**
  $$t = \operatorname{clamp}\!\left(-\frac{(P_0 - C) \cdot (P_1 - P_0)}{\lVert P_1 - P_0 \rVert^2},\; 0,\; 1\right), \quad \lVert P_0 + t(P_1 - P_0) - C \rVert \le R$$
- **Residual Area-of-Effect DoT:**
  $$\Delta \text{HP} = \text{DPS} \cdot \Delta t_{\text{scaled}}$$

---

## 5. Visual & Audio Direction

- **Visuals:** High-contrast cyberpunk HUD panels, animated 24 FPS sprite sequences for Worm, Spyware, and Trojan units, 6 distinct visual evolution sprites per tower class (`24` tower sprites total), pulsing spawn-lane warnings, and translucent range/ghost placement overlays.
- **Audio:** Original procedurally synthesized 16-bit PCM retro-synth soundtrack (`menu_theme.wav`, `early_wave.wav`, `mid_wave.wav`, `late_wave.wav`) crafted for zero-latency looping and 100% royalty-free distribution.
