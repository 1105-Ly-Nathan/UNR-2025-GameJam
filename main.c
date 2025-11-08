
#include "raylib.h"

#define MAX_ENEMIES 10

// Create enemy class
typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 accel;
    float speed;
    bool alive;
} Enemy;

void AutoMove(Vector2 *pos, Vector2 *vel, float speed, float dt, int screenwidth, int screenheight);
void EnemiesMove(Enemy enemies[], float dt, int screenwidth, int screenheight);

int main(void) {
    InitWindow(800, 600, "WASD to move");
    SetTargetFPS(60);

    int screenwidth = GetScreenWidth();
    int screenheight = GetScreenHeight();

    // Controls character position (float x, float y)
    Vector2 pos = {screenwidth/2, screenheight*0.8};

    // Controls character direction, where {-1.0f, 0.0f} points left
    Vector2 vel = {1.0f, 0.0f};

    // Speed of character
    float speed = 200.0f;

    // Store enemies into array
    Enemy enemies[MAX_ENEMIES];

    // Randomize enemy stats
    for (int i=0; i<MAX_ENEMIES; i++) {
        enemies[i].pos = (Vector2) {GetRandomValue(0, screenwidth), GetRandomValue(0, screenheight*0.7f)};

        // Cant randomize velocity direction between -1 and 1 (could get 0). Instead, convert 2 outcomes
        float dirx;
        if (GetRandomValue(0, 1) == 0) {
            dirx = -1;
        }
        else {
            dirx = 1;
        }
        enemies[i].vel = (Vector2) {dirx, 0};
        enemies[i].speed = GetRandomValue(100, 300);
    }




    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_W)) pos.y -= speed * dt;
        if (IsKeyDown(KEY_S)) pos.y += speed * dt;
        if (IsKeyDown(KEY_A)) pos.x -= speed * dt;
        if (IsKeyDown(KEY_D)) pos.x += speed * dt;

        // Auto movement
        AutoMove(&pos, &vel, speed, dt, screenwidth, screenheight);
        EnemiesMove(enemies, dt, screenwidth, screenheight);

        if (IsKeyDown(KEY_SPACE)) {
            DrawText("TESTING", 30, 30, 50, BLUE);
            DrawText("100 100 HERE", 100, 100, 50, RED);
        }


        BeginDrawing();
        ClearBackground(DARKGREEN);
        DrawCircleV(pos, 20, RED);
        // Draw all enemies
        for (int i=0; i<MAX_ENEMIES; i++) {
            DrawCircleV(enemies[i].pos, 10, BLUE);
        }
        DrawText("WASD to move", 10, 10, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

void AutoMove(Vector2 *pos, Vector2 *vel, float speed, float dt, int screenwidth, int screenheight) {
    // We have position's address, or pointer, as input. Get the pos struct, then access x member with (*pos).x
    // Or alternatively, shorthand pos->x   Leaving here for learning purposes.
    // Move the character right, then bounce off the wall and back
    (*pos).x += vel->x * speed * dt;

    // Bounce off Right wall
    if (pos->x > screenwidth) {
        pos->x = screenwidth;
        vel->x = -1.0;
    }
    // Bounce off left
    else if (pos->x < 0) {
        pos ->x = 0;
        vel->x = 1.0;
    }
}

void EnemiesMove(Enemy enemies[], float dt, int screenwidth, int screenheight) {
    for (int i=0; i<MAX_ENEMIES; i++) {
        enemies[i].pos.x += enemies[i].vel.x * enemies[i].speed * dt;
        // Bounce back 
        if (enemies[i].pos.x > screenwidth) {
            enemies[i].pos.x = screenwidth;
            enemies[i].vel.x = -1.0;
        }
        else if (enemies[i].pos.x < 0) {
            enemies[i].pos.x = 0;
            enemies[i].vel.x = 1.0;
        }
    }
}