#include "raylib.h"
#include <math.h>

#define MAX_ENEMIES   10
#define MAX_BULLETS   50
#define MAX_SHRAPNEL  64

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

int main(void)
{
    InitWindow(800, 600, "WASD move | 1-2-3 guns | E fire");
    SetTargetFPS(60);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Vector2 playerPos = { sw/2, sh*0.8f };
    float   playerSpeed = 200.0f;

    GunType gun = BASIC;

    Bullet   bullets[MAX_BULLETS]   = {0};
    Shrapnel shrapnel[MAX_SHRAPNEL] = {0};
    int      bulletCount   = 0;
    int      shrapnelCount = 0;

    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].pos   = (Vector2){ GetRandomValue(50, sw-50), GetRandomValue(50, 350) };
        enemies[i].vel   = (Vector2){ GetRandomValue(0,1) ? 1.0f : -1.0f, 0 };
        enemies[i].speed = GetRandomValue(80, 200);
        enemies[i].alive = true;
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ----- INPUT -----
        if (IsKeyPressed(KEY_ONE))  gun = BASIC;
        if (IsKeyPressed(KEY_TWO))  gun = GRENADE;
        if (IsKeyPressed(KEY_THREE))gun = LASER;

        if (IsKeyPressed(KEY_E) && bulletCount < MAX_BULLETS-1)
        {
            // FIXED: recycle slots - find inactive bullet
            int slot = -1;
            for (int j=0; j<MAX_BULLETS; j++) {
                if (!bullets[j].active) { slot = j; break; }
            }
            if (slot == -1) slot = bulletCount++; // use next if full
            
            Bullet *b = &bullets[slot];
            b->pos    = (Vector2){ playerPos.x, playerPos.y - 20 };
            b->active = true;
            b->timer  = 0.0f;

            if (gun == BASIC) {
                b->type = 0;
                b->vel  = (Vector2){ 0, -600 };
            }
            else if (gun == GRENADE) {
                b->type = 1;
                float a = GetRandomValue(-25,25) * DEG2RAD;
                b->vel  = (Vector2){ sinf(a)*200, -cosf(a)*700 }; // FIXED: higher arc north
            }
            else {
                b->type = 2;
                b->vel  = (Vector2){ 0, 0 };
            }
        }

        // ----- PLAYER MOVE -----
        if (IsKeyDown(KEY_W)) playerPos.y -= playerSpeed * dt;
        if (IsKeyDown(KEY_S)) playerPos.y += playerSpeed * dt;
        if (IsKeyDown(KEY_A)) playerPos.x -= playerSpeed * dt;
        if (IsKeyDown(KEY_D)) playerPos.x += playerSpeed * dt;

        // ----- UPDATE BULLETS -----
        for (int i = 0; i < MAX_BULLETS; i++) // FIXED: loop MAX, not bulletCount
        {
            Bullet *b = &bullets[i];
            if (!b->active) continue;
            b->timer += dt;

            if (b->type == 0) {
                b->pos.y += b->vel.y * dt;
                if (b->pos.y < -50) b->active = false;
            }
            else if (b->type == 1) {
                b->vel.y += 1200 * dt;
                b->pos.x += b->vel.x * dt;
                b->pos.y += b->vel.y * dt;

                if (b->pos.y >= 280 && b->timer < 0.4f) { // FIXED: land NORTH at 280px
                    b->timer = 0.4f;
                }
                if (b->timer >= 0.9f) { // explode after delay
                    for (int s = 0; s < 8; s++) {
                        if (shrapnelCount >= MAX_SHRAPNEL-1) break;
                        int ss = -1;
                        for (int j=0; j<MAX_SHRAPNEL; j++) {
                            if (!shrapnel[j].active) { ss = j; break; }
                        }
                        if (ss != -1) {
                            float a = s * (PI/4);
                            shrapnel[ss] = (Shrapnel){
                                .pos    = b->pos,
                                .vel    = { sinf(a)*300, cosf(a)*300 },
                                .life   = 0.3f,
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

        // ----- UPDATE SHRAPNEL -----
        for (int i = 0; i < MAX_SHRAPNEL; i++) // FIXED: loop MAX
        {
            if (!shrapnel[i].active) continue;
            shrapnel[i].life -= dt;
            shrapnel[i].pos.x += shrapnel[i].vel.x * dt;
            shrapnel[i].pos.y += shrapnel[i].vel.y * dt;
            if (shrapnel[i].life <= 0) shrapnel[i].active = false;
        }

        // ----- ENEMIES -----
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (!enemies[i].alive) continue;
            enemies[i].pos.x += enemies[i].vel.x * enemies[i].speed * dt;
            if (enemies[i].pos.x > sw) { enemies[i].pos.x = sw; enemies[i].vel.x = -1; }
            if (enemies[i].pos.x < 0)  { enemies[i].pos.x = 0;  enemies[i].vel.x =  1; }
        }

        // ----- DRAW -----
        BeginDrawing();
        ClearBackground(DARKGREEN);

        DrawCircleV(playerPos, 20, RED);

        for (int i = 0; i < MAX_BULLETS; i++) {
            Bullet *b = &bullets[i];
            if (!b->active) continue;
            if (b->type == 0) DrawCircleV(b->pos, 4, YELLOW);
            if (b->type == 1) DrawCircleV(b->pos, 8, ORANGE);
            if (b->type == 2) {
                float w = 20 + b->timer * 40;
                DrawRectangle(playerPos.x - w/2, 0, w, playerPos.y-20, Fade(VIOLET, 0.75f));
            }
        }

        for (int i = 0; i < MAX_SHRAPNEL; i++)
            if (shrapnel[i].active) DrawCircleV(shrapnel[i].pos, 6, WHITE);

        for (int i = 0; i < MAX_ENEMIES; i++)
            if (enemies[i].alive) DrawCircleV(enemies[i].pos, 10, BLUE);

        const char *names[] = {"BASIC", "GRENADE", "MEGA LASER"};
        DrawText(TextFormat("WEAPON: %s", names[gun]), 10, 10, 20, WHITE);
        DrawText("1-2-3 switch | E fire", 10, 40, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}