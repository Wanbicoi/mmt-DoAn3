#include <windows.h>

int mouse_move(float x, float y, int width, int height) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dx = (int) (x / (float)width * 65535);
	input.mi.dy = (int) (y / (float)height * 65535);
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_left_down(float x, float y, int width, int height) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dx = (int) (x / (float)width * 65535);
	input.mi.dy = (int) (y / (float)height * 65535);
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_left_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_middle_down() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_middle_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_right_down() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_right_up() {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_wheel_v(int delta) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.mouseData = delta;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;

	return SendInput(1, &input, sizeof(INPUT));
}

int mouse_wheel_h(int delta) {
	INPUT input = {0};

	input.type = INPUT_MOUSE;
	input.mi.mouseData = delta;
	input.mi.dwFlags = MOUSEEVENTF_HWHEEL;

	return SendInput(1, &input, sizeof(INPUT));
}