// test.c - PLAYER VISIBLE 100% GUARANTEED
#include "raylib.h"

int main(void) {
    InitWindow(800, 600, "ONE SHOT - PLAYER TEST");
    SetTargetFPS(60);

    Vector2 player = {400, 300};  // DEAD CENTER

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // HUGE PLAYER - IMPOSSIBLE TO MISS
        DrawTriangle((Vector2){player.x, player.y-100},
                     (Vector2){player.x-80, player.y+100},
                     (Vector2){player.x+80, player.y+100}, WHITE);
        DrawCircle(player.x, player.y, 60, YELLOW);
        DrawText("PLAYER HERE", player.x-90, player.y-20, 40, RED);

        DrawText("IF YOU SEE THIS SHIP, IT WORKS", 100, 500, 30, LIME);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}