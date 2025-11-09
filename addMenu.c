// addMenu.c
#include "raylib.h"
#include <math.h>

#define MAX_ENEMIES   10
#define MAX_BULLETS   50
#define MAX_SHRAPNEL  64
#define MAX_EXPLOSIONS 20

typedef enum { MENU, PLAYING, QUIT } GameState;
typedef enum { BASIC, GRENADE, LASER } GunType;

typedef struct {
    Vector2 pos, vel;
    float   timer;
    int     type;
    bool    active;
} Bullet;

typedef struct {
    Vector2 pos, vel;
    float   life;
    bool    active;
} Shrapnel;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float   speed;
    bool    alive;
} Enemy;

typedef struct {
    Vector2 pos;
    float   timer;
    bool    active;
} Explosion;

int main(void)
{
    InitWindow(800, 600, "One shot, One Kill. One Kill, One Shot.");
    SetTargetFPS(60);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    GameState state = MENU;
    int menuSelection = 0; // 0=Play, 1=Shop, 2=Quit

    // Persistent game stats
    int gold = 0;
    int ammo = 1;

    // Level data
    int currentLevel = 1;
    int enemiesToSpawn = 2;

    Vector2 playerPos = { sw/2, sh*0.8f };
    float playerSpeed = 200.0f;
    GunType gun = BASIC;

    Bullet   bullets[MAX_BULLETS]   = {0};
    Shrapnel shrapnel[MAX_SHRAPNEL] = {0};
    Explosion explosions[MAX_EXPLOSIONS] = {0};

    Enemy enemies[MAX_ENEMIES];

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (state == MENU)
        {
            if (IsKeyPressed(KEY_DOWN)) menuSelection = (menuSelection + 1) % 3;
            if (IsKeyPressed(KEY_UP))   menuSelection = (menuSelection + 2) % 3;

            if (IsKeyPressed(KEY_ENTER))
            {
                if (menuSelection == 0) // PLAY
                {
                    state = PLAYING;
                    currentLevel = 1;
                    enemiesToSpawn = 2;
                    gold = 0;
                    ammo = 1;

                    // Clear everything
                    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
                    for (int i = 0; i < MAX_SHRAPNEL; i++) shrapnel[i].active = false;
                    for (int i = 0; i < MAX_EXPLOSIONS; i++) explosions[i].active = false;

                    // Spawn enemies
                    for (int i = 0; i < enemiesToSpawn && i < MAX_ENEMIES; i++) {
                        enemies[i].pos   = (Vector2){ GetRandomValue(100, sw-100), GetRandomValue(50, 300) };
                        enemies[i].vel   = (Vector2){ GetRandomValue(0,1) ? 1.0f : -1.0f, 0 };
                        enemies[i].speed = GetRandomValue(80, 150);
                        enemies[i].alive = true;
                    }
                }
                else if (menuSelection == 2) // QUIT
                {
                    state = QUIT;
                }
            }
        }
        else if (state == PLAYING)
        {
            // ESC = back to menu (keeps gold/ammo)
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = MENU;
                continue;
            }

            // Weapon switch
            if (IsKeyPressed(KEY_ONE)) gun = BASIC;
            if (IsKeyPressed(KEY_TWO)) gun = GRENADE;
            if (IsKeyPressed(KEY_THREE)) gun = LASER;
            if (IsKeyPressed(KEY_Q)) { state = MENU; continue; }
            // Shoot
            if (IsKeyPressed(KEY_E) && ammo > 0)
            {
                ammo--;
                int slot = -1;
                for (int j = 0; j < MAX_BULLETS; j++) if (!bullets[j].active) { slot = j; break; }
                if (slot == -1) continue;

                Bullet *b = &bullets[slot];
                b->pos    = (Vector2){ playerPos.x, playerPos.y - 20 };
                b->active = true;
                b->timer  = 0.0f;

                if (gun == BASIC) {
                    b->type = 0;
                    b->vel  = (Vector2){ 0, -700 };
                }
                else if (gun == GRENADE) {
                    b->type = 1;
                    float a = GetRandomValue(-20,20) * DEG2RAD;
                    b->vel  = (Vector2){ sinf(a)*220, -cosf(a)*750 };
                }
                else {
                    b->type = 2;
                    b->vel  = (Vector2){ 0, 0 };
                }
            }

            // Player movement
            if (IsKeyDown(KEY_W)) playerPos.y -= playerSpeed * dt;
            if (IsKeyDown(KEY_S)) playerPos.y += playerSpeed * dt;
            if (IsKeyDown(KEY_A)) playerPos.x -= playerSpeed * dt;
            if (IsKeyDown(KEY_D)) playerPos.x += playerSpeed * dt;

            // Update bullets
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                Bullet *b = &bullets[i];
                if (!b->active) continue;
                b->timer += dt;

                if (b->type == 0) {
                    b->pos.y += b->vel.y * dt;
                    if (b->pos.y < -50) b->active = false;
                }
                else if (b->type == 1) {
                    b->vel.y += 1300 * dt;
                    b->pos.x += b->vel.x * dt;
                    b->pos.y += b->vel.y * dt;

                    if (b->pos.y >= 280 && b->timer < 0.4f) b->timer = 0.4f;
                    if (b->timer >= 0.9f) {
                        for (int s = 0; s < 8; s++) {
                            int ss = -1;
                            for (int j = 0; j < MAX_SHRAPNEL; j++) if (!shrapnel[j].active) { ss = j; break; }
                            if (ss != -1) {
                                float a = s * (PI/4);
                                shrapnel[ss] = (Shrapnel){
                                    .pos    = b->pos,
                                    .vel    = { sinf(a)*350, cosf(a)*350 },
                                    .life   = 0.25f,
                                    .active = true
                                };
                            }
                        }
                        b->active = false;
                    }
                }
                else {
                    if (b->timer > 3.0f) b->active = false;
                }
            }

            // Update shrapnel
            for (int i = 0; i < MAX_SHRAPNEL; i++)
            {
                if (!shrapnel[i].active) continue;
                shrapnel[i].life -= dt;
                shrapnel[i].pos.x += shrapnel[i].vel.x * dt;
                shrapnel[i].pos.y += shrapnel[i].vel.y * dt;
                if (shrapnel[i].life <= 0) shrapnel[i].active = false;
            }

            // Update explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++) {
                if (explosions[i].active) {
                    explosions[i].timer -= dt;
                    if (explosions[i].timer <= 0) explosions[i].active = false;
                }
            }

            // COLLISION: Bullets & Shrapnel
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active || bullets[i].type == 2) continue;
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (!enemies[e].alive) continue;
                    if (CheckCollisionCircles(bullets[i].pos, 6, enemies[e].pos, 12)) {
                        enemies[e].alive = false;
                        bullets[i].active = false;
                        gold++;
                        ammo++;

                        int slot = -1;
                        for (int j = 0; j < MAX_EXPLOSIONS; j++) if (!explosions[j].active) { slot = j; break; }
                        if (slot != -1) {
                            explosions[slot].pos = enemies[e].pos;
                            explosions[slot].timer = 0.3f;
                            explosions[slot].active = true;
                        }
                        break;
                    }
                }
            }

            for (int i = 0; i < MAX_SHRAPNEL; i++) {
                if (!shrapnel[i].active) continue;
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (!enemies[e].alive) continue;
                    if (CheckCollisionCircles(shrapnel[i].pos, 6, enemies[e].pos, 12)) {
                        enemies[e].alive = false;
                        shrapnel[i].active = false;
                        gold++;
                        ammo++;

                        int slot = -1;
                        for (int j = 0; j < MAX_EXPLOSIONS; j++) if (!explosions[j].active) { slot = j; break; }
                        if (slot != -1) {
                            explosions[slot].pos = enemies[e].pos;
                            explosions[slot].timer = 0.3f;
                            explosions[slot].active = true;
                        }
                        break;
                    }
                }
            }

            // LASER COLLISION
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].type != 2 || !bullets[i].active) continue;
                float w = 20 + bullets[i].timer * 40;
                Rectangle laser = { playerPos.x - w/2, 0, w, playerPos.y - 20 };
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (!enemies[e].alive) continue;
                    if (CheckCollisionCircleRec(enemies[e].pos, 12, laser)) {
                        enemies[e].alive = false;
                        gold++;
                        ammo++;

                        int slot = -1;
                        for (int j = 0; j < MAX_EXPLOSIONS; j++) if (!explosions[j].active) { slot = j; break; }
                        if (slot != -1) {
                            explosions[slot].pos = enemies[e].pos;
                            explosions[slot].timer = 0.3f;
                            explosions[slot].active = true;
                        }
                    }
                }
            }

            // Enemy movement
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].alive) continue;
                enemies[i].pos.x += enemies[i].vel.x * enemies[i].speed * dt;
                if (enemies[i].pos.x > sw) { enemies[i].pos.x = sw; enemies[i].vel.x = -1; }
                if (enemies[i].pos.x < 0)  { enemies[i].pos.x = 0;  enemies[i].vel.x =  1; }
            }

            // Check win/lose
            bool allDead = true;
            for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].alive) allDead = false;
            if (allDead && enemiesToSpawn > 0) {
                DrawText("LEVEL COMPLETE!", sw/2 - 180, sh/2 - 50, 40, YELLOW);
                if (IsKeyPressed(KEY_ENTER)) state = MENU;
            }
            if (ammo <= 0 && !allDead) {
                DrawText("OUT OF AMMO!", sw/2 - 140, sh/2, 40, RED);
                if (IsKeyPressed(KEY_ENTER)) state = MENU;
            }
        }

        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);

        if (state == MENU)
        {
            DrawText("One shot, One Kill.", 800/2 - MeasureText("One shot, One Kill.", 40)/2, 150, 40, WHITE);
            DrawText("One Kill, One Shot.", 800/2 - MeasureText("One Kill, One Shot.", 40)/2, 200, 40, WHITE);

            const char *options[] = {"Play", "Shop", "Quit"};
            for (int i = 0; i < 3; i++) {
                Color c = (i == menuSelection) ? YELLOW : DARKGRAY;
                DrawText(options[i], 800/2 - MeasureText(options[i], 30)/2, 300 + i*60, 30, c);
            }
            DrawText("Use UP/DOWN + ENTER", 800/2 - 140, 500, 20, GRAY);
        }
        else if (state == PLAYING)
        {
            // Player (sky blue + shoulders + gun)
            DrawCircleV(playerPos, 20, SKYBLUE);
            DrawCircle((int)playerPos.x - 18, (int)playerPos.y - 8, 10, SKYBLUE); // left shoulder
            DrawCircle((int)playerPos.x + 18, (int)playerPos.y - 8, 10, SKYBLUE); // right shoulder
            DrawRectangle(playerPos.x - 8, playerPos.y - 30, 16, 20, SKYBLUE);     // gun

            // Bullets
            for (int i = 0; i < MAX_BULLETS; i++) {
                Bullet *b = &bullets[i];
                if (!b->active) continue;
                if (b->type == 0) DrawCircleV(b->pos, 5, RED);
                if (b->type == 1) DrawCircleV(b->pos, 9, RED);
                if (b->type == 2) {
                    float w = 20 + b->timer * 40;
                    DrawRectangle(playerPos.x - w/2, 0, w, playerPos.y-20, Fade(RED, 0.6f));
                }
            }

            // Shrapnel
            for (int i = 0; i < MAX_SHRAPNEL; i++)
                if (shrapnel[i].active) DrawCircleV(shrapnel[i].pos, 5, RED);

            // Explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++) {
                if (explosions[i].active) {
                    float s = 1.0f + (1.0f - explosions[i].timer/0.3f) * 3.0f;
                    DrawCircleV(explosions[i].pos, 15 * s, Fade(YELLOW, explosions[i].timer/0.3f));
                }
            }

            // Enemies (white)
            for (int i = 0; i < MAX_ENEMIES; i++)
                if (enemies[i].alive) DrawCircleV(enemies[i].pos, 12, WHITE);

            // HUD
            DrawText(TextFormat("LEVEL %d", currentLevel), 10, 10, 20, WHITE);
            DrawText(TextFormat("GOLD: %d", gold), 10, 40, 25, YELLOW);
            DrawText(TextFormat("AMMO: %d", ammo), 10, 70, 25, ammo > 0 ? GREEN : RED);
            DrawText("ESC = Menu", 10, sh - 30, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}