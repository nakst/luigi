#define UI_LINUX
#define UI_IMPLEMENTATION
#include "luigi.h"

#include <stdio.h>

typedef struct Unit {
	const char *name;
	double ratio, bias;
} Unit;

typedef struct Category {
	const char *name;
	Unit *units;
} Category;

#define CATEGORY_START(name) { name, (Unit []) {
#define CATEGORY_END() { NULL }, }, },
#define UNIT(...) { __VA_ARGS__ },

const Category categories[] = {
	CATEGORY_START("Length")
		UNIT("Millimeters (mm)", 0.001)
		UNIT("Meters (m)", 1.0)
		UNIT("Kilometers (km)", 1000.0)
		UNIT("Yards", 0.9144)
	CATEGORY_END()

	CATEGORY_START("Temperature")
		UNIT("Celsius", 1.0)
		UNIT("Fahrenheit", 0.555555555556, 32.0)
	CATEGORY_END()
};

int category = -1, from = -1, to = -1;
UITable *fromTable, *toTable;
UITextbox *input;
UILabel *output;

void Calculate() {
	if (category == -1 || from == -1 || to == -1) {
		UILabelSetContent(output, "Select units to convert.", -1);
	} else if (!input->bytes) {
		UILabelSetContent(output, "Enter value to convert.", -1);
	} else {
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "%.*s", (int) input->bytes, input->string);
		Unit *unitFrom = &categories[category].units[from];
		Unit *unitTo = &categories[category].units[to];
		double x = (strtod(buffer, NULL) - unitFrom->bias) * unitFrom->ratio / unitTo->ratio + unitTo->bias;
		snprintf(buffer, sizeof(buffer), "%f %s", x, unitTo->name);
		UILabelSetContent(output, buffer, -1);
	}

	UIElementRepaint(&output->e, NULL);
}

int CategoryTableMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_TABLE_GET_ITEM) {
		UITableGetItem *m = (UITableGetItem *) dp;
		m->isSelected = category == m->index;
		return snprintf(m->buffer, m->bufferBytes, "%s", categories[m->index]);
	} else if (message == UI_MSG_LEFT_DOWN || message == UI_MSG_MOUSE_DRAG) {
		int hit = UITableHitTest((UITable *) element, element->window->cursorX, element->window->cursorY);
		if (hit != -1) category = hit;
		else return 0;
		int unitCount = 0;
		while (categories[category].units[unitCount].name) unitCount++;
		fromTable->itemCount = toTable->itemCount = unitCount;
		from = to = -1;
		UITableResizeColumns(fromTable);
		UITableResizeColumns(toTable);
		UIElementRepaint(&element->window->e, NULL);
	}

	return 0;
}

int FromToTableMessage(UIElement *element, UIMessage message, int di, void *dp) {
	int *index = (int *) element->cp;

	if (message == UI_MSG_TABLE_GET_ITEM && category != -1) {
		UITableGetItem *m = (UITableGetItem *) dp;
		m->isSelected = *index == m->index;
		return snprintf(m->buffer, m->bufferBytes, "%s", categories[category].units[m->index].name);
	} else if (message == UI_MSG_LEFT_DOWN || message == UI_MSG_MOUSE_DRAG) {
		int hit = UITableHitTest((UITable *) element, element->window->cursorX, element->window->cursorY);
		if (hit != -1) *index = hit;
		else return 0;
		UIElementRepaint(&element->window->e, NULL);
		Calculate();
	}

	return 0;
}

int InputMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_VALUE_CHANGED) {
		Calculate();
	}

	return 0;
}

int main() {
	UIInitialise();
	UIWindowCreate(0, UI_ELEMENT_PARENT_PUSH, "Converter", 500, 300);
	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_GRAY | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_HORIZONTAL | UI_ELEMENT_V_FILL);
	UITable *table = UITableCreate(0, UI_ELEMENT_H_FILL, "Category");
	table->e.messageUser = CategoryTableMessage;
	table->itemCount = sizeof(categories) / sizeof(categories[0]);
	UITableResizeColumns(table);
	fromTable = UITableCreate(0, UI_ELEMENT_H_FILL, "From");
	fromTable->e.messageUser = FromToTableMessage;
	fromTable->e.cp = &from;
	toTable = UITableCreate(0, UI_ELEMENT_H_FILL, "To");
	toTable->e.messageUser = FromToTableMessage;
	toTable->e.cp = &to;
	UIParentPop();
	UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_HORIZONTAL);
	input = UITextboxCreate(0, UI_ELEMENT_H_FILL);
	input->e.messageUser = InputMessage;
	UILabelCreate(0, 0, " --> ", -1);
	output = UILabelCreate(0, UI_ELEMENT_H_FILL, 0, -1);
	UIElementFocus(&input->e);
	return UIMessageLoop();
}
