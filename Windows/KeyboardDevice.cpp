#include "input/input_state.h"
#include "KeyboardDevice.h"
#include "../Common/CommonTypes.h"
#include "../Core/HLE/sceCtrl.h"
#include "WinUser.h"

static const unsigned int key_ctrl_map[] = {
	VK_TAB,   PAD_BUTTON_LEFT_THUMB,
	VK_RETURN, CTRL_START,
	VK_SPACE, CTRL_SELECT,
	'A',      CTRL_SQUARE,
	'W',      CTRL_TRIANGLE,
	'D',      CTRL_CIRCLE,
	'S',      CTRL_CROSS,
	'Q',      CTRL_LTRIGGER,
	'E',      CTRL_RTRIGGER,
	VK_UP,    CTRL_UP,
	VK_DOWN,  CTRL_DOWN,
	VK_LEFT,  CTRL_LEFT,
	VK_RIGHT, CTRL_RIGHT,
};

static const unsigned short analog_ctrl_map[] = {
	'I', CTRL_UP,
	'K', CTRL_DOWN,
	'J', CTRL_LEFT,
	'L', CTRL_RIGHT,
};

int KeyboardDevice::UpdateState(InputState &input_state) {
	bool alternate = GetAsyncKeyState(VK_SHIFT) != 0;
	static u32 alternator = 0;
	bool doAlternate = alternate && (alternator++ % 10) < 5;

	for (int i = 0; i < sizeof(key_pad_map)/sizeof(key_pad_map[0]); i += 2) {
		if (!GetAsyncKeyState(key_pad_map[i])) {
			continue;
		}
		if (!doAlternate || key_pad_map[i + 1] > PAD_BUTTON_SELECT) {
			input_state.pad_buttons |= key_pad_map[i+1];
		}
	}

	float analogX = 0;
	float analogY = 0;
	for (int i = 0; i < sizeof(analog_ctrl_map)/sizeof(analog_ctrl_map[0]); i += 2) {
		if (!GetAsyncKeyState(analog_ctrl_map[i])) {
			continue;
		}

		switch (analog_ctrl_map[i + 1]) {
		case CTRL_UP:
			analogY += 1.0f;
			break;
		case CTRL_DOWN:
			analogY -= 1.0f;
			break;
		case CTRL_LEFT:
			analogX -= 1.0f;
			break;
		case CTRL_RIGHT:
			analogX += 1.0f;
			break;
		}
	}

	input_state.pad_lstick_x += analogX;
	input_state.pad_lstick_y += analogY;
	return 0;
}