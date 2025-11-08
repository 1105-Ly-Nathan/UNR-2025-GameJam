// test.c - FIXED & CLEAN (no Clamp/Vector2Add needed)
//gcc test.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o test
#include "raylib.h"

#define MAX_ENEMIES 30
#define ENEMY_SPEED 60

typedef struct { Vector2 pos; bool active; } Enemy;
typedef struct { Vector2 pos; Vector2 vel; bool active; float radius; } Bullet;

int main(void) {
    InitWindow(800, 600, "test.c - SPACE = one exploding shot");
    SetTargetFPS(60);

    Vector2 player = {400, 500};
    float pspeed = 300;

    Enemy enemies[MAX_ENEMIES] = {0};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].pos = (Vector2){100 + (i%10)*65, 60 + (i/10)*80};
        enemies[i].active = true;
    }

    Bullet bullet = {0};
    int score = 0;
    bool canFire = true;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Player left/right
        if (IsKeyDown(KEY_A)) player.x -= pspeed * dt;
        if (IsKeyDown(KEY_D)) player.x += pspeed * dt;
        if (player.x < 30) player.x = 30;
        if (player.x > 770) player.x = 770;

        // Fire once
        if (IsKeyPressed(KEY_SPACE) && canFire) {
            bullet.pos = player;
            bullet.vel = (Vector2){0, -700};
            bullet.active = true;
            bullet.radius = 10;
            canFire = false;
        }

        // Bullet update
        if (bullet.active) {
            bullet.pos.x += bullet.vel.x * dt;
            bullet.pos.y += bullet.vel.y * dt;
            bullet.radius += 200 * dt;
            if (bullet.pos.y < -200) {
                bullet.active = false;
                canFire = true;
            }
        }

        // Enemies move down
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].pos.y += ENEMY_SPEED * dt;
                if (enemies[i].pos.y > 650) enemies[i].active = false;
            }
        }

        // Collision
        if (bullet.active) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active &&
                    CheckCollisionCircleRec(bullet.pos, bullet.radius,
                    (Rectangle){enemies[i].pos.x-15, enemies[i].pos.y-15, 30, 30})) {
                    enemies[i].active = false;
                    score++;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Player ship
        DrawTriangle((Vector2){player.x, player.y-25},
                     (Vector2){player.x-20, player.y+15},
                     (Vector2){player.x+20, player.y+15}, SKYBLUE);

        // Enemies
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (enemies[i].active)
                DrawRectangle(enemies[i].pos.x-15, enemies[i].pos.y-15, 30, 30, RED);

        // Bullet explosion
        if (bullet.active)
            DrawCircleV(bullet.pos, bullet.radius, YELLOW);

        DrawText(TextFormat("SCORE: %d", score), 10, 10, 30, WHITE);
        DrawText("A/D move  SPACE = fire once", 10, 560, 20, LIME);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}