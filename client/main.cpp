#include "raylib.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(...) (void)0
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "raylib-nuklear.h"
#include "fragment.h"

#include "client.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

nk_context *ctx;
Texture2D screen_texture;
Image screen_image;
size_t screen_data_size;
Camera2D camera = {0};
Shader shader;

void UpdateFrame() {
	UpdateNuklear(ctx);

	if (nk_begin(ctx, "Nuklear", nk_rect(100, 100, 220, 220), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Button")) {
			//Clicked
		}
	}
	nk_end(ctx);
	get_image(screen_image.data, screen_data_size);
	UpdateTexture(screen_texture, screen_image.data);
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
	connect("192.168.56.1");
	screen_image = GenImageColor(get_client_width(), get_client_height(), BLANK);
	screen_data_size = screen_image.width * screen_image.height * 4;
	screen_texture = LoadTextureFromImage(screen_image);
	camera.zoom = 0.5;
	camera.offset = {-0.5, -0.5};
	shader = LoadShaderFromMemory(NULL, fragment_shader);
	while (!WindowShouldClose()) {
		UpdateFrame();
	}

	UnloadNuklear(ctx);
	UnloadTexture(screen_texture);
	UnloadImage(screen_image);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}