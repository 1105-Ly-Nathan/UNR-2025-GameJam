// Authors: Matthew Johnson and Nathan Ly 
// GameJam
// gcc -o oneshotv1 oneshotv1.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 && ./oneshotv1

#include "raylib.h"
#include <math.h>
#include <string.h>

#define MAX_ENEMIES     100
#define MAX_BULLETS     600
#define MAX_EXPLOSIONS  40

typedef enum { MENU, LEVELS, PLAY, SHOP, SUCCESS, FAIL, WIN, CREDITS } Screen;
typedef enum { BASIC, GRENADE, LASER, SHIELD } Weapon;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float timer;
    int type;
    bool active;
    bool player;
} Bullet;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 targetVel;
    float speed;
    bool alive;
    int health;
    int maxHealth;
    bool big;
    bool boss;
    float size;
    float baseSize;
    Color color;
    float shootTimer;
    float changeTimer;
    int burstCount;
    bool bigRocket;
    float shakeTimer;
    Vector2 shakeOffset;
} Enemy;

typedef struct {
    Vector2 pos;
    float timer;
    bool active;
} Explosion;

typedef struct {
    bool active;
    float duration;
    float alpha;
} Shield;

typedef struct {
    int gold;
    int ammo;
    bool hasGrenade;
    bool hasLaser;
    bool hasShield;
    bool level2;
    bool level3;
} SavedState;

// GLOBALS
int w, h, fenceY, barY;
Screen screen = MENU;
int menuSel = 0, levelSel = 0;
int gold = 0, ammo = 1, level, alive, bigAlive;
bool devMode = false, hasGrenade = false, hasLaser = false, hasShield = false, level2 = false, level3 = false;
Vector2 player;
Weapon weapon = BASIC;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
Explosion explosions[MAX_EXPLOSIONS];
Shield shield;
SavedState saved;
float spinAngle = 0, countdown = 0, screenTimer = 0;
float playerShakeTimer = 0;
Vector2 playerShakeOffset = {0,0};

void InitGame(void);
void UpdateGame(float dt);
void DrawGame(void);
void SpawnLevel(int lvl);
void FireWeapon(void);
void UpdateEnemies(float dt);
void UpdateBullets(float dt);
void HandleCollisions(float dt);
void DrawPlayer(void);
void DrawShield(void);
void DrawHUD(void);
void DrawShop(void);
void DrawControlsOverlay(void);

int main(void)
{
    InitWindow(1200, 800, "ONE SHOT, ONE KILL");
    SetTargetFPS(60);
    InitGame();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        spinAngle += 180 * dt;

        if (IsKeyPressed(KEY_ZERO))
        {
            devMode = !devMode;
            if (devMode)
            {
                saved.gold = gold; saved.ammo = ammo;
                saved.hasGrenade = hasGrenade; saved.hasLaser = hasLaser; saved.hasShield = hasShield;
                saved.level2 = level2; saved.level3 = level3;
                gold = 5000; ammo = 500;
                hasGrenade = hasLaser = hasShield = true;
                level2 = level3 = true;
            }
            else
            {
                gold = saved.gold; ammo = saved.ammo;
                hasGrenade = saved.hasGrenade; hasLaser = saved.hasLaser; hasShield = saved.hasShield;
                level2 = saved.level2; level3 = saved.level3;
            }
        }

        if (IsKeyPressed(KEY_M) && screen != MENU) { screen = MENU; countdown = 0; screenTimer = 0; continue; }

        bool up = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
        bool down = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
        bool enter = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

        if (screen == MENU)
        {
            if (up) menuSel = (menuSel + 2) % 3;
            if (down) menuSel = (menuSel + 1) % 3;
            if (enter)
            {
                if (menuSel == 0) screen = LEVELS;
                if (menuSel == 1) screen = SHOP;
                if (menuSel == 2) break;
            }
        }
        else if (screen == LEVELS)
        {
            if (up) levelSel = (levelSel + 2) % 3;
            if (down) levelSel = (levelSel + 1) % 3;
            if (enter && (levelSel == 0 || (levelSel == 1 && level2) || (levelSel == 2 && level3)))
            {
                screen = PLAY;
                level = levelSel + 1;
                ammo = devMode ? 500 : 1;
                countdown = 3.0f;
                screenTimer = 0;
                SpawnLevel(level);
            }
        }
        else if (screen == SHOP)
        {
            if (IsKeyPressed(KEY_ONE) && gold >= 4 && !hasGrenade) { gold -= 4; hasGrenade = true; }
            if (IsKeyPressed(KEY_TWO) && gold >= 8 && !hasLaser) { gold -= 8; hasLaser = true; }
            if (IsKeyPressed(KEY_THREE) && gold >= 12 && !hasShield) { gold -= 12; hasShield = true; }
        }
        else if (screen == PLAY)
        {
            if (countdown > 0) { countdown -= dt; if (countdown <= 0) countdown = 0; }
            else UpdateGame(dt);
        }
        else if (screen == SUCCESS || screen == FAIL || screen == CREDITS)
        {
            screenTimer += dt;
            if (screenTimer > 3.0f || IsKeyPressed(KEY_M)) { screen = MENU; screenTimer = 0; }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawGame();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void InitGame(void)
{
    w = GetScreenWidth(); h = GetScreenHeight();
    fenceY = h * 0.65f; barY = h - 80;
    screen = MENU; menuSel = 0; levelSel = 0;
    gold = 0; ammo = 1;
    hasGrenade = hasLaser = hasShield = false;
    level2 = level3 = false;
    player = (Vector2){w/2, h*0.8f};
    weapon = BASIC;
    devMode = false;
    spinAngle = countdown = screenTimer = 0;
    playerShakeTimer = 0;
    playerShakeOffset = (Vector2){0,0};
    memset(bullets, 0, sizeof(bullets));
    memset(enemies, 0, sizeof(enemies));
    memset(explosions, 0, sizeof(explosions));
    shield = (Shield){0};
}

void SpawnLevel(int lvl)
{
    alive = bigAlive = 0;
    memset(bullets, 0, sizeof(bullets));
    memset(enemies, 0, sizeof(enemies));

    int smallCount = (lvl == 1) ? 10 : 20;
    int bigCount = (lvl == 2) ? 3 : (lvl == 3) ? 3 : 0;
    bool hasBoss = (lvl == 3);

    int idx = 0;
    for (int i = 0; i < bigCount && idx < MAX_ENEMIES; i++)
    {
        bool isBoss = (lvl == 3 && i == bigCount-1);
        float size = isBoss ? 70 : 50;
        enemies[idx++] = (Enemy){
            .alive = true,
            .pos = { w/2 + (i-1)*200, 120 },
            .speed = isBoss ? 180 : 160,
            .vel = {0,0},
            .targetVel = {1.0f, 0},
            .big = true,
            .boss = isBoss,
            .health = isBoss ? 300 : 40,
            .maxHealth = isBoss ? 300 : 40,
            .size = size,
            .baseSize = size,
            .color = isBoss ? MAROON : ORANGE,
            .shootTimer = 0,
            .changeTimer = GetRandomValue(150,300)*0.01f,
            .burstCount = 0,
            .bigRocket = false,
            .shakeTimer = 0,
            .shakeOffset = {0,0}
        };
        alive++; bigAlive++;
    }

    for (int i = 0; i < smallCount && idx < MAX_ENEMIES; i++)
    {
        enemies[idx++] = (Enemy){
            .alive = true,
            .pos = { GetRandomValue(100, w-100), GetRandomValue(fenceY-200, fenceY-50) },
            .speed = 160,
            .vel = {0,0},
            .targetVel = { GetRandomValue(-100,100)/100.0f, GetRandomValue(-20,20)/100.0f },
            .big = false,
            .boss = false,
            .health = 1,
            .maxHealth = 1,
            .size = 24,
            .baseSize = 24,
            .color = LIME,
            .shootTimer = 0,
            .changeTimer = GetRandomValue(100,300)*0.01f,
            .shakeTimer = 0,
            .shakeOffset = {0,0}
        };
        alive++;
    }
}

Vector2 GetMuzzlePos(void)
{
    return (Vector2){ player.x + 26, player.y - 65 };
}

void FireWeapon(void)
{
    if (ammo <= 0) return;

    int cost = (weapon == GRENADE) ? 0 : (weapon == LASER) ? 0 : 0;
    if (ammo < cost) return;
    ammo -= cost;

    if (weapon == SHIELD)
    {
        if (ammo >= 10)
        {
            ammo -= 10;
            shield.active = true;
            shield.duration = 15.0f;
            shield.alpha = 1.0f;
        }
        return;
    }

    int slot = -1;
    for (int i = 0; i < MAX_BULLETS; i++)
        if (!bullets[i].active) { slot = i; break; }
    if (slot == -1) return;

    Vector2 muzzle = GetMuzzlePos();

    bullets[slot] = (Bullet){
        .pos = muzzle,
        .vel = (weapon == BASIC) ? (Vector2){0, -900} :
               (weapon == GRENADE) ? (Vector2){GetRandomValue(-200,200), -1100} :
               (Vector2){0, 0},
        .timer = 0,
        .type = weapon,
        .active = true,
        .player = true
    };
}

void UpdateGame(float dt)
{
    if (IsKeyPressed(KEY_ONE)) weapon = BASIC;
    if (IsKeyPressed(KEY_TWO) && hasGrenade) weapon = GRENADE;
    if (IsKeyPressed(KEY_THREE) && hasLaser) weapon = LASER;
    if (IsKeyPressed(KEY_FOUR) && hasShield)
    {
        if (ammo >= 10)
        {
            ammo -= 10;
            shield.active = true;
            shield.duration = 15.0f;

            shield.alpha = 1.0f;
        }
    }
    if (IsKeyPressed(KEY_E)) {
        if (ammo > 0 || weapon == SHIELD) {
            FireWeapon();
        } else {
            screen = FAIL;
            screenTimer = 0;
        }
    }

    if (IsKeyDown(KEY_W) && player.y > fenceY + 40) player.y -= 300 * dt;
    if (IsKeyDown(KEY_S) && player.y < barY - 40) player.y += 300 * dt;
    if (IsKeyDown(KEY_A) && player.x > 40) player.x -= 300 * dt;
    if (IsKeyDown(KEY_D) && player.x < w - 40) player.x += 300 * dt;

    if (shield.active)
    {
        shield.duration -= dt;
        if (shield.duration <= 0) shield.active = false;
    }

    UpdateBullets(dt);
    UpdateEnemies(dt);
    HandleCollisions(dt);

    if (playerShakeTimer > 0)
    {
        playerShakeTimer -= dt;
        playerShakeOffset.x = (GetRandomValue(-100,100)/100.0f) * 4;
        playerShakeOffset.y = (GetRandomValue(-100,100)/100.0f) * 4;
    }
    else
    {
        playerShakeOffset = (Vector2){0,0};
    }

    if (level == 1 && alive == 0) { level2 = true; screen = SUCCESS; screenTimer = 0; }
    if (level == 2 && bigAlive == 0) { level3 = true; screen = SUCCESS; screenTimer = 0; }
    if (level == 3 && bigAlive == 0) { screen = CREDITS; screenTimer = 0; }
}

void UpdateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Bullet *b = &bullets[i];
        if (!b->active) continue;
        b->timer += dt;

        if (b->type == 0)
        {
            b->pos.y += b->vel.y * dt;
            if (b->pos.y < -50 || b->pos.y > h + 50) b->active = false;
        }
        else if (b->type == 1)
        {
            b->vel.y += 1600 * dt;
            b->pos.x += b->vel.x * dt;
            b->pos.y += b->vel.y * dt;

            if (b->timer > 0.9f)
            {
                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!enemies[e].alive) continue;
                    float dx = b->pos.x - enemies[e].pos.x;
                    float dy = b->pos.y - enemies[e].pos.y;
                    if (sqrtf(dx*dx + dy*dy) < 180.0f)
                    {
                        if (enemies[e].boss)
                            enemies[e].health -= 0.5f;
                        else if (enemies[e].big)
                            enemies[e].health -= 15;
                        else
                            enemies[e].health = 0;

                        if (enemies[e].health <= 0)
                        {
                            enemies[e].alive = false;
                            alive--;
                            if (enemies[e].big || enemies[e].boss) bigAlive--;
                            gold += enemies[e].big ? 20 : 1;
                            ammo += enemies[e].big ? 15 : 2;
                        }
                    }
                }

                for (int j = 0; j < MAX_EXPLOSIONS; j++)
                {
                    if (!explosions[j].active)
                    {
                        explosions[j].pos = b->pos;
                        explosions[j].timer = 0.4f;
                        explosions[j].active = true;
                        break;
                    }
                }

                b->active = false;
            }
        }
        else
        {
            if (b->timer > 3.0f) b->active = false;
        }
    }
}

void UpdateEnemies(float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive) continue;

        enemies[i].changeTimer -= dt;
        if (enemies[i].changeTimer <= 0)
        {
            float maxX = enemies[i].boss ? 0.7f : (enemies[i].big ? 0.6f : 1.0f);
            float maxY = enemies[i].boss ? 0.3f : (enemies[i].big ? 0.2f : 0.3f);
            enemies[i].targetVel.x = GetRandomValue(-100,100)/100.0f * maxX;
            enemies[i].targetVel.y = GetRandomValue(-100,100)/100.0f * maxY;
            enemies[i].changeTimer = GetRandomValue(120,250)*0.01f;
        }

        enemies[i].vel.x += (enemies[i].targetVel.x - enemies[i].vel.x) * 5 * dt;
        enemies[i].vel.y += (enemies[i].targetVel.y - enemies[i].vel.y) * 5 * dt;

        enemies[i].pos.x += enemies[i].vel.x * enemies[i].speed * dt;
        enemies[i].pos.y += enemies[i].vel.y * enemies[i].speed * dt;

        if (enemies[i].pos.x < 100) { enemies[i].pos.x = 100; enemies[i].vel.x *= -0.6f; }
        if (enemies[i].pos.x > w-100) { enemies[i].pos.x = w-100; enemies[i].vel.x *= -0.6f; }
        if (enemies[i].pos.y < 100) { enemies[i].pos.y = 100; enemies[i].vel.y *= -0.6f; }
        if (enemies[i].pos.y > fenceY-100) { enemies[i].pos.y = fenceY-100; enemies[i].vel.y *= -0.6f; }

        float ratio = (float)enemies[i].health / enemies[i].maxHealth;
        enemies[i].size = enemies[i].baseSize * (0.7f + 0.3f * ratio);

        if (enemies[i].shakeTimer > 0)
        {
            enemies[i].shakeTimer -= dt;
            enemies[i].shakeOffset.x = (GetRandomValue(-100,100)/100.0f) * 3;
            enemies[i].shakeOffset.y = (GetRandomValue(-100,100)/100.0f) * 3;
        }
        else
        {
            enemies[i].shakeOffset = (Vector2){0,0};
        }

        // ENEMIES ALWAYS SHOOT
        if (enemies[i].big && !enemies[i].boss)
        {
            enemies[i].shootTimer += dt;
            if (enemies[i].shootTimer > 1.8f)
            {
                for (int j = 0; j < MAX_BULLETS; j++)
                {
                    if (!bullets[j].active)
                    {
                        bullets[j] = (Bullet){
                            .pos = enemies[i].pos,
                            .vel = {0, 500},
                            .timer = 0,
                            .type = 0,
                            .active = true,
                            .player = false
                        };
                        break;
                    }
                }
                enemies[i].shootTimer = 0;
            }
        }

        if (enemies[i].boss)
        {
            enemies[i].shootTimer += dt;
            if (enemies[i].shootTimer > 0.8f)
            {
                enemies[i].burstCount++;
                if (enemies[i].burstCount <= 5)
                {
                    for (int j = 0; j < MAX_BULLETS; j++)
                    {
                        if (!bullets[j].active)
                        {
                            Vector2 dir = { player.x - enemies[i].pos.x, player.y - enemies[i].pos.y };
                            float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
                            if (len > 0) { dir.x /= len; dir.y /= len; }
                            bullets[j] = (Bullet){
                                .pos = enemies[i].pos,
                                .vel = { dir.x * 600, dir.y * 600 },
                                .timer = 0,
                                .type = 0,
                                .active = true,
                                .player = false
                            };
                            break;
                        }
                    }
                    enemies[i].shootTimer = 0.15f;
                }
                else
                {
                    enemies[i].burstCount = 0;
                    enemies[i].shootTimer = 2.0f;
                }
            }
        }
    }
}

void HandleCollisions(float dt)
{
    for (int b = 0; b < MAX_BULLETS; b++)
    {
        if (!bullets[b].active || bullets[b].type == 1) continue;

        if (bullets[b].player)
        {
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (!enemies[e].alive) continue;
                if (CheckCollisionCircles(bullets[b].pos, 8, enemies[e].pos, enemies[e].size))
                {
                    enemies[e].health--;
                    enemies[e].shakeTimer = 0.1f;
                    if (enemies[e].health <= 0)
                    {
                        enemies[e].alive = false;
                        alive--;
                        if (enemies[e].big || enemies[e].boss) bigAlive--;
                        gold += enemies[e].big ? 20 : 1;
                        ammo += enemies[e].big ? 15 : 2;
                    }
                    if (bullets[b].type != 2) bullets[b].active = false;
                }
            }
        }
        else if (CheckCollisionCircles(bullets[b].pos, 8, player, 90))
        {
            float dx = bullets[b].pos.x - player.x;
            float dy = bullets[b].pos.y - player.y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (shield.active && dist <= 98.0f)
            {
                bullets[b].vel.x = -1.0f;
                bullets[b].vel.y= -1.0f;
                playerShakeTimer = 0.15f;
                if (dist > 0.1f)
                {
                    bullets[b].pos.x = player.x + (dx / dist) * 98.0f;
                    bullets[b].pos.y = player.y + (dy / dist) * 98.0f;
                }
            }
        }
        else if (CheckCollisionCircleRec(bullets[b].pos, 8, (Rectangle){player.x-30, player.y-30, 60, 60}))
            {
                screen = FAIL;
                screenTimer = 0;
                bullets[b].active = false;
            }
    }
    

    for (int b = 0; b < MAX_BULLETS; b++)
    {
        if (bullets[b].active && bullets[b].type == 2 && bullets[b].player)
        {
            float width = 20;
            Rectangle beam = { player.x - width/2, 0, width, player.y - 20 };
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (!enemies[e].alive) continue;
                if (CheckCollisionCircleRec(enemies[e].pos, enemies[e].size, beam))
                {
                    enemies[e].health -= 20 * dt;
                    enemies[e].shakeTimer = 0.05f;
                    if (enemies[e].health <= 0)
                    {
                        enemies[e].alive = false;
                        alive--;
                        if (enemies[e].big || enemies[e].boss) bigAlive--;
                        gold += enemies[e].big ? 20 : 1;
                        ammo += enemies[e].big ? 15 : 2;
                    }
                }
            }
        }
    }
}

void DrawPlayer(void)
{
    Vector2 drawPos = { player.x + playerShakeOffset.x, player.y + playerShakeOffset.y };
    DrawCircle(drawPos.x - 25, drawPos.y + 15, 18, DARKBLUE);
    DrawCircle(drawPos.x + 25, drawPos.y + 15, 18, DARKBLUE);
    DrawCircleV(drawPos, 30, weapon == LASER ? PURPLE : SKYBLUE);

    DrawRectangle(drawPos.x + 20, drawPos.y - 60, 12, 60, GRAY);
    DrawRectangle(drawPos.x + 15, drawPos.y - 65, 22, 10, DARKGRAY);

    if (weapon == LASER && IsKeyDown(KEY_E))
    {
        DrawCircle(GetMuzzlePos().x + playerShakeOffset.x, GetMuzzlePos().y + playerShakeOffset.y, 20, Fade(PURPLE, 0.3f));
    }
}

void DrawShield(void)
{
    if (shield.active)
    {
        Vector2 p = { player.x + playerShakeOffset.x, player.y + playerShakeOffset.y };
        DrawRing(p, 70, 90, 0, -180, 32, Fade(SKYBLUE, 0.7f));
    }
}

void DrawHUD(void)
{
    DrawText(TextFormat("GOLD: %d", gold), 20, 20, 30, YELLOW);
    DrawText(TextFormat("AMMO: %d", ammo), 20, 60, 30, ammo > 0 ? GREEN : RED);
    if (devMode) DrawText("DEV MODE", w - 210, 20, 40, RED);
    DrawText("Press M to return to menu", w - 300, h - 30, 20, Fade(WHITE, 0.6f));
}

void DrawShop(void)
{
    DrawRectangle(100, 100, w-200, h-220, Fade(BLACK, 0.9f));
    DrawText("SHOP", w/2 - 100, 130, 80, GOLD);
    DrawText("1 - NADES (4g)", 300, 280, 40, hasGrenade ? GREEN : WHITE);
    DrawText("   explodes, kills everything", 300, 320, 30, Fade(WHITE, 0.7f));
    DrawText("2 - Yuge Laser (12g)", 300, 460, 40, hasLaser ? GREEN : WHITE);
    DrawText("   hold E to fire", 300, 500, 30, Fade(WHITE, 0.7f));
    DrawText("3 - Shield (8g)", 300, 600, 40, hasShield ? GREEN : WHITE);
    DrawText("   press 4 to activate", 300, 640, 30, Fade(WHITE, 0.7f));
    DrawText(TextFormat("GOLD: %d", gold), 150, 130, 50, YELLOW);
}

void DrawControlsOverlay(void)
{
    if (screen != PLAY) return;
    
    DrawText("WASD - MOVE", 20, barY - 140, 32, BLACK);
    DrawText("1-4 WEAPONS", 20, barY - 105, 32, BLACK);
    DrawText("E - FIRE", 20, barY - 70, 32, BLACK);
    DrawText("M - MENU", 20, barY - 35, 32, BLACK);
}

void DrawGame(void)
{
    DrawControlsOverlay();  // DRAWN FIRST

    if (screen != PLAY || countdown > 0)
    {
        DrawCircle(w - 80, 80, 40, Fade(YELLOW, 0.8f));
        DrawPoly((Vector2){w-80,80}, 6, 30, spinAngle, WHITE);
    }

    for (int x = 0; x < w; x += 20) DrawPixel(x, fenceY, WHITE);

    DrawRectangle(0, barY, w, 80, Fade(BLACK, 0.9f));
    Color itemColor = LIGHTGRAY;
    DrawText("1 pew pew", 50, barY + 25, 30, weapon == BASIC ? YELLOW : itemColor);
    DrawText(hasGrenade ? "2 NADES" : "2 NADES", 300, barY + 25, 30, weapon == GRENADE ? YELLOW : itemColor);
    DrawText(hasLaser ? "3 LASER" : "3 LASER", 600, barY + 25, 30, weapon == LASER ? YELLOW : itemColor);
    DrawText(hasShield ? "4 SHIELD" : "4 SHIELD", 900, barY + 25, 30, shield.active ? YELLOW : itemColor);

    DrawPlayer();
    DrawShield();

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active) continue;
        if (bullets[i].type == 0)
            DrawCircleV(bullets[i].pos, 8, bullets[i].player ? RED : PINK);
        if (bullets[i].type == 1)
            DrawCircleV(bullets[i].pos, 12, ORANGE);
        if (bullets[i].type == 2)
        {
            float width = 20;
            DrawRectangle(player.x - width/2 + playerShakeOffset.x, 0, width, player.y - 20, Fade(RED, 0.7f));
            DrawRectangle(player.x - width/2 + 4 + playerShakeOffset.x, 0, width-8, player.y - 20, Fade(YELLOW, 0.7f));
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive) continue;
        Vector2 drawPos = { enemies[i].pos.x + enemies[i].shakeOffset.x, enemies[i].pos.y + enemies[i].shakeOffset.y };
        DrawCircleV(drawPos, enemies[i].size, enemies[i].color);

        if (enemies[i].boss)
            DrawText("DADDY", drawPos.x - 35, drawPos.y - 15, 24, WHITE);
        else if (enemies[i].big)
            DrawText("15", drawPos.x - 15, drawPos.y - 15, 24, WHITE);
        else
            DrawText("2", drawPos.x - 8, drawPos.y - 10, 20, WHITE);

        if (enemies[i].shakeTimer > 0)
        {
            for (int s = 0; s < 3; s++)
            {
                Vector2 spark = { drawPos.x + GetRandomValue(-20,20), drawPos.y + GetRandomValue(-20,20) };
                DrawPixelV(spark, YELLOW);
            }
        }
    }

    for (int i = 0; i < MAX_EXPLOSIONS; i++)
        if (explosions[i].active)
        {
            float r = 180 * (explosions[i].timer/0.4f);
            DrawCircleV(explosions[i].pos, r, Fade(ORANGE, explosions[i].timer/0.4f));
            explosions[i].timer -= GetFrameTime();
            if (explosions[i].timer <= 0) explosions[i].active = false;
        }

    if (countdown > 0)
        DrawText(TextFormat("%.1f", countdown), w/2 - 50, h/2 - 50, 120, YELLOW);

    DrawHUD();

    if (screen == MENU)
    {
        DrawText("ONE SHOT, ONE KILL", w/2 - 300, 200, 80, GOLD);
        DrawText("Levels", w/2 - 100, 400, 60, menuSel == 0 ? YELLOW : GRAY);
        DrawText("Shop", w/2 - 80, 480, 60, menuSel == 1 ? YELLOW : GRAY);
        DrawText("Quit", w/2 - 80, 560, 60, menuSel == 2 ? YELLOW : GRAY);
    }
    else if (screen == LEVELS)
    {
        DrawText("SELECT LEVEL", w/2 - 250, 150, 70, WHITE);
        DrawText("Level 1", w/2 - 120, 300, 50, levelSel == 0 ? YELLOW : WHITE);
        DrawText(level2 ? "Level 2" : "Level 2 - LOCKED", w/2 - 120, 380, 50, levelSel == 1 ? YELLOW : WHITE);
        DrawText(level3 ? "Level 3" : "Level 3 - LOCKED", w/2 - 120, 460, 50, levelSel == 2 ? YELLOW : WHITE);
    }
    else if (screen == SHOP) DrawShop();
    else if (screen == SUCCESS) DrawText("LEVEL COMPLETE!", w/2 - 300, h/2 - 50, 80, GREEN);
    else if (screen == CREDITS)
    {
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.8f));
        DrawText("CREDITS", w/2 - 200, h/2 - 120, 80, GOLD);
        DrawText("Matthew Johnson", w/2 - 220, h/2 - 20, 50, WHITE);
        DrawText("Nathan Ly", w/2 - 140, h/2 + 40, 50, WHITE);
    }
    else if (screen == FAIL) DrawText("FAILURE!", w/2 - 250, h/2 - 50, 100, RED);
}