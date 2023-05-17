#include <windows.h>

void mouse_move(float x, float y, int width, int height) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dx = (int) (x / (float)width * 65535);
	input.mi.dy = (int) (y / (float)height * 65535);
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_left_down() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_left_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_middle_down() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_middle_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_right_down() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_right_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_wheel_v(int delta) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.mouseData = delta;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;

	SendInput(1, &input, sizeof(INPUT));
}

void mouse_wheel_h(int delta) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.mouseData = delta;
	input.mi.dwFlags = MOUSEEVENTF_HWHEEL;

	SendInput(1, &input, sizeof(INPUT));
}