
#include "raylib.h"

#define MAX_ENEMIES 40

// Create enemy class
typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 accel;
    float speed;
    bool alive;
} Enemy;

// Create player
typedef struct {
    Vector2 pos;
    Vector2 vel; // Player direction, no current use implemented yet
    float speed;
    bool alive;
    float upper_y; // How far player can move up the screen
} Player;

void PlayerMove(Player *player, float dt, int screenwidth, int screenheight);
void EnemiesMove(Enemy enemies[], float dt, int screenwidth, int screenheight);
void LoadLevel(int level, Enemy enemies[], int *enemyCount, int screenwidth, int screenheight);

int main(void) {
    InitWindow(800, 600, "WASD to move");
    SetTargetFPS(60);

    int screenwidth = GetScreenWidth();
    int screenheight = GetScreenHeight();

    // Initialize player's attributes
    Player player;
    player.pos = (Vector2) {screenwidth/2, screenheight*0.8f};
    player.speed = 200.0f;
    player.alive = 1;
    player.upper_y = screenheight * 0.7f;

    // Store enemies into array
    Enemy enemies[MAX_ENEMIES];
    int enemyCount = 0;
    int currentLevel = 1;
    LoadLevel(currentLevel, enemies, &enemyCount, screenwidth, screenheight);
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        PlayerMove(&player, dt, screenwidth, screenheight);
        EnemiesMove(enemies, dt, screenwidth, screenheight);

        if (IsKeyDown(KEY_SPACE)) {
            DrawText("TESTING", 30, 30, 50, BLUE);
            DrawText("100 100 HERE", 100, 100, 50, RED);
        }

        // TODO: Check if level is over, and if so, currentLevel++, LoadLevel()

        BeginDrawing();
        ClearBackground(DARKGREEN);
        // Draw player
        DrawCircleV(player.pos, 20, RED);
        // Draw all enemies
        for (int i=0; i<enemyCount; i++) {
            DrawCircleV(enemies[i].pos, 10, BLUE);
        }
        DrawText("WASD to move", 10, 10, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}


void PlayerMove(Player *player, float dt, int screenwidth, int screenheight) {

    if (IsKeyDown(KEY_W)) player->pos.y -= player->speed * dt;
    if (IsKeyDown(KEY_S)) player->pos.y += player->speed * dt;
    if (IsKeyDown(KEY_A)) player->pos.x -= player->speed * dt;
    if (IsKeyDown(KEY_D)) player->pos.x += player->speed * dt;

    // Add movement bounds
    if (player->pos.x > screenwidth) {
        player->pos.x = screenwidth;
    }
    else if (player->pos.x < 0) {
        player->pos.x = 0;   
    }

    if (player->pos.y < player->upper_y) {
        player->pos.y = player->upper_y;
    }
    else if (player->pos.y > screenheight) {
        player->pos.y = screenheight;
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

void LoadLevel(int level, Enemy enemies[], int *enemyCount, int screenwidth, int screenheight) {
    // Vary enemy count based on level
    switch (level) {
        case 1:
            *enemyCount = 10;
            break;
        case 2:
            *enemyCount = 20;
            break;
        case 3:
            *enemyCount = 30;
            break;
        case 4:
            *enemyCount = 40;
    }

    // Randomize enemy stats
    for (int i=0; i<*enemyCount; i++) {
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
}