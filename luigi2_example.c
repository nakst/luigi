#include <stdio.h>

// #define UI_LINUX
// #define UI_WINDOWS
// #define UI_DEBUG
#define UI_IMPLEMENTATION
#include "luigi2 (beta).h"

UIWindow *window;
UILabel *label;

UISlider *slider_horiz;
UIGauge *gauge_horiz1;
UIGauge *gauge_horiz2;

UISlider *slider_vert;
UIGauge *gauge_vert1;
UIGauge *gauge_vert2;

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

int MySliderHMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		gauge_horiz2->position = slider_horiz->position;
		gauge_vert1->position = slider_horiz->position;
		UIElementRepaint(&gauge_horiz2->e, NULL);
		UIElementRepaint(&gauge_vert1->e, NULL);
	}

	return 0;
}

int MySliderVMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		gauge_vert2->position = slider_vert->position;
		gauge_horiz1->position = slider_vert->position;
		UIElementRepaint(&gauge_vert1->e, NULL);
		UIElementRepaint(&gauge_horiz1->e, NULL);
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

	window = UIWindowCreate(0, 0, "luigi2 - Example Application", 0, 0);

    // Split window (vertically) into top/bottom panes.
	UISplitPane *uisplit_topbottom = UISplitPaneCreate(&window->e, UI_SPLIT_PANE_VERTICAL, 0.75f);

	// Split top pane (horizontally) into left/right panes.
	UISplitPane *uisplit_top_leftright = UISplitPaneCreate(&uisplit_topbottom->e, 0, 0.3f);

	{
		// In the top-left pane - create a single panel taking up the whole pane.
		UIPanel *panel = UIPanelCreate(&uisplit_top_leftright->e, UI_PANEL_COLOR_1 | UI_PANEL_MEDIUM_SPACING);
		// Panels are by default vertical in layout, so items start at top and go down.
		UIButtonCreate(&panel->e, 0, "Hello World", -1)->e.messageUser = MyButtonMessage;
		// Create a new horizontal-layout "sub-panel" and put left and right panels inside it.
		UIPanel *subpanel = UIPanelCreate(&panel->e, UI_PANEL_COLOR_1 | UI_PANEL_HORIZONTAL);
		// The left side will layout elements horizontally, with custom borders and gap.
		UIPanel *sub_left = UIPanelCreate(&subpanel->e, UI_PANEL_COLOR_1 | UI_PANEL_HORIZONTAL);
		sub_left->border.t = 10;
		sub_left->border.b = 10;
		sub_left->border.l = 10;
		sub_left->border.r = 10;
		sub_left->gap = 2;
		gauge_vert1 = UIGaugeCreate(&sub_left->e, UI_GAUGE_VERTICAL);
		gauge_vert2 = UIGaugeCreate(&sub_left->e, UI_GAUGE_VERTICAL);
		slider_vert = UISliderCreate(&sub_left->e, UI_SLIDER_VERTICAL);
		slider_vert->e.messageUser = MySliderVMessage;
		// The right side will lay out elements vertically (the default), with default medium spacing.
		UIPanel *sub_right = UIPanelCreate(&subpanel->e, UI_PANEL_COLOR_1 | UI_PANEL_MEDIUM_SPACING);
		UIButtonCreate(&sub_right->e, 0, "1", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&sub_right->e, 0, "2", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&sub_right->e, 0, "3", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&sub_right->e, 0, "4", -1)->e.messageUser = MyButtonMessage;
		UIButtonCreate(&sub_right->e, 0, "5", -1)->e.messageUser = MyButtonMessage;
		// Back outside of the "sub-panel", we continue layout downwards.
		UIButtonCreate(&panel->e, 0, "Goodbye World", -1)->e.messageUser = MyButtonMessage;
		gauge_horiz1 = UIGaugeCreate(&panel->e, 0);
		gauge_horiz2 = UIGaugeCreate(&panel->e, 0);
		slider_horiz = UISliderCreate(&panel->e, 0);
		slider_horiz->e.messageUser = MySliderHMessage;
		UITextboxCreate(&panel->e, 0);
		// Set default slider positions.
		UISliderSetPosition(slider_vert, 0.1, true);
		UISliderSetPosition(slider_horiz, 0.3, true);
	}

	{
		// Top-Right pane.
		UICode *code = UICodeCreate(&uisplit_top_leftright->e, 0);
		char *buffer = (char *) malloc(262144);
		FILE *f = fopen("luigi.h", "rb");
		size_t size = fread(buffer, 1, 262144, f);
		fclose(f);
		UICodeInsertContent(code, buffer, size, true);
		UICodeFocusLine(code, 0);
	}

	// Split bottom pane (horizontally) into left/right panes.
	UISplitPane *uisplit_bottom_leftright = UISplitPaneCreate(&uisplit_topbottom->e, 0, 0.3f);

	{
		// Bottom-Left pane.
		UIPanel *panel = UIPanelCreate(&uisplit_bottom_leftright->e, UI_PANEL_COLOR_2);
		panel->border = UI_RECT_1(5);
		panel->gap = 5;
		UIButtonCreate(&panel->e, 0, "It's a button??", -1)->e.messageUser = MyButton2Message;
		label = UILabelCreate(&panel->e, UI_ELEMENT_H_FILL, "Hello, I am a label!", -1);
	}

	{
		// Bottom-Right pane.
		UITabPane *tabPane = UITabPaneCreate(&uisplit_bottom_leftright->e, 0, "Tab 1\tMiddle Tab\tTab 3");
		UITable *table = UITableCreate(&tabPane->e, 0, "Column 1\tColumn 2");
		table->itemCount = 100000;
		table->e.messageUser = MyTableMessage;
		UITableResizeColumns(table);
		UILabelCreate(&UIPanelCreate(&tabPane->e, UI_PANEL_COLOR_1)->e, 0, "you're in tab 2, bucko", -1);
		UILabelCreate(&UIPanelCreate(&tabPane->e, UI_PANEL_COLOR_1)->e, 0, "hiii!!!", -1);
	}

	UIWindowRegisterShortcut(window, (UIShortcut) { .code = UI_KEYCODE_LETTER('T'), .ctrl = true, .invoke = MyMenuCallback, .cp = (void *) "Keyboard shortcut!" });

	{
		// Create a separate window demonstrating the MDI element
		UIWindow *window = UIWindowCreate(0, 0, "luigi 2 - MDI Example", 0, 0);
		UIMDIClient *client = UIMDIClientCreate(&window->e, 0);
		UIMDIChild *child1 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(10, 600, 10, 400), "My Window", -1);
		UIPanel *panel1 = UIPanelCreate(&child1->e, UI_PANEL_COLOR_1 | UI_PANEL_MEDIUM_SPACING);
		UILabelCreate(&panel1->e, 0, "It's a christmas miracle", -1);
		UIMDIChild *child2 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(40, 630, 40, 430), "Second Window", -1);
		UIPanel *panel2 = UIPanelCreate(&child2->e, UI_PANEL_COLOR_1 | UI_PANEL_MEDIUM_SPACING);
		UILabelCreate(&panel2->e, 0, "the system is down", -1);
		UIMDIChild *child3 = UIMDIChildCreate(&client->e, UI_MDI_CHILD_CLOSE_BUTTON, UI_RECT_4(70, 670, 70, 470), "Third Window", -1);
		UIButtonCreate(&child3->e, 0, "giant button!!", -1);
	}

	return UIMessageLoop();
}
