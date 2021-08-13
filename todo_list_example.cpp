// A simple example of a "TODO" list.

// Include the Luigi library.

#define UI_WINDOWS
#define UI_IMPLEMENTATION
#include "../util/luigi.h"

// Include standard headers.

#include <stdlib.h>

// Array of TODO items.

struct Item {
	char *text;
	size_t textBytes;
	bool completed;
};

Item *items;
size_t itemCount;

// Tabs.

#define TAB_ALL ((intptr_t) 0)
#define TAB_ACTIVE ((intptr_t) 1)
#define TAB_COMPLETED ((intptr_t) 2)
intptr_t tab;
UIButton *switchTabButtons[3];

// UI elements.

UITextbox *inputTextbox;
UIPanel *itemsPanel;

int EditItemMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_CLICKED) {
		// Handle clicking the edit item button.
		// - Replace the textbox's contents with the item's contents.
		// - Remove the item from the list.
		
		intptr_t index = (intptr_t) element->cp;
		UITextboxClear(inputTextbox, false);
		UITextboxReplace(inputTextbox, items[index].text, items[index].textBytes, false);
		memmove(items + index, items + index + 1, (itemCount - index - 1) * sizeof(Item));
		itemCount--;
		UIElementDestroy(element->parent);
		UIElementRefresh(&itemsPanel->e);
		UIElementRefresh(&inputTextbox->e);
	}

	return 0;
}

int CheckItemMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_CLICKED) {
		// Handle checking/unchecking an item.
		// - Update the label.
		// - If the tab no longer should display the item, destroy its UI.
		
		Item *item = items + (intptr_t) element->cp;
		item->completed = !item->completed;
		UIButton *button = (UIButton *) element;
		button->label[0] = item->completed ? 15 : ' ';
		UIElementRefresh(element);

		if ((item->completed && tab == TAB_ACTIVE)
				|| (!item->completed && tab == TAB_COMPLETED)) {
			UIElementDestroy(element->parent);
			UIElementRefresh(&itemsPanel->e);
		}
	}

	return 0;
}

void AddItem(Item *item) {
	// Add an item to the items panel.
	// - Create a panel to contain the item.
	// - Add a check button.
	// - Add an edit button.
	// - Add a label containing the item's contents.
	
	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_HORIZONTAL | UI_ELEMENT_H_FILL | UI_PANEL_MEDIUM_SPACING);
	char label = item->completed ? 15 : ' ';
	UIButton *button = UIButtonCreate(0, UI_BUTTON_SMALL, &label, 1);
	button->e.cp = (void *) (item - items);
	button->e.messageUser = CheckItemMessage;
	button = UIButtonCreate(0, UI_BUTTON_SMALL, "edit", -1);
	button->e.cp = (void *) (item - items);
	button->e.messageUser = EditItemMessage;
	UILabelCreate(0, UI_ELEMENT_H_FILL, item->text, item->textBytes);
	UIParentPop();
	UIElementRefresh(&itemsPanel->e);
}

void SwitchTab(void *_target) {
	// Switch to a different tab.
	// - Remove the old items.
	// - Add the new items.
	// - Update the tab buttons' checked states.
	
	tab = (intptr_t) _target;
	UIElementDestroyDescendents(&itemsPanel->e);

	for (uintptr_t i = 0; i < itemCount; i++) {
		if (tab == TAB_COMPLETED && !items[i].completed) {
			continue;
		} else if (tab == TAB_ACTIVE && items[i].completed) {
			continue;
		}

		AddItem(items + i);
	}

	UIElementRefresh(&itemsPanel->e);

	for (uintptr_t i = 0; i < 3; i++) {
		if (i == tab) switchTabButtons[i]->e.flags |= UI_BUTTON_CHECKED;
		else switchTabButtons[i]->e.flags &= ~UI_BUTTON_CHECKED;
		UIElementRefresh(&switchTabButtons[i]->e);
	}
}

int InputTextboxMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_KEY_TYPED && ((UIKeyTyped *) dp)->code == UI_KEYCODE_ENTER) {
		// Add a new item to the array.
		
		items = (Item *) realloc(items, (itemCount + 1) * sizeof(Item));
		items[itemCount].textBytes = inputTextbox->bytes;
		items[itemCount].text = (char *) malloc(inputTextbox->bytes);
		items[itemCount].completed = false;
		memcpy(items[itemCount].text, inputTextbox->string, inputTextbox->bytes);
		UITextboxClear(inputTextbox, false);
		UIElementRefresh(&inputTextbox->e);
		
		// Switch to the correct tab if necessary, and add the item.
		
		if (tab == TAB_COMPLETED) SwitchTab((void *) TAB_ALL);
		itemsPanel->scrollBar->position = 1e10; // Scroll to the bottom.
		AddItem(items + itemCount);
		itemCount++;
	}

	return 0;
}

int WinMain(HINSTANCE, HINSTANCE, char *, int) {
	// Initialise Luigi and create a window.
	
	UIInitialise();
	ui.theme = _uiThemeClassic;
	UIWindowCreate(0, UI_ELEMENT_PARENT_PUSH, "To-do List", 0, 0);
	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_MEDIUM_SPACING | UI_PANEL_GRAY);
	
	// Create the task insertion panel.

	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_HORIZONTAL | UI_PANEL_MEDIUM_SPACING | UI_PANEL_GRAY | UI_ELEMENT_H_FILL);
	UILabelCreate(0, 0, "Task:", -1);
	inputTextbox = UITextboxCreate(0, 0);
	inputTextbox->e.messageUser = InputTextboxMessage;
	UIParentPop();
	
	// Create the tabs panel.

	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_HORIZONTAL | UI_PANEL_MEDIUM_SPACING | UI_PANEL_GRAY | UI_ELEMENT_H_FILL);
#define SWITCH_TAB_BUTTON(label, target) { UIButton *b = UIButtonCreate(0, 0, label, -1); \
		b->invoke = SwitchTab; b->e.cp = (void *) target; switchTabButtons[target] = b; }
	SWITCH_TAB_BUTTON("All", TAB_ALL);
	SWITCH_TAB_BUTTON("Active", TAB_ACTIVE);
	SWITCH_TAB_BUTTON("Completed", TAB_COMPLETED);
	UIParentPop();
	
	// Create the items panel.

	itemsPanel = UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_WHITE 
			| UI_PANEL_MEDIUM_SPACING | UI_ELEMENT_H_FILL 
			| UI_ELEMENT_V_FILL | UI_PANEL_SCROLL);
			
	// Switch to the "all" tab.

	SwitchTab(TAB_ALL);
	
	// Process input messages until the window is closed.

	return UIMessageLoop();
}
