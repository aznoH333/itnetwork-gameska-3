#ifndef G_FRAMEWORK
#define G_FRAMEWORK

#include "raylib.h"

//------------------------------------------------------
// Conf
//------------------------------------------------------
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const char* WINDOW_NAME = "template window";
const int DEFAULT_SPRITE_SIZE = 32;
const float DEFAULT_CAMERA_ZOOM = 2.0f;


//------------------------------------------------------------------------------------
// UTILITY
//------------------------------------------------------------------------------------
bool checkBoxCollisions(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2){
    return x1 + w1 > x2 &&
           x1 < x2 + w2 &&
           y1 + h1 > y2 &&
           y1 < y2 + h2;
}

float lerp(float a, float b, float w){
    return a * (1.0 - w) + (b * w);
}

float pythagoras(float x1, float y1, float x2, float y2){
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

float dirTowards(int x1, int y1, int x2, int y2){
    return atan2(x1 - x2, y1 - y2);
}

int min(int a, int b){
    if (a < b){
        return a;
    }
    return b;
}

//------------------------------------------------------
// sprites
//------------------------------------------------------
struct FrameworkSpriteSheet{
	Texture2D spriteSheetTexture;
	int width;
	int height;

};
typedef struct FrameworkSpriteSheet FrameworkSpriteSheet;

FrameworkSpriteSheet mainSpriteSheet;
FrameworkSpriteSheet initSpriteSheet(){
	FrameworkSpriteSheet out;
	out.spriteSheetTexture = LoadTexture("resources/spritesheet.png");
	out.width = out.spriteSheetTexture.width / DEFAULT_SPRITE_SIZE;
	out.height = out.spriteSheetTexture.height / DEFAULT_SPRITE_SIZE + 1;
	
	return out;
}

void unloadSpriteSheet(FrameworkSpriteSheet spriteSheet){
	UnloadTexture(spriteSheet.spriteSheetTexture);
}


//------------------------------------------------------
// Variables
//------------------------------------------------------
FrameworkSpriteSheet loadedSheet;
RenderTexture2D renderTexture;
Camera2D cam;
float scalingFactor;
int renderTextureOffset;

//------------------------------------------------------
// drawing
//------------------------------------------------------
void draw(int spriteIndex, int x, int y){	
	Rectangle src = {(spriteIndex % loadedSheet.width) * DEFAULT_SPRITE_SIZE, (spriteIndex / loadedSheet.height) * DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE};
	Rectangle dest = {x, y, DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE};
	Vector2 origin = {0.0f,0.0f};

	DrawTexturePro(loadedSheet.spriteSheetTexture, src, dest, origin, 0.0f, WHITE);
}


void fDrawBegin(){
	BeginTextureMode(renderTexture);
    BeginMode2D(cam);
}

void fDrawEnd(){
	EndMode2D();
    EndTextureMode();
    
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle r = { 0, 0, (float)(renderTexture.texture.width), (float)(-renderTexture.texture.height) };
    Rectangle r2 = { renderTextureOffset, 0, (float)(GetScreenWidth()) * scalingFactor, (float)(GetScreenHeight()) };
    Vector2 v = {0, 0};
    DrawTexturePro(renderTexture.texture,r,r2,v,0,WHITE);

    EndDrawing();
}


//------------------------------------------------------
// init
//------------------------------------------------------
void initFramework(){
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME);
	SetTargetFPS(60);
	renderTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	loadedSheet = initSpriteSheet();
	scalingFactor = SCREEN_WIDTH /(float)(GetScreenWidth());
	renderTextureOffset = ((GetScreenWidth()) / 2) - (SCREEN_WIDTH / 2);
	ToggleFullscreen();
	cam.zoom = DEFAULT_CAMERA_ZOOM;
}

//------------------------------------------------------
// dispose
//------------------------------------------------------
void disposeFramework(){
	CloseWindow();
	unloadSpriteSheet(loadedSheet);
	UnloadRenderTexture(renderTexture);
}


#endif
