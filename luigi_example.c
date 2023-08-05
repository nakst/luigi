#include <stdio.h>

// #define UI_LINUX
// #define UI_WINDOWS
// #define UI_DEBUG
#define UI_IMPLEMENTATION
#include "luigi.h"

UIWindow *window;
UILabel *label;
UISlider *slider;
UIGauge *gauge;

const char *themeItems[] = {
	"panel1", 
	"panel2",
	"selected",
	"border",
	"text", 
	"textDisabled", 
	"textSelected", 
	"buttonNormal", 
	"buttonHovered", 
	"buttonPressed", 
	"buttonDisabled",
	"textboxNormal", 
	"textboxFocused", 
	"codeFocused", 
	"codeBackground", 
	"codeDefault", 
	"codeComment", 
	"codeString", 
	"codeNumber", 
	"codeOperator", 
	"codePreprocessor",
};

UIColorPicker *themeEditorColorPicker;
UITable *themeEditorTable;
int themeEditorSelectedColor = -1;

int ThemeEditorTableMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_TABLE_GET_ITEM) {
		UITableGetItem *m = (UITableGetItem *) dp;
		m->isSelected = themeEditorSelectedColor == m->index;

		if (m->column == 0) {
			return snprintf(m->buffer, m->bufferBytes, "%s", themeItems[m->index]);
		} else {
			return snprintf(m->buffer, m->bufferBytes, "#%.6x", ((uint32_t *) &ui.theme)[m->index]);
		}
	} else if (message == UI_MSG_CLICKED) {
		themeEditorSelectedColor = UITableHitTest((UITable *) element, element->window->cursorX, element->window->cursorY);
		UIColorToHSV(((uint32_t *) &ui.theme)[themeEditorSelectedColor], 
			&themeEditorColorPicker->hue, &themeEditorColorPicker->saturation, &themeEditorColorPicker->value);
		UIElementRepaint(&themeEditorColorPicker->e, NULL);
	}

	return 0;
}

int ThemeEditorColorPickerMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		if (themeEditorSelectedColor == -1) return 0;
		UIColorToRGB(themeEditorColorPicker->hue, themeEditorColorPicker->saturation, themeEditorColorPicker->value,
			&((uint32_t *) &ui.theme)[themeEditorSelectedColor]);
		UIElementRepaint(&window->e, NULL);
		UIElementRepaint(&element->window->e, NULL);
	}

	return 0;
}

int MyButtonMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_CLICKED) {
		printf("clicked!\n");
		UIElementDestroy(element);
		UIElementRefresh(element->parent);
	}

	return 0;
}

void MyMenuCallback(void *cp) {
	UILabelSetContent(label, (const char *) cp, -1);
	UIElementRefresh(&label->e);
}

int MyButton2Message(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_CLICKED) {
		UIMenu *menu = UIMenuCreate(element, 0);
		UIMenuAddItem(menu, 0, "Item 1\tCtrl+F5", -1, MyMenuCallback, (void *) "Item 1 clicked!");
		UIMenuAddItem(menu, 0, "Item 2\tF6", -1, MyMenuCallback, (void *) "Item 2 clicked!");
		UIMenuShow(menu);
	}

	return 0;
}

int MySliderMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		gauge->position = slider->position;
		UIElementRepaint(&gauge->e, NULL);
	}

	return 0;
}

int selected;

int MyTableMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_TABLE_GET_ITEM) {
		UITableGetItem *m = (UITableGetItem *) dp;
		m->isSelected = selected == m->index;

		if (m->column == 0) {
			return snprintf(m->buffer, m->bufferBytes, "Item %d", m->index);
		} else {
			return snprintf(m->buffer, m->bufferBytes, "other column %d", m->index);
		}
	} else if (message == UI_MSG_LEFT_DOWN) {
		int hit = UITableHitTest((UITable *) element, element->window->cursorX, element->window->cursorY);

		if (selected != hit) {
			selected = hit;

			if (!UITableEnsureVisible((UITable *) element, selected)) {
				UIElementRepaint(element, NULL);
			}
		}
	}

	return 0;
}

#ifdef UI_LINUX
int main(int argc, char **argv) {
#else
int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand) {
#endif

	UIInitialise();

	window = UIWindowCreate(0, 0, "Test Window", 0, 0);

	UISplitPane *split1 = UISplitPaneCreate(&window->e, UI_SPLIT_PANE_VERTICAL, 0.8f);

	UISplitPane *split3 = UISplitPaneCreate(&split1->e, 0, 0.3f);

	{
		UIPanel *panel = UIPanelCreate(&split3->e, UI_PANEL_GRAY);
		panel->border.t = 5;
		panel->gap = 5;
		UIButtonCreate(&panel->e, 0, "Hello", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&panel->e, 0, "World", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&panel->e, 0, "3", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&panel->e, 0, "4", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&panel->e, 0, "5", -1)->e.messageUser = MyButtonMessage;
		gauge = UIGaugeCreate(&panel->e, 0);
		gauge->position = 0.3f;
		slider = UISliderCreate(&panel->e, 0);
		slider->e.messageUser = MySliderMessage;
		slider->position = 0.3f;
		UITextboxCreate(&panel->e, 0);
	}

	{
		UICode *code = UICodeCreate(&split3->e, 0);
		char *buffer = (char *) malloc(262144);
		FILE *f = fopen("luigi.h", "rb");
		size_t size = fread(buffer, 1, 262144, f);
		fclose(f);
		UICodeInsertContent(code, buffer, size, true);
		UICodeFocusLine(code, 0);
	}

	UISplitPane *split2 = UISplitPaneCreate(&split1->e, 0, 0.3f);

	{
		UIPanel *panel = UIPanelCreate(&split2->e, UI_PANEL_WHITE);
		panel->border = UI_RECT_1(5);
		panel->gap = 5;
		UIButtonCreate(&panel->e, 0, "It's a button??", -1)->e.messageUser = MyButton2Message;
		label = UILabelCreate(&panel->e, UI_ELEMENT_H_FILL, "Hello, I am a label!", -1);
	}

	UITabPane *tabPane = UITabPaneCreate(&split2->e, 0, "Tab 1\tMiddle Tab\tTab 3");
	UITable *table = UITableCreate(&tabPane->e, 0, "Column 1\tColumn 2");
	table->itemCount = 100000;
	table->e.messageUser = MyTableMessage;
	UITableResizeColumns(table);
	UILabelCreate(&UIPanelCreate(&tabPane->e, UI_PANEL_GRAY)->e, 0, "you're in tab 2, bucko", -1);
	UILabelCreate(&UIPanelCreate(&tabPane->e, UI_PANEL_GRAY)->e, 0, "hiii!!!", -1);

	{
		UIWindow *window = UIWindowCreate(0, 0, "Theme Editor", 0, 0);
		UISplitPane *pane = UISplitPaneCreate(&window->e, 0, 0.5f);
		themeEditorColorPicker = UIColorPickerCreate(&UIPanelCreate(&pane->e, UI_PANEL_GRAY)->e, 0);
		themeEditorColorPicker->e.messageUser = ThemeEditorColorPickerMessage;
		themeEditorTable = UITableCreate(&pane->e, 0, "Item\tColor");
		themeEditorTable->itemCount = sizeof(themeItems) / sizeof(themeItems[0]);
		themeEditorTable->e.messageUser = ThemeEditorTableMessage;
		UITableResizeColumns(themeEditorTable);
	}

	UIWindowRegisterShortcut(window, (UIShortcut) { .code = UI_KEYCODE_LETTER('T'), .ctrl = true, .invoke = MyMenuCallback, .cp = (void *) "Keyboard shortcut!" });

	{
		UIWindow *window = UIWindowCreate(0, 0, "MDI Example", 0, 0);
		UIMDIClient *client = UIMDIClientCreate(&window->e, 0);
		UIMDIChild *child1 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(10, 600, 10, 400), "My Window", -1);
		UIPanel *panel1 = UIPanelCreate(&child1->e, UI_PANEL_GRAY | UI_PANEL_MEDIUM_SPACING);
		UILabelCreate(&panel1->e, 0, "It's a christmas miracle", -1);
		UIMDIChild *child2 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(40, 630, 40, 430), "Second Window", -1);
		UIPanel *panel2 = UIPanelCreate(&child2->e, UI_PANEL_GRAY | UI_PANEL_MEDIUM_SPACING);
		UILabelCreate(&panel2->e, 0, "the system is down", -1);
		UIMDIChild *child3 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(70, 670, 70, 470), "Third Window", -1);
		UIButtonCreate(&child3->e, 0, "giant button!!", -1);
	}

	return UIMessageLoop();
}
