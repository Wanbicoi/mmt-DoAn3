#include "raylib.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(...) (void)0
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "raylib-nuklear.h"
#include "fragment.h"

#include <iostream>

#include "define.h"
#include "client.h"

enum TabView {
	WELCOME = 0,
	APPLICATIONS,
	PROCESSES,
	VIEW,
	DIRECTORY,
	SETTINGS
};

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define PANEL_SIZE 200

nk_context *ctx = NULL;
TabView current_tab = VIEW;

Texture2D screen_texture = {0};
Image screen_image = {0};

int mouse_x, mouse_y, mouse_width, mouse_height;
Texture2D mouse_texture = {0};

Camera2D camera = {0};
Shader shader = {0};
std::vector<std::tuple<std::string, int, char>> processes;

//processes = ControlSocketGetProcesses();

void PanelView() {
	if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, PANEL_SIZE, GetScreenHeight()), 0)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Applications")) {
			//current_tab = APPLICATIONS;
		}
		if (nk_button_label(ctx, "Procceses")) {
			//current_tab = PROCESSES;
		}
		if (nk_button_label(ctx, "Live View")) {
			current_tab = VIEW;
		}
		if (nk_button_label(ctx, "Files")) {
			//current_tab = DIRECTORY;
		}
		if (nk_button_label(ctx, "Settings")) {
			//current_tab = SETTINGS;
		}
	}
	nk_end(ctx);
}

void TabView() {
	UpdateNuklear(ctx);
	PanelView();
	if (ScreenSocketGetMouseInfo(mouse_x, mouse_y)) {
		unsigned char *mouse_data = ScreenSocketGetMouse(mouse_width, mouse_height);
		if (mouse_width != mouse_texture.width || mouse_height != mouse_texture.height) {
			UnloadTexture(mouse_texture);
			Image mouse_image = GenImageColor(mouse_width, mouse_height, BLANK);
			mouse_texture = LoadTextureFromImage(mouse_image);
			UnloadImage(mouse_image);
			SetTextureFilter(mouse_texture, TEXTURE_FILTER_BILINEAR);
			SetTextureWrap(mouse_texture, TEXTURE_WRAP_CLAMP);
		}
		UpdateTexture(mouse_texture, mouse_data);
		free(mouse_data);
	}
	ScreenSocketGetScreen(screen_image.data);
	UpdateTexture(screen_texture, screen_image.data);

	if ((GetScreenWidth() - PANEL_SIZE) / (float) GetScreenHeight() < screen_image.width / (float)screen_image.height)
		camera.zoom = (float) (GetScreenWidth() - PANEL_SIZE) / screen_image.width;
	else
		camera.zoom = (float) GetScreenHeight() / screen_image.height;
	camera.offset.x = ((GetScreenWidth() - PANEL_SIZE) - camera.zoom * screen_image.width) / 2 + PANEL_SIZE;
	camera.offset.y = (GetScreenHeight() - camera.zoom * screen_image.height) / 2;

	BeginDrawing();
		ClearBackground(GRAY);
		BeginMode2D(camera);
			BeginShaderMode(shader);
				DrawTexture(screen_texture, 0, 0, WHITE);
				DrawTexture(mouse_texture, mouse_x, mouse_y, WHITE);
			EndShaderMode();
		EndMode2D();
		DrawNuklear(ctx);
		DrawFPS(10, GetScreenHeight() - 20);
	EndDrawing();
}

int main(void) {
	ScreenSocketConnect("192.168.56.1");
	ControlSocketConnect("192.168.56.1");
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH + PANEL_SIZE, SCREEN_HEIGHT, "3ChangDev");
	InitAudioDevice();
	ctx = InitNuklear(10);
	screen_image = GenImageColor(ScreenSocketGetWidth(), ScreenSocketGetHeight(), BLANK);
	screen_texture = LoadTextureFromImage(screen_image);
	SetTextureFilter(screen_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(screen_texture, TEXTURE_WRAP_CLAMP);

	Image mouse_image = GenImageColor(32, 32, BLANK);
	mouse_texture = LoadTextureFromImage(mouse_image);
	UnloadImage(mouse_image);

	shader = LoadShaderFromMemory(NULL, fragment_shader);
	while (!WindowShouldClose()) {
		switch (current_tab) {
			case WELCOME:
				break;
			case APPLICATIONS:
				break;
			case PROCESSES:
				break;
			case VIEW:
				TabView();
				break;
			case DIRECTORY:
				break;
			case SETTINGS:
				break;
			default:
				break;
		}
	}

	UnloadNuklear(ctx);
	UnloadTexture(screen_texture);
	UnloadImage(screen_image);
	//ScreenSocketClose();
	//ControlSocketClose();
	CloseAudioDevice();
	CloseWindow();
	return 0;
}