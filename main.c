// walker.c - top-down walker
#include "raylib.h"

int main(void) {
    InitWindow(800, 600, "Top-down walker - WASD");
    SetTargetFPS(60);

    Vector2 pos = {400, 300};
    float speed = 200.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_W)) pos.y -= speed * dt;
        if (IsKeyDown(KEY_S)) pos.y += speed * dt;
        if (IsKeyDown(KEY_A)) pos.x -= speed * dt;
        if (IsKeyDown(KEY_D)) pos.x += speed * dt;

        if (IsKeyDown(KEY_SPACE)) {
            DrawText("TESTING", 30, 30, 50, BLUE);
            DrawText("100 100 HERE", 100, 100, 50, RED);
        
        }

        BeginDrawing();
        ClearBackground(DARKGREEN);
        DrawCircleV(pos, 20, RED);
        DrawText("WASD to move", 10, 10, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}