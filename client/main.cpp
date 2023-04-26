#include "raylib.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(...) (void)0
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "raylib-nuklear.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

nk_context *ctx;

void UpdateFrame() {
	UpdateNuklear(ctx);

	if (nk_begin(ctx, "Nuklear", nk_rect(100, 100, 220, 220), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Button")) {
			//Clicked
		}
	}
	nk_end(ctx);

	BeginDrawing();
		ClearBackground(GRAY);
		DrawText("Congrats! You created your first window!", 190, 360, 40, LIGHTGRAY);
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

	while (!WindowShouldClose()) {
		UpdateFrame();
	}

	UnloadNuklear(ctx);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}