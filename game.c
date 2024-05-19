/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#include "gframework.c"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------------
int gameTimer = 0;
// mining
int miningX = 0;
int miningY = -1;
int miningProgress = 40;
int miningTime = 40;
int currentMiningTime = 0;


#define WORLD_WIDTH 20
#define WORLD_HEIGHT 20
int depth = 0;
// types
const int TYPE_AIR = 0;
const int TYPE_ROCK = 1;
const int TYPE_TOUGH_ROCK = 2;


// modifiers
const int MODIFIER_NONE = 0;
#define MODIFIER_SILVER 1
#define MODIFIER_GOLD 2
#define MODIFIER_DIAMONDS 3
#define MODIFIER_COAL 4
#define MODIFIER_ZIRCON 5
#define MODIFIER_COBALT 6
#define MODIFIER_OPAL 7
#define MODIFIER_SPIKES 8


const int MODIFIER_OFFSET = 10;

float worldOffset = 0.0f;

//------------------------------------------------------------------------------------
// TextPopups
//------------------------------------------------------------------------------------
#define TEXT_POPUP_LENGTH 10
struct TextPopup{
    int x;
    int y;
    char text[TEXT_POPUP_LENGTH];
    int lifeTime;
    bool exists;
    Color c;
};
typedef struct TextPopup TextPopup;
#define MAX_POPUPS 3

TextPopup popups[MAX_POPUPS];

void updatePopups(){
    for (int i = 0; i < MAX_POPUPS; i++){
        TextPopup* p = &popups[i];

        if (p->exists){
            p->y -= 1;
            p->lifeTime--;
            drawFancyText(p->text, p->x, p->y, 1, p->c);
            if (p->lifeTime == 0){
                p->exists = false;
            }
        }
    }
}

int nextPopupIndex = 0;
void initPopup(int x, int y, char text[TEXT_POPUP_LENGTH], Color c){
    TextPopup p = {
        .x = x, .y = y, .exists = true, .lifeTime = 45, .c = c
    };
    strcpy(p.text, text);

    popups[nextPopupIndex] = p;
    nextPopupIndex++;
    nextPopupIndex %= MAX_POPUPS;
}

//------------------------------------------------------------------------------------
// Shop
//------------------------------------------------------------------------------------

int shopX;
int shopY;
bool isShopOpen = false;
bool shopInteracted = false;
int selectedShopSlot = 0;

int itemLevels[] = {0, 0, 0};
const int costMultiplier = 64;
void activateSlot();
char displayText[10];

int calculatePrice(int slot){
    return itemLevels[slot] * costMultiplier + 100;
}

void updateShop(){
    draw(36, shopX, shopY - worldOffset - (depth * 32));
    if (shopInteracted == false){
        drawFancyText("SHOP", shopX + 4, shopY - 16 - worldOffset - (depth * 32), 10, GOLD);
    }



    if (isShopOpen){
        for (int i = 0; i < 4; i++){
            Color c = GRAY;
            if (i == selectedShopSlot){
                c = WHITE;
            }

            if (i < 3){
                sprintf(displayText, "%i000$", calculatePrice(i));
                drawFancyText(displayText, 194 + i * 64, 132, 1, WHITE);

            }
            drawC(37 + i, 194 + i * 64, 100, c);
        }


        if (IsKeyPressed(KEY_A)){
            selectedShopSlot--;
            if (selectedShopSlot < 0){
                selectedShopSlot = 3;
            }
        }
        if (IsKeyPressed(KEY_D)){
            selectedShopSlot++;
            selectedShopSlot %= 4;
        }

        if (IsKeyPressed(KEY_S)){
            activateSlot();
        }
    }
}

void generateShop(int y){
    shopX = GetRandomValue(0, WORLD_WIDTH) * 32;
    shopY = y * 32;
    shopInteracted = false;

}

void openShop(){

    shopInteracted = true;
    isShopOpen = true;

}




//------------------------------------------------------------------------------------
// Particles
//------------------------------------------------------------------------------------
struct Particle{
    float x;
    float y;
    float velocityX;
    float velocityY;
    bool exists;
    int internalTimer;
    Color color;
};
typedef struct Particle Particle;


#define MAX_PARTICLES 30
Particle particles[MAX_PARTICLES];
int nextParticleIndex = 0;
void updateParticles(){
    for (int i = 0; i < MAX_PARTICLES; i++){
        Particle* p = &particles[i];

        if (p->exists){
            p->x += p->velocityX;
            p->y += p->velocityY;
            p->velocityY += (p->velocityY < 3.0f) * 0.1f;
            p->internalTimer++;


            drawC(29 + (((p->internalTimer % 10) / 10.0f) * 3), p->x, p->y - worldOffset - (depth * 32), p->color);
        }
    }
}

void addParticle(int x, int y, Color c){
    Particle p = {
        x + GetRandomValue(-16, 16), y + GetRandomValue(-16, 16), GetRandomValue(-1,1), GetRandomValue(-3, 1), true, GetRandomValue(0, 20), c
    };

    particles[nextParticleIndex] = p;
    nextParticleIndex++;
    nextParticleIndex %= MAX_PARTICLES;
}

//------------------------------------------------------------------------------------
// World
//------------------------------------------------------------------------------------
struct WorldTile{
    int type;
    bool isSolid;
    int modifier;
    int sprite;
};
typedef struct WorldTile WorldTile;



int convertMiningY(int yToConvert){
    return yToConvert - depth;
}

WorldTile world[WORLD_WIDTH][WORLD_HEIGHT];

void finishedMiningTile(int x, int y);

void updateWorld(){
    for (int x = 0; x < WORLD_WIDTH; x++){
        for (int y = 0; y < WORLD_HEIGHT; y++){
            WorldTile tile = world[x][y];

            if (tile.type == TYPE_ROCK){
                draw((tile.sprite * 2) + !tile.isSolid, x * 32, y * 32 - worldOffset);

                if (tile.modifier != MODIFIER_NONE){
                    int modifierValue = tile.modifier + MODIFIER_OFFSET;
                    switch (tile.modifier){
                        case MODIFIER_SPIKES: modifierValue = 15; break;
                        case MODIFIER_ZIRCON: modifierValue = 33; break;
                        case MODIFIER_COBALT: modifierValue = 34; break;
                        case MODIFIER_OPAL: modifierValue = 35; break;
                    }
                    draw(modifierValue, x * 32, y * 32 - worldOffset);
                }

            }else if (tile.type == TYPE_TOUGH_ROCK){
                draw(10, x * 32, y * 32 - worldOffset);
            }

        }
    }

    // mining
    int y = convertMiningY(miningY);
    if (miningProgress >= currentMiningTime && !(miningX == 0 && miningY == -1)){
        finishedMiningTile(miningX, miningY);
        WorldTile* tile = &world[miningX][y];
        tile->isSolid = false;
        tile->modifier = MODIFIER_NONE;
        screenShake(4.5f);
        miningProgress = 0;
        miningX = 0;
        miningY = -1;
    }else {
        draw(16 + floor(((float)miningProgress / currentMiningTime * 3)), miningX * 32, y * 32 - worldOffset);
    }
}

int getMiningTimeForTile(int x, int y){
    int cY = convertMiningY(y);

    WorldTile tile = world[x][cY];

    int out = miningTime;

    out += tile.sprite * 30;
    switch (tile.modifier){
        case MODIFIER_COAL:out += 10;break;
        case MODIFIER_SILVER:out += 30;break;
        case MODIFIER_GOLD:out += 60;break;
        case MODIFIER_DIAMONDS:out += 100;break;
        case MODIFIER_ZIRCON:out += 120;break;
        case MODIFIER_COBALT:out += 130; break;
        case MODIFIER_OPAL:out += 200; break;
    }

    return out;

}


bool isTileMinable(int x, int y){
    int cY = convertMiningY(y);

    WorldTile tile = world[x][cY];

    return tile.isSolid && tile.type == TYPE_ROCK;
}

Color getColorForTile(int x, int y){
    int cY = convertMiningY(y);

    WorldTile tile = world[x][cY];
    Color out = WHITE;

    switch (tile.sprite){
        default:
        case 0: out.r = 38; out.g = 133; out.b = 76; break;
        case 1: out.r = 162; out.g = 109; out.b = 63; break;
        case 2: out.r = 222; out.g = 93; out.b = 58; break;
        case 3: out.r = 107; out.g = 58; out.b = 222; break;
        case 4: out.r = 107; out.g = 38; out.b = 67; break;
    }


    if (tile.modifier != MODIFIER_NONE && GetRandomValue(0, 9) > 6){
        switch (tile.modifier){
            default:
            case MODIFIER_SILVER: out.r = 222; out.g = 206; out.b = 237; break;
            case MODIFIER_GOLD: out.r = 243; out.g = 168; out.b = 51; break;
            case MODIFIER_DIAMONDS: out.r = 109; out.g = 234; out.b = 214; break;
            case MODIFIER_SPIKES: out.r = 236; out.g = 39; out.b = 63; break;
            case MODIFIER_COAL: out.r = 44; out.g = 30; out.b = 49; break;
            case MODIFIER_ZIRCON: out.r = 90; out.g = 181; out.b = 82; break;
            case MODIFIER_COBALT: out.r = 51; out.g = 136; out.b = 222; break;
            case MODIFIER_OPAL: out.r = 255; out.g = 162; out.b = 172; break;

        }

    }

    return out;
}

bool canMoveToWH(float x, float y, float w, float h){
    if (x < 0 || x  + w > 640){
        return false;
    }
    for (int i = x; i < x + w; i += 1){
        for (int j = y; j < y + h; j += 1){
            int cx = (i / 32);
            int cy = convertMiningY((j / 32));
            if (world[cx][cy].isSolid){
                return false;
            }
        }

    }

    return true;
}

bool canMoveTo(float x, float y){
    return canMoveToWH(x,y, 32, 32);
}

void mineTile(int x, int y){
    if (!isTileMinable(x, y)){
        return;
    }
    if (miningX == x && miningY == y){
        miningProgress += miningProgress < currentMiningTime;

        if (gameTimer % 3 == 0){
            Color c = getColorForTile(x, y);
            addParticle(x * 32, y * 32, c);
        }
    }else {
        miningX = x;
        miningY = y;
        miningProgress = 0;
        currentMiningTime = getMiningTimeForTile(x, y);
    }
}

WorldTile generateTile(int tileDepth){
    WorldTile output;

    output.isSolid = true;
    output.modifier = 0;
    output.sprite = 0;
    output.type = TYPE_ROCK;
    output.modifier = MODIFIER_NONE;

    // choose sprite

    if (tileDepth < 150){
        output.sprite = 0;
    }else if (tileDepth < 160){
        output.sprite = GetRandomValue(0, 1);
    }else if (tileDepth < 270){
        output.sprite = 1;
    }else if (tileDepth < 280){
        output.sprite = GetRandomValue(1, 2);
    }else if (tileDepth < 400){
        output.sprite = 2;
    }else if (tileDepth < 410){
        output.sprite = GetRandomValue(2,3);
    }else if (tileDepth < 600){
        output.sprite = 3;
    }else if (tileDepth < 610){
        output.sprite = GetRandomValue(3, 4);
    }else {
        output.sprite = 4;
    }


    // generate air
    if (tileDepth <= 5){
        output.isSolid = false;
        output.type = TYPE_AIR;
    }else if (GetRandomValue(0, 100) < min(10, (tileDepth / 20))){
        output.type = TYPE_TOUGH_ROCK;
    }else if (tileDepth > 10){
        // generate modifier
        int rng = GetRandomValue(0, 100);

        if (rng < min(45, fmax((tileDepth / 20.0f), 5))){
            rng = GetRandomValue(0, 100);

            if (rng < min(45, (float)tileDepth / 20 + ((sin(tileDepth * DEG2RAD) * 0.5f + 0.5f) * 20.0f))){
                output.modifier = MODIFIER_SPIKES;
            }else if (rng < 90){
                // money
                output.modifier = MODIFIER_SILVER;
                rng = GetRandomValue(0, 100);
                if (rng < min(45, (float)tileDepth / 10 + ((sin(tileDepth * DEG2RAD) * 0.5f + 0.5f) * 30.0f))){
                    output.modifier++;
                }
                rng = GetRandomValue(0, 100);
                if (rng > 95 && tileDepth > 150){
                    output.modifier++;
                }
                if (tileDepth > 250){
                    output.modifier++;
                }
                rng = GetRandomValue(0, 100);
                if (tileDepth > 350 && rng > 65){
                    output.modifier++;
                }
                if (tileDepth > 650){
                    output.modifier++;
                }
            }else {
                // coal
                output.modifier = MODIFIER_COAL;
            }
        }
    }
    return output;
}



void generateLayer(int layer){
    for (int i = 0; i < WORLD_WIDTH; i++){
        world[i][layer] = generateTile(depth + layer);
        // generate shop
        if ((layer + depth) % 120 >= 115){
            world[i][layer].isSolid = false;
            world[i][layer].type = TYPE_ROCK;
            world[i][layer].modifier = MODIFIER_NONE;

        }

    }
    if (layer + depth == 5){
        generateShop(layer+depth);
    }

    if ((layer+depth) % 120 == 119){
        generateShop(layer+depth);
    }
}

void moveDown(float ammount){
    worldOffset += ammount;

    if (worldOffset > 32.0f){
        worldOffset -= 32.0f;
        // shift world upwards
        for (int i = 0; i < WORLD_HEIGHT - 1; i++){
            for (int j = 0; j < WORLD_WIDTH; j++){
                world[j][i] = world[j][i+1];
            }
        }
        depth++;
        generateLayer(WORLD_HEIGHT - 1);
    }
}

//------------------------------------------------------------------------------------
// player
//------------------------------------------------------------------------------------

const int DIRECTION_LEFT = 0;
const int DIRECTION_DOWN = 1;
const int DIRECTION_RIGHT = 2;


struct Player{
    float x;
    float y;
    float fuel;
    float maxFuel;
    int health;
    int maxHealth;
    float velocityX;
    float velocityY;
    int direction;
    int money;
};
typedef struct Player Player;

Player initPlayer(float x, float y){
    Player out = {
        .x = x,
        .y = y,
        .fuel = 40,
        .maxFuel = 40,
        .health = 50,
        .maxHealth = 50,
        .velocityX = 0.0f,
        .velocityY = 0.0f,
        .direction = DIRECTION_RIGHT,
        .money = 100,
    };
    return out;
}
Player player;

void playerAlive(bool isOnGround, float convY){

    // camera
    if (convY > 200){
        moveDown(fmax(1, player.velocityY));
    }

    // movement
    if (IsKeyDown(KEY_A) && player.velocityX > -2.5f){
        player.velocityX -= 0.1f;
        player.direction = DIRECTION_LEFT;
        player.fuel -= 0.01f;

    }else if (IsKeyDown(KEY_D) && player.velocityX < 2.5f){
        player.velocityX += 0.1f;
        player.direction = DIRECTION_RIGHT;
        player.fuel -= 0.01f;

    }else if (IsKeyDown(KEY_S)){
        player.direction = DIRECTION_DOWN;

        if (isOnGround){
            mineTile((player.x + 16) / 32, (player.y / 32) + 2);
            player.fuel -= 0.01f;
        }
    }

    if (player.velocityX != 0 && !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
        player.velocityX *= 0.9;
        if (fabs(player.velocityX) < 0.1f){
                player.velocityX = 0;
        }
    }

    if (IsKeyPressed(KEY_W) && isOnGround){
        player.velocityY -= 2.5f;
    }

    // shop
    if (shopInteracted == false && checkBoxCollisions(player.x, player.y, 32, 32, shopX, shopY, 32, 32)){
        player.velocityX = 0;
        openShop();

    }
}

void updatePlayer(){
    bool isOnGround = true;
    float convY = player.y - worldOffset - (depth * 32);


    if (canMoveToWH(player.x + 2, player.y + 34, 28, 1)){
        isOnGround = false;

        if (player.velocityY < 3.0f){
            player.velocityY += 0.1f;
        }
    }

    if (canMoveToWH(player.x + 2, player.y + 2 + player.velocityY + (30 * (player.velocityY > 0)), 28, 1)){
        player.y += player.velocityY;

    }else {
        player.velocityY = 0;
    }


    if (canMoveToWH(player.x + ((player.velocityX > 0) * 32), player.y + 4, 1, 28)){
        player.x += player.velocityX;
    }else {
        if (isOnGround){
            mineTile(((player.x + 16) / 32) + (sign(player.velocityX) * 1), (player.y + 32) / 32);
        }
        player.velocityX = 0;
    }
    bool isAlive = player.health > 0 && player.fuel > 0;
    if (isAlive && !isShopOpen){
        playerAlive(isOnGround, convY);
    }

    if (player.health < 0){
        player.health = 0;
    }
    if (player.fuel < 0){
        player.fuel = 0;
    }

    // draw
    int yOffset = 1;
    if (player.direction == DIRECTION_DOWN && isAlive){
        yOffset = 7;
    }
    if (isAlive){
        draw(23 + (player.direction * 2) + ((gameTimer % 10) > 5), player.x, convY + yOffset);
    }else {
        draw(32, player.x, convY + yOffset);

    }


}

void finishedMiningTile(int x, int y){
    int cY = convertMiningY(y);
    char str[TEXT_POPUP_LENGTH];

    WorldTile tile = world[x][cY];
    switch(tile.modifier){
        case MODIFIER_COAL:
            strcpy(str, "+10L");
            initPopup(x * 32, cY * 32, str, WHITE);

            player.fuel += 10;
            if (player.fuel > player.maxFuel){
                player.fuel = player.maxFuel;
            }
            break;
        case MODIFIER_SILVER:
            strcpy(str, "+2000$");
            initPopup(x * 32, cY * 32, str, GOLD);
            player.money += 2;
            break;
        case MODIFIER_GOLD:
            strcpy(str, "+15000$");
            initPopup(x * 32, cY * 32, str, GOLD);
            player.money += 15;
            break;
        case MODIFIER_DIAMONDS:
            strcpy(str, "+36000$");
            initPopup(x * 32, cY * 32, str, BLUE);
            player.money += 36;
            break;
        case MODIFIER_SPIKES:
            strcpy(str, "-15HP");
            initPopup(x * 32, cY * 32, str, RED);
            player.health -= 15;
            break;
        case MODIFIER_ZIRCON:
            strcpy(str, "+98000$");
            initPopup(x * 32, cY * 32, str, GREEN);
            player.money += 98;
            break;
        case MODIFIER_COBALT:
            strcpy(str, "+256000$");
            initPopup(x * 32, cY * 32, str, BLUE);
            player.money += 256;
            break;
        case MODIFIER_OPAL:
            strcpy(str, "+734000$");
            initPopup(x * 32, cY * 32, str, PINK);
            player.money += 734;
            break;
    }
}

void activateSlot(){

    if (selectedShopSlot <= 2){
        if (player.money < calculatePrice(selectedShopSlot)){
            return;
        }
        player.money -= calculatePrice(selectedShopSlot);
        itemLevels[selectedShopSlot]++;
    }


    switch (selectedShopSlot){

        case 0: miningTime -= 15;break;
        case 1: player.maxFuel += 10; break;
        case 2: player.maxHealth += 10; break;
        case 3: isShopOpen = false; player.health = player.maxHealth; player.fuel = player.maxFuel; break;

    }
}

//------------------------------------------------------------------------------------
// hud
//------------------------------------------------------------------------------------
#define DEPTH_COUNTER_SIZE 30
char display[DEPTH_COUNTER_SIZE];

void updateHud(){
    // depth
    drawFancyText("Hloubka", 10, 10, 20, YELLOW);
    sprintf(display, "%06i", depth);
    drawFancyText(display, 110, 10, 20, YELLOW);

    // fuel
    drawFancyText("Palivo", 10, 40, 20, YELLOW);
    sprintf(display, "%i/%i", (int)player.fuel, (int)player.maxFuel);
    drawFancyText(display, 110, 40, 20, YELLOW);

    // health
    drawFancyText("Integrita", 10, 70, 20, RED);
    sprintf(display, "%i/%i", player.health, player.maxHealth);
    drawFancyText(display, 110, 70, 20, RED);

    // money
    drawFancyText("Prachy", 410, 10, 20, YELLOW);
    sprintf(display, "%06i000$", player.money);
    drawFancyText(display, 510, 10, 20, YELLOW);

}



//------------------------------------------------------------------------------------
// reset
//------------------------------------------------------------------------------------
void reset(){
    depth = 0;
    worldOffset = 0.0f;
    miningTime = 40;

    for (int i = 0; i < 3; i++ ){
        itemLevels[i] = 0;
    }
    for (int i = 0; i < WORLD_HEIGHT; i++){
        generateLayer(i);
    }
    player = initPlayer(0, 0);

}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    initFramework();

    const Color BACKGROUND_COLOR = {51, 136, 222, 255};
    reset();

    // Main game loop
    while (!WindowShouldClose())
    {
        gameTimer++;
        
        fDrawBegin();

            ClearBackground(BACKGROUND_COLOR);
            updateWorld();
            updateShop();
            updatePlayer();
            updateParticles();
            updatePopups();
            updateHud();
        fDrawEnd();
        
    }

	disposeFramework();
    

    return 0;
}
