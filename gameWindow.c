// gameWindow.c – Big window, not fullscreen
#include "raylib.h"

int main(void)
{
    InitWindow(1600, 900, "My Game – Big Window");  // 1600×900
    SetTargetFPS(60);

    while (!WindowShouldClose())    // ESC or Alt+F4 to quit
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("BIG WINDOW – press ESC to quit", 400, 400, 40, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}