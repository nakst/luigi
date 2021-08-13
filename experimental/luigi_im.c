// TODO UIColorPicker, UIExpandPane, UIImageDisplay, UISplitPane, UITabPane, UITable, UICode.
// TODO Syncing element flags: UI_ELEMENT_DISABLED, UI_ELEMENT_HIDE.

#define UI_IMPLEMENTATION
#define UI_WINDOWS
#include "luigi.h"

#include <stdio.h>

///////////////////

UIElement *enumerateSource;
bool enumerateQueued;
UIElement *stack[32];
int stackPosition;

void EnumerateUI();

UIElement *FindByID(uintptr_t id) {
	while (true) {
		if (!stack[stackPosition]) {
			return NULL;
		}

		UI_ASSERT(stackPosition != sizeof(stack) / sizeof(stack[0]));
		UIElement *element = stack[stackPosition];
		stack[stackPosition] = element->next;

		if (element->cp == (void *) id) {
			return element;
		} else {
			UIElementDestroy(element);
		}
	}
}

UIPanel *Panel(uintptr_t id, uint32_t flags) {
	UIElement *element = FindByID(id);
	UIPanel *panel = element ? (UIPanel *) element : UIPanelCreate(0, flags);
	panel->e.cp = (void *) id;
	stack[++stackPosition] = panel->e.children;
	UIParentPush(&panel->e);

	if (!element) {
		UIElementRefresh(panel->e.parent);
	}

	return panel;
}

int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_CLICKED) {
		enumerateSource = element;
		enumerateQueued = true;
	}

	return 0;
}

bool Button(uintptr_t id, uint32_t flags, const char *label) {
	size_t labelBytes = strlen(label);
	UIButton *element = (UIButton *) FindByID(id);

	if (element && (element->labelBytes != labelBytes || memcmp(element->label, label, labelBytes))) {
		UIElementDestroy(&element->e);
		element = NULL;
	}

	UIButton *button = element ? element : UIButtonCreate(0, flags, label, labelBytes);
	button->e.cp = (void *) id;
	button->e.messageUser = ButtonMessage;

	if (!element) {
		UIElementRefresh(button->e.parent);
	}

	return button == (UIButton *) enumerateSource;
}

void Label(uintptr_t id, uint32_t flags, const char *string) {
	size_t stringBytes = strlen(string);
	UIElement *element = FindByID(id);
	UILabel *label = element ? (UILabel *) element : UILabelCreate(0, flags, 0, 0);
	label->e.cp = (void *) id;

	if (label->labelBytes != stringBytes || memcmp(label->label, string, stringBytes)) {
		UILabelSetContent(label, string, stringBytes);
		UIElementRefresh(&label->e);
		UIElementRefresh(label->e.parent);
	}

	if (!element) {
		UIElementRefresh(label->e.parent);
	}
}

void Spacer(uintptr_t id, uint32_t flags, int width, int height) {
	UIElement *element = FindByID(id);
	UISpacer *spacer = element ? (UISpacer *) element : UISpacerCreate(0, flags, 0, 0);
	spacer->e.cp = (void *) id;

	if (!element || spacer->width != width || spacer->height != height) {
		spacer->width = width;
		spacer->height = height;
		UIElementRefresh(spacer->e.parent);
	}
}

void Gauge(uintptr_t id, uint32_t flags, float position) {
	UIElement *element = FindByID(id);
	UIGauge *gauge = element ? (UIGauge *) element : UIGaugeCreate(0, flags);
	gauge->e.cp = (void *) id;

	if (gauge->position != position) {
		gauge->position = position;
		UIElementRefresh(&gauge->e);
	}

	if (!element) {
		UIElementRefresh(gauge->e.parent);
	}
}

int ValueMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		enumerateSource = element;
		enumerateQueued = true;
	}

	return 0;
}

float Slider(uintptr_t id, uint32_t flags, float position, int steps) {
	UIElement *element = FindByID(id);
	UISlider *slider = element ? (UISlider *) element : UISliderCreate(0, flags);
	slider->e.cp = (void *) id;
	slider->e.messageUser = ValueMessage;
	slider->steps = steps;

	if (slider->position != position && &slider->e != enumerateSource) {
		slider->position = position;
		UIElementRefresh(&slider->e);
	}

	if (!element) {
		UIElementRefresh(slider->e.parent);
	}

	return slider->position;
}

void Textbox(uintptr_t id, uint32_t flags, char *buffer, size_t bufferSpace) {
	size_t bytes = strlen(buffer);
	UIElement *element = FindByID(id);
	UITextbox *textbox = element ? (UITextbox *) element : UITextboxCreate(0, flags);
	textbox->e.cp = (void *) id;
	textbox->e.messageUser = ValueMessage;

	if ((textbox->bytes != bytes || memcmp(textbox->string, buffer, bytes)) && &textbox->e != enumerateSource) {
		UITextboxClear(textbox, false);
		UITextboxReplace(textbox, buffer, -1, false);
		UIElementRefresh(&textbox->e);
	}

	if (&textbox->e == enumerateSource) {
		bytes = bufferSpace - 1;
		if (textbox->bytes < bytes) bytes = textbox->bytes;
		memcpy(buffer, textbox->string, bytes);
		buffer[bytes] = 0;
	}

	if (!element) {
		UIElementRefresh(textbox->e.parent);
	}
}

void Pop() {
	while (stack[stackPosition]) {
		UIElement *element = stack[stackPosition];
		stack[stackPosition] = element->next;
		UIElementDestroy(element);
	}

	stackPosition--;
	UIParentPop();
}

int WindowMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED && enumerateQueued) {
		UIParentPush(element);
		stack[0] = element->children;
		EnumerateUI();
		UIParentPop();
		enumerateQueued = false;
		enumerateSource = NULL;
		UI_ASSERT(stackPosition == 0);
	}

	return 0;
}

///////////////////

int counter;

void EnumerateUI() {
	Panel(1, UI_PANEL_GRAY | UI_PANEL_MEDIUM_SPACING);
		if (Button(1, 0, "Increment counter")) {
			counter++;
		}

		if (Button(2, 0, "Decrement counter")) {
			counter--;
		}

		Spacer(5, 0, 10, 10);
		
		{
			char buffer[16];
			snprintf(buffer, 16, "Counter: %d", counter);
			Label(3, 0, buffer);
			Gauge(4, 0, counter / 10.0f);
			counter = 10.0f * Slider(5, 0, counter / 10.0f, 0);
		}

		Spacer(5, 0, 10, 10);
		
		{
			static char buffer[64];
			Textbox(6, 0, buffer, sizeof(buffer));
			Label(7, 0, buffer);
		}
	Pop();
}

///////////////////

int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand) {
	UIInitialise();
	ui.theme = _uiThemeClassic;
	UIWindow *window = UIWindowCreate(0, UI_ELEMENT_PARENT_PUSH, "Luigi Immediate Mode", 0, 0);
	window->e.messageUser = WindowMessage;
	EnumerateUI();
	UIParentPop();
	return UIMessageLoop();
}
