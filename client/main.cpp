#include "raylib.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(...) (void)0
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "raylib-nuklear.h"
#include "fragment.h"

#include "client.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

nk_context *ctx = NULL;
Texture2D screen_texture = {0};
Image screen_image = {0};
size_t screen_data_size = 0;
Camera2D camera = {0};
Shader shader = {0};

void UpdateFrame() {
	UpdateNuklear(ctx);
	if (nk_begin(ctx, "Nuklear", nk_rect(100, 100, 220, 220), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Button")) {
			//Clicked
		}
	}
	nk_end(ctx);
	ScreenSocketGetData(screen_image.data, screen_data_size);
	UpdateTexture(screen_texture, screen_image.data);
	camera.zoom = (float) GetScreenWidth() / screen_image.width;
	BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);
			BeginShaderMode(shader);
				DrawTexture(screen_texture, 0, 0, WHITE);
			EndShaderMode();
		EndMode2D();
		DrawNuklear(ctx);
		DrawFPS(10, 10);
	EndDrawing();
}

int main(void) {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3ChangDev");
	InitAudioDevice();
	ctx = InitNuklear(10);
	ScreenSocketConnect("192.168.56.1");
	screen_image = GenImageColor(ScreenSocketGetWidth(), ScreenSocketGetHeight(), BLANK);
	screen_data_size = screen_image.width * screen_image.height * 4;
	screen_texture = LoadTextureFromImage(screen_image);
	shader = LoadShaderFromMemory(NULL, fragment_shader);
	while (!WindowShouldClose()) {
		UpdateFrame();
	}

	UnloadNuklear(ctx);
	UnloadTexture(screen_texture);
	UnloadImage(screen_image);
	ScreenSocketClose();
	CloseAudioDevice();
	CloseWindow();
	return 0;
}