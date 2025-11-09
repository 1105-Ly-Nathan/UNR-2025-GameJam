# ONE SHOT, ONE KILL
**A retro-style, high-intensity arcade shooter built in C with Raylib**

**By Matthew Johnson & Nathan Ly**  
Game Jam 2025

---

## PLAY IT NOW

```bash
gcc -o oneshotv1 oneshotv1.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 && ./oneshotv1

CONTROLS

WASD - Move 
1-4 - Select weapon
E-Fire / Hold for Laser
M - Return to menu
0-Toggle DEV MODE (999 everything)

WEAPONS

BASIC – Pew pew. 1 ammo per shot.
NADES – 1 Explodes. Devastating.
YUGE LASER – Melts everything.
SHIELD –  Press 4. Blocks all damage for 15 seconds.


LEVELS

Level 1 – 10 small enemies. Learn the ropes.
Level 2 – 20 small + 1 big (15 HP). Chaos begins.
Level 3 – 20 small + 3 big + DADDY (300 HP). Final boss. Good luck.


FEATURES

Smooth enemy movement with realistic acceleration
Dynamic shrinking enemies
Grenade explosions 
Boss with erratic 2D dodging and fire
Shield with visual feedback
Spinning indicator so you know it's not frozen
3-second countdown before each level
Full dev mode (press 0)
Clean shop with backdrop
Credits screen on victory


DEV MODE (Press 0)

999 gold & ammo
All weapons unlocked
All levels unlocked
Projectiles bounce off you
Toggle on/off anytime — your progress is saved and restored!


MADE WITH

C + Raylib
Pure passion
Zero sleep
Maximum fun


"One shot. One kill. No luck. Just skill."
— You, probably