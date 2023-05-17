#include "raylib.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(...) (void)0
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "raylib-nuklear.h"
#include "fragment.h"
#include "types.h"
#include "client.h"
#include <iostream>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define PANEL_SIZE 38

nk_context *ctx = NULL;
enum View {
	VIEW_NONE = 0,
	VIEW_APP,
	VIEW_PROCESS,
	VIEW_DIRECTORY,
	VIEW_SETTINGS,
};

View current_view = VIEW_NONE;

std::string current_dir = "";
std::vector<ProcessInfo> processes;

Texture2D screen_texture = {0};
Image screen_image = {0};

int mouse_x, mouse_y, mouse_width, mouse_height;
Texture2D mouse_texture = {0};

Camera2D camera = {0};
Shader shader = {0};

#define PROCESS_FETCH_INTERVAL 5
double last_processes_get_time = -PROCESS_FETCH_INTERVAL; //seconds

void PanelView(nk_context *ctx) {
	if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, GetScreenWidth(), PANEL_SIZE), NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 30, 4);
		if (nk_button_label(ctx, "Applications")) {
			current_view = VIEW_APP;
		}
		if (nk_button_label(ctx, "Procceses")) {
			current_view = VIEW_PROCESS;
		}
		if (nk_button_label(ctx, "Files")) {
			current_view = VIEW_DIRECTORY;
		}
		if (nk_button_label(ctx, "Settings")) {
			current_view = VIEW_SETTINGS;
		}
	}
	nk_end(ctx);
}

int view_begin(const char *name) {
	const int pad_x = 50;
	const int pad_y = 50;
	return nk_begin(ctx, name,
		nk_rect(pad_x, pad_y, ScreenSocketGetWidth() / 2 - pad_x * 2, ScreenSocketGetHeight() / 2 - pad_y * 2),
		NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE);
}

void ProcessesView(nk_context *ctx, char type) {
	const int pid_size = 50;
	const int button_size = 100;
	const char *window_name[] = {"Procceses", "Applications"};
	if (view_begin(window_name[type])) {
		//Header
		nk_layout_row_template_begin(ctx, 30);
		nk_layout_row_template_push_static(ctx, pid_size);
		nk_layout_row_template_push_dynamic(ctx);
		nk_layout_row_template_push_static(ctx, button_size * 3);
		nk_layout_row_template_end(ctx);

		nk_label(ctx, "PID", NK_TEXT_CENTERED);
		nk_label(ctx, "Executable", NK_TEXT_LEFT);
		nk_label(ctx, "Action", NK_TEXT_LEFT);

		//Content
		nk_layout_row_template_begin(ctx, 30);
		nk_layout_row_template_push_static(ctx, pid_size); //PID fixed width of 50
		nk_layout_row_template_push_dynamic(ctx); //Name can grow or shrink
		nk_layout_row_template_push_static(ctx, button_size); //Button
		nk_layout_row_template_push_static(ctx, button_size); //Button
		nk_layout_row_template_push_static(ctx, button_size); //Button
		nk_layout_row_template_end(ctx);
		for (auto &process: processes) {
			if (process.type == type) {
				nk_label(ctx, TextFormat("%d", process.pid), NK_TEXT_CENTERED);
				nk_label(ctx, TextFormat("%s", process.name.c_str()), NK_TEXT_LEFT);
				if (nk_button_symbol_label(ctx, NK_SYMBOL_RECT_SOLID, "Suspend", NK_TEXT_RIGHT)) {
					ControlSocketSendData({PROCESS_SUSPEND, process.pid}, NULL);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
				if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, "Resume", NK_TEXT_RIGHT)) {
					ControlSocketSendData({PROCESS_RESUME, process.pid}, NULL);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
				if (nk_button_symbol_label(ctx, NK_SYMBOL_X, "Terminate", NK_TEXT_RIGHT)) {
					ControlSocketSendData({PROCESS_KILL, process.pid}, NULL);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
			}
		}
	} else current_view = VIEW_NONE;
	nk_end(ctx);
}

void UpdateFrame() {
	UpdateNuklear(ctx);
	PanelView(ctx);

	double time = GetTime();
	if (time - last_processes_get_time >= PROCESS_FETCH_INTERVAL) { //5 seconds
		if (current_view == VIEW_APP || current_view == VIEW_PROCESS) {
			processes = ControlSocketGetProcesses();
			last_processes_get_time = time;
		}
	}
	switch (current_view) {
		case VIEW_APP:
			ProcessesView(ctx, 1);
			break;
		case VIEW_PROCESS:
			ProcessesView(ctx, 0);
			break;
	}

	//Get mouse location and whether mouse image has changed
	if (ScreenSocketGetMouseInfo(&mouse_x, &mouse_y)) {
		//Mouse image changed
		unsigned char *mouse_data = ScreenSocketGetMouse(&mouse_width, &mouse_height);
		if (mouse_width != mouse_texture.width || mouse_height != mouse_texture.height) {
			//Size changed, create new texture
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

	if (IsWindowResized()) {
		//Calculate new zoom so the screen view is contained and centered by the window
		if ((GetScreenWidth()) / (float) (GetScreenHeight() - PANEL_SIZE) < screen_image.width / (float)screen_image.height)
			camera.zoom = (float) (GetScreenWidth()) / screen_image.width;
		else
			camera.zoom = (float) (GetScreenHeight() - PANEL_SIZE) / screen_image.height;
		camera.offset.x = ((GetScreenWidth()) - camera.zoom * screen_image.width) / 2;
		camera.offset.y = ((GetScreenHeight() - PANEL_SIZE) - camera.zoom * screen_image.height) / 2 + PANEL_SIZE;
	}

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
	//Socket connect
	ScreenSocketConnect("192.168.56.1");
	ControlSocketConnect("192.168.56.1");

	//Raylib Window Creation
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3ChangDev");
	InitAudioDevice();
	SetExitKey(0); //Prevent app from closing when pressing ESC key

	//Nuklear GUI Init
	ctx = InitNuklear(10);

	//Screen texture Init
	screen_image = GenImageColor(ScreenSocketGetWidth(), ScreenSocketGetHeight(), BLANK);
	screen_texture = LoadTextureFromImage(screen_image);
	SetTextureFilter(screen_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(screen_texture, TEXTURE_WRAP_CLAMP);

	SetWindowMinSize(ScreenSocketGetWidth() / 2, ScreenSocketGetHeight() / 2 + PANEL_SIZE);
	SetWindowSize(ScreenSocketGetWidth() / 2, ScreenSocketGetHeight() / 2 + PANEL_SIZE);

	//Mouse texture init
	Image mouse_image = GenImageColor(32, 32, BLANK);
	mouse_texture = LoadTextureFromImage(mouse_image);
	UnloadImage(mouse_image);
	SetTextureFilter(mouse_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(mouse_texture, TEXTURE_WRAP_CLAMP);

	//Shader to flip RGB channel
	shader = LoadShaderFromMemory(NULL, fragment_shader);

	while (!WindowShouldClose()) {
		UpdateFrame();
	}

	//Free resources
	UnloadNuklear(ctx);
	UnloadTexture(screen_texture);
	UnloadImage(screen_image);
	//ScreenSocketClose();
	//ControlSocketClose();
	CloseAudioDevice();
	CloseWindow();
	return 0;
}