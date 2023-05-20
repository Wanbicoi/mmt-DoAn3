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
#include <thread>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

ScreenClient screen_client;
ControlClient control_client;

#define PANEL_SIZE 38
#define UI_LINE_HEIGHT 30

#define MOUSE_NONE -2
#define MOUSE_HOVER -1


nk_context *ctx = NULL;
int mouse_interacting_nuklear = MOUSE_NONE;
enum View {
	VIEW_NONE = 0,
	VIEW_APP,
	VIEW_PROCESS,
	VIEW_DIRECTORY,
	VIEW_SETTINGS,
};

struct nk_image pause_img, play_img, stop_img, file_img, folder_img;

View current_view = VIEW_NONE;

std::string current_dir = "";
std::vector<FileInfo> files_list;
std::vector<ProcessInfo> processes;
#define FILELIST_FETCH_INTERVAL 5
double last_files_get_time = -FILELIST_FETCH_INTERVAL; //seconds

Texture2D screen_texture = {0};
Image screen_image = {0};

int mouse_x, mouse_y, mouse_width, mouse_height;
Texture2D mouse_texture = {0};
bool mouse_was_down[3] = {0};

Camera2D camera = {0};
Shader shader = {0};

#define PROCESS_FETCH_INTERVAL 5
double last_processes_get_time = -PROCESS_FETCH_INTERVAL; //seconds

int view_begin(const char *name) {
	const int pad_x = 50;
	const int pad_y = 50;
	return nk_begin(ctx, name,
		nk_rect(pad_x, pad_y, screen_client.getWidth() / 2 - pad_x * 2, screen_client.getHeight() / 2 - pad_y * 2),
		NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE);
}

void ProcessesView(nk_context *ctx, char type) {
	double time = GetTime();
	if (time - last_processes_get_time >= PROCESS_FETCH_INTERVAL) { //5 seconds
		processes = control_client.getProcesses();
		last_processes_get_time = time;
	}

	const int pid_size = 60;
	const int button_size = 130;
	const char *window_name[] = {"Procceses", "Applications"};
	if (view_begin(window_name[type])) {
		//Header
		nk_layout_row_template_begin(ctx, UI_LINE_HEIGHT);
		nk_layout_row_template_push_static(ctx, pid_size);
		nk_layout_row_template_push_dynamic(ctx);
		nk_layout_row_template_push_static(ctx, button_size * 3);
		nk_layout_row_template_end(ctx);

		nk_label(ctx, "PID", NK_TEXT_CENTERED);
		nk_label(ctx, "Executable", NK_TEXT_LEFT);
		nk_label(ctx, "Action", NK_TEXT_LEFT);

		//Content
		nk_layout_row_template_begin(ctx, UI_LINE_HEIGHT);
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
				if (nk_button_image_label(ctx, pause_img, "Suspend", NK_TEXT_RIGHT)) {
					control_client.suspendProcess(process.pid);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
				if (nk_button_image_label(ctx, play_img, "Resume", NK_TEXT_RIGHT)) {
					control_client.resumeProcess(process.pid);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
				if (nk_button_image_label(ctx, stop_img, "Terminate", NK_TEXT_RIGHT)) {
					control_client.terminateProcess(process.pid);
					last_processes_get_time = -PROCESS_FETCH_INTERVAL; //So it will update
				}
			}
		}
	} else {
		current_view = VIEW_NONE;
	}
	nk_end(ctx);
}

void DirectoryView(nk_context *ctx) {
	if (current_dir == "") {
		current_dir = control_client.getDefaultLocation();
	}

	double time = GetTime();
	if (time - last_files_get_time >= FILELIST_FETCH_INTERVAL) { //5 seconds
		files_list = control_client.listDir(current_dir);
		last_files_get_time = time;
	}

	if (view_begin("Files")) {
		nk_layout_row_template_begin(ctx, UI_LINE_HEIGHT);
		nk_layout_row_template_push_static(ctx, UI_LINE_HEIGHT);
		nk_layout_row_template_push_dynamic(ctx);
		nk_layout_row_template_end(ctx);
		for (auto &file: files_list) {
			if (file.type == 1) {
				nk_image(ctx, folder_img);
			}
			else {
				nk_image(ctx, file_img);
			}
			int selectable;
			nk_selectable_label(ctx, file.name.c_str(), NK_TEXT_LEFT, &selectable);
		}
	} else {
		current_view = VIEW_NONE;
	}
	nk_end(ctx);
}

void NuklearView(nk_context *ctx) {
	UpdateNuklear(ctx);
	if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, GetScreenWidth(), PANEL_SIZE), NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, UI_LINE_HEIGHT, 4);
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

	switch (current_view) {
		case VIEW_APP:
			ProcessesView(ctx, 1);
			break;
		case VIEW_PROCESS:
			ProcessesView(ctx, 0);
			break;
		case VIEW_DIRECTORY:
			DirectoryView(ctx);
			break;
		case VIEW_SETTINGS:
			break;
	}
}

void UpdateFrame() {
	NuklearView(ctx);

	//Get mouse location and whether mouse image has changed
	if (screen_client.isFrameChanged()) {
		screen_client.getMouseInfo(&mouse_x, &mouse_y);
		if (screen_client.isMouseImgChanged()) {
			unsigned char *mouse_data = screen_client.getMouseData(&mouse_width, &mouse_height);
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
		}
		unsigned char *screen_data = screen_client.getScreenData();
		UpdateTexture(screen_texture, screen_data);
	}

	if (IsWindowResized()) {
		//Calculate new zoom so the screen view is contained and centered by the window
		if ((GetScreenWidth()) / (float) (GetScreenHeight() - PANEL_SIZE) < screen_texture.width / (float)screen_texture.height)
			camera.zoom = (float) (GetScreenWidth()) / screen_texture.width;
		else
			camera.zoom = (float) (GetScreenHeight() - PANEL_SIZE) / screen_texture.height;
		camera.offset.x = ((GetScreenWidth()) - camera.zoom * screen_texture.width) / 2;
		camera.offset.y = ((GetScreenHeight() - PANEL_SIZE) - camera.zoom * screen_texture.height) / 2 + PANEL_SIZE;
	}

	const int mouse_type[] = {MOUSE_LEFT_BUTTON, MOUSE_MIDDLE_BUTTON, MOUSE_RIGHT_BUTTON};
	const OperationCode mouse_op_down[] = {MOUSE_LEFT_DOWN, MOUSE_MIDDLE_DOWN, MOUSE_RIGHT_DOWN};
	const OperationCode mouse_op_up[] = {MOUSE_LEFT_UP, MOUSE_MIDDLE_UP, MOUSE_RIGHT_UP};

	if (nk_item_is_any_active(ctx)) { //Nuklear element is interacted/hovered
		//Interact
		for (int i = 0; i < 3; i++) {
			if (IsMouseButtonPressed(mouse_type[i]))
				mouse_interacting_nuklear = mouse_type[i];
		}
		//Hover
		if (mouse_interacting_nuklear == MOUSE_NONE)
			mouse_interacting_nuklear = MOUSE_HOVER;
	}
	else { //Outside nuklear, except for dragable widget interaction
		if (mouse_interacting_nuklear < 0 /*NONE and HOVER*/ ||  /*short circuiting*/ IsMouseButtonUp(mouse_interacting_nuklear)) {
			mouse_interacting_nuklear = MOUSE_NONE;
		}
	}

	//Mouse input
	Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
	if (mouse.x >= 0 && mouse.x <= screen_texture.width && mouse.y >= 0 && mouse.y <= screen_texture.height) { //Inside view bound
		if (mouse_interacting_nuklear == MOUSE_NONE) { //Mouse not occupied by GUI
			for (int i = 0; i < 3; i++) {
				if (IsMouseButtonPressed(mouse_type[i])) {
					std::cout << mouse.x << " | " << mouse.y << std::endl;
					MousePosition mp = {mouse.x, mouse.y, screen_texture.width, screen_texture.height};
					//control_client.sendControl(MOUSE_MOVE, sizeof(mp), &mp);
					//control_client.sendControl(mouse_op_down[i]);
					mouse_was_down[i] = 1;
				}
				if (mouse_was_down[i] && IsMouseButtonReleased(mouse_type[i])) {
					//control_client.sendControl(mouse_op_up[i]);
					mouse_was_down[i] = 0;
				}
			}
		}
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
		//DrawText(TextFormat("%d", mouse_interacting_nuklear), 10, GetScreenHeight() - 20, 20, GREEN);
	EndDrawing();
}

int main(void) {
	//Socket connect
	screen_client.connect("192.168.1.3");
	control_client.connect("192.168.1.3");

	screen_client.init();

	std::thread socket_thread(IoContextRun);

	//Raylib Window Creation
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3ChangDev");
	InitAudioDevice();
	SetExitKey(0); //Prevent app from closing when pressing ESC key

	//Nuklear GUI Init
	const int fontSize = 16;
	Font font = LoadFontEx("resource/Roboto-Medium.ttf", fontSize, NULL, 0);
	ctx = InitNuklearEx(font, fontSize);
	pause_img = LoadNuklearImage("resource/pause.png");
	play_img = LoadNuklearImage("resource/play.png");
	stop_img = LoadNuklearImage("resource/stop.png");
	file_img = LoadNuklearImage("resource/file.png");
	folder_img = LoadNuklearImage("resource/folder.png");

	//Screen texture Init
	screen_image = GenImageColor(screen_client.getWidth(), screen_client.getHeight(), BLANK);
	screen_texture = LoadTextureFromImage(screen_image);
	SetTextureFilter(screen_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureWrap(screen_texture, TEXTURE_WRAP_CLAMP);

	SetWindowMinSize(screen_texture.width / 2, screen_texture.height / 2 + PANEL_SIZE);
	SetWindowSize(screen_texture.width / 2, screen_texture.height / 2 + PANEL_SIZE);

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
	
	control_client.sendDisconnect();
	IoContextStop();
	socket_thread.detach();
	//Free resources
	UnloadNuklear(ctx);
	UnloadFont(font);
	UnloadNuklearImage(pause_img);
	UnloadNuklearImage(play_img);
	UnloadNuklearImage(stop_img);
	UnloadNuklearImage(file_img);
	UnloadNuklearImage(folder_img);
	UnloadTexture(screen_texture);
	UnloadImage(screen_image);
	UnloadTexture(mouse_texture);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}