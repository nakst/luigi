// Compile with: clang++ luigi_sdl2.cpp -lsdl2 -lopengl32 -o luigi_sdl.exe -O2 && luigi_sdl2.exe

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/gl.h>
#include "SDL2/SDL.h"

// TODO Menu support.

#define UI_ASSERT assert
#define UI_CALLOC(x) calloc(1, (x))
#define UI_FREE free
#define UI_MALLOC malloc
#define UI_REALLOC realloc
#define UI_CLOCK SDL_GetTicks
#define UI_CLOCKS_PER_SECOND (1000)
#define UI_CLOCK_T uint32_t

#define UI_IMPLEMENTATION
#include "luigi.h"

const int UI_KEYCODE_A = SDL_SCANCODE_A;
const int UI_KEYCODE_BACKSPACE = SDL_SCANCODE_BACKSPACE;
const int UI_KEYCODE_DELETE = SDL_SCANCODE_DELETE;
const int UI_KEYCODE_DOWN = SDL_SCANCODE_DOWN;
const int UI_KEYCODE_END = SDL_SCANCODE_END;
const int UI_KEYCODE_ENTER = SDL_SCANCODE_RETURN;
const int UI_KEYCODE_ESCAPE = SDL_SCANCODE_ESCAPE;
const int UI_KEYCODE_F1 = SDL_SCANCODE_F1;
const int UI_KEYCODE_F10 = SDL_SCANCODE_F10;
const int UI_KEYCODE_F11 = SDL_SCANCODE_F11;
const int UI_KEYCODE_F12 = SDL_SCANCODE_F12;
const int UI_KEYCODE_F2 = SDL_SCANCODE_F2;
const int UI_KEYCODE_F3 = SDL_SCANCODE_F3;
const int UI_KEYCODE_F4 = SDL_SCANCODE_F4;
const int UI_KEYCODE_F5 = SDL_SCANCODE_F5;
const int UI_KEYCODE_F6 = SDL_SCANCODE_F6;
const int UI_KEYCODE_F7 = SDL_SCANCODE_F7;
const int UI_KEYCODE_F8 = SDL_SCANCODE_F8;
const int UI_KEYCODE_F9 = SDL_SCANCODE_F9;
const int UI_KEYCODE_HOME = SDL_SCANCODE_HOME;
const int UI_KEYCODE_LEFT = SDL_SCANCODE_LEFT;
const int UI_KEYCODE_RIGHT = SDL_SCANCODE_RIGHT;
const int UI_KEYCODE_SPACE = SDL_SCANCODE_SPACE;
const int UI_KEYCODE_TAB = SDL_SCANCODE_TAB;
const int UI_KEYCODE_UP = SDL_SCANCODE_UP;

SDL_Cursor *cursors[UI_CURSOR_COUNT];

UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int width, int height) { 
	return NULL; 
}

void _UIWindowGetScreenPosition(UIWindow *window, int *x, int *y) { 
	*x = *y = 0; 
}

void _UIWindowEndPaint(UIWindow *window, UIPainter *painter) {
}

void _UIWindowSetCursor(UIWindow *window, int cursor) {
	SDL_SetCursor(cursors[cursor]);
}

int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_LAYOUT && element->children) {
		UIElementMove(element->children, element->bounds, false);
		UIElementRepaint(element, NULL);
	} else if (message == UI_MSG_DESTROY) {
		UIWindow *window = (UIWindow *) element;
		_UIWindowDestroyCommon(window);
		exit(0);
	}

	return 0;
}

void _UIWindowUpdateSize(UIWindow *window, SDL_Window *sdlWindow) {
	SDL_GetWindowSize(sdlWindow, &window->width, &window->height);
	glViewport(0, 0, window->width, window->height);
	window->bits = (uint32_t *) UI_REALLOC(window->bits, window->width * window->height * 4);
	window->e.bounds = UI_RECT_2S(window->width, window->height);
	window->e.clip = UI_RECT_2S(window->width, window->height);
	UIElementMessage(&window->e, UI_MSG_LAYOUT, 0, 0);
	_UIUpdate();
}

void UISendSDLEvent(UIWindow *window, SDL_Event *event, SDL_Window *sdlWindow) {
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
		_UIWindowUpdateSize(window, sdlWindow);
	} else if (event->type == SDL_MOUSEMOTION) {
		window->cursorX = event->motion.x;
		window->cursorY = event->motion.y;
		_UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
	} else if (event->type == SDL_MOUSEWHEEL) {
		_UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, -50 * event->wheel.y, 0);
	} else if (event->type == SDL_MOUSEBUTTONDOWN) {
		_UIWindowInputEvent(window, event->button.button == SDL_BUTTON_LEFT ? UI_MSG_LEFT_DOWN 
			: event->button.button == SDL_BUTTON_MIDDLE ? UI_MSG_MIDDLE_DOWN 
			: UI_MSG_RIGHT_DOWN, 0, 0);
	} else if (event->type == SDL_MOUSEBUTTONUP) {
		_UIWindowInputEvent(window, event->button.button == SDL_BUTTON_LEFT ? UI_MSG_LEFT_UP 
			: event->button.button == SDL_BUTTON_MIDDLE ? UI_MSG_MIDDLE_UP 
			: UI_MSG_RIGHT_UP, 0, 0);
	} else if (event->type == SDL_KEYDOWN) {
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		window->ctrl = state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];
		window->shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];
		window->alt = state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT];
		UIKeyTyped m = { 0 };
		m.code = event->key.keysym.scancode;
		_UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
	} else if (event->type == SDL_TEXTINPUT) {
		UIKeyTyped m = { 0 };
		m.text = event->text.text;
		m.textBytes = strlen(m.text);
		_UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
	}
}

UIWindow *UIInitialiseSDL() {
	ui.theme = _uiThemeDark;

	cursors[UI_CURSOR_ARROW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	cursors[UI_CURSOR_TEXT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	cursors[UI_CURSOR_SPLIT_V] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[UI_CURSOR_SPLIT_H] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[UI_CURSOR_FLIPPED_ARROW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	cursors[UI_CURSOR_CROSS_HAIR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	cursors[UI_CURSOR_HAND] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	cursors[UI_CURSOR_RESIZE_V] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[UI_CURSOR_RESIZE_H] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[UI_CURSOR_RESIZE_NESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[UI_CURSOR_RESIZE_NWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	UIWindow *window = (UIWindow *) UIElementCreate(sizeof(UIWindow), NULL, UI_ELEMENT_WINDOW, _UIWindowMessage, "Window");
	window->scale = 1.0f;
	window->e.window = window;
	window->hovered = &window->e;
	window->next = ui.windows;
	ui.windows = window;

	return window;
}

#ifdef _WIN32
int WinMain(HINSTANCE instance, HINSTANCE previousInstance, char *commandLine, int showCommand) {
#else
int main(int argc, char **argv) {
#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window *sdlWindow = SDL_CreateWindow("SDL Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
	SDL_GL_MakeCurrent(sdlWindow, glContext);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);

	UIWindow *window = UIInitialiseSDL();
	UIMDIClient *client = UIMDIClientCreate(&window->e, UI_MDI_CLIENT_TRANSPARENT);
	UIMDIChild *child = UIMDIChildCreate(&client->e, 0, UI_RECT_4(10, 300, 10, 300), "Test Window", -1);

	UIPanel *panel = UIPanelCreate(&child->e, UI_PANEL_GRAY | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING | UI_PANEL_SCROLL | UI_ELEMENT_PARENT_PUSH);
		UILabelCreate(0, 0, "Vertex 1", -1);
		UIPanelCreate(0, UI_PANEL_HORIZONTAL | UI_PANEL_SMALL_SPACING | UI_ELEMENT_PARENT_PUSH);
			UISlider *sliderX1 = UISliderCreate(0, UI_ELEMENT_H_FILL);
			sliderX1->position = 0.5f;
			UISlider *sliderY1 = UISliderCreate(0, UI_ELEMENT_H_FILL);
			sliderY1->position = 1.0f;
		UIParentPop();
		UILabelCreate(0, 0, "Vertex 2", -1);
		UIPanelCreate(0, UI_PANEL_HORIZONTAL | UI_PANEL_SMALL_SPACING | UI_ELEMENT_PARENT_PUSH);
			UISlider *sliderX2 = UISliderCreate(0, UI_ELEMENT_H_FILL);
			UISlider *sliderY2 = UISliderCreate(0, UI_ELEMENT_H_FILL);
		UIParentPop();
		UILabelCreate(0, 0, "Vertex 3", -1);
		UIPanelCreate(0, UI_PANEL_HORIZONTAL | UI_PANEL_SMALL_SPACING | UI_ELEMENT_PARENT_PUSH);
			UISlider *sliderX3 = UISliderCreate(0, UI_ELEMENT_H_FILL);
			sliderX3->position = 1.0f;
			UISlider *sliderY3 = UISliderCreate(0, UI_ELEMENT_H_FILL);
		UIParentPop();
		UILabelCreate(0, 0, "Color", -1);
		UIColorPicker *colorPicker = UIColorPickerCreate(&UIPanelCreate(0, 0)->e, 0);
		colorPicker->value = 1;
	UIParentPop();

	_UIWindowUpdateSize(window, sdlWindow);

	bool running = true;

	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}	

			UISendSDLEvent(window, &event, sdlWindow);
		}

		if (ui.animating) {
			_UIProcessAnimations();
		}

		glClear(GL_COLOR_BUFFER_BIT);

		uint32_t selectedColor;
		UIColorToRGB(colorPicker->hue, colorPicker->saturation, colorPicker->value, &selectedColor);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
		glColor4f(UI_COLOR_RED_F(selectedColor), UI_COLOR_GREEN_F(selectedColor), UI_COLOR_BLUE_F(selectedColor), 1);
		glVertex2f(sliderX1->position * 2 - 1, sliderY1->position * 2 - 1);
		glVertex2f(sliderX2->position * 2 - 1, sliderY2->position * 2 - 1);
		glVertex2f(sliderX3->position * 2 - 1, sliderY3->position * 2 - 1);
		glEnd();

		GLuint texture;
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window->width, window->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, window->bits);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBegin(GL_TRIANGLES);
		glColor4f(1, 1, 1, 1);
		glTexCoord2f(0, 1); glVertex2f(-1, -1);
		glTexCoord2f(1, 1); glVertex2f(1, -1);
		glTexCoord2f(0, 0); glVertex2f(-1, 1);
		glTexCoord2f(1, 1); glVertex2f(1, -1);
		glTexCoord2f(0, 0); glVertex2f(-1, 1);
		glTexCoord2f(1, 0); glVertex2f(1, 1);
		glEnd();
		glDeleteTextures(1, &texture);

		SDL_GL_SwapWindow(sdlWindow);
	}
	
	return 0;
}
