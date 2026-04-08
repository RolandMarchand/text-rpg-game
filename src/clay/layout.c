#include "clay.h"
#include "clay_renderer_raylib.c"
#include "../common.h"

/* Colors */
#define COLOR_BACKGROUND (Clay_Color) { 0x0C, 0x0A, 0x09, 0xFF }
#define COLOR_PANEL (Clay_Color) { 0x1C, 0x19, 0x17, 0xFF }
#define COLOR_OUTLINE (Clay_Color) { 0x44, 0x40, 0x3C, 0xFF }
#define COLOR_OUTLINE_BRIGHT (Clay_Color) { 0x57, 0x53, 0x4E, 0xFF }
#define COLOR_TEXT_HIGHLIGHT (Clay_Color) { 0xFC, 0xD3, 0x4D, 0xFF }
#define COLOR_TEXT (Clay_Color) { 0xE7, 0xE5, 0xE4, 0xFF }
#define COLOR_TEXT_MUTED (Clay_Color) { 0x78, 0x71, 0x6C, 0xFF }

enum {
	FONT_ID_BODY_24 = 0,
	FONT_ID_BODY_16,
	FONT_ID_MAX,
};

static Clay_TextElementConfig getFontBody(void)
{
	return (Clay_TextElementConfig) {
		.fontSize = 24,
		.fontId = FONT_ID_BODY_24,
		.textColor = COLOR_TEXT
	};
}

static Clay_TextElementConfig getFontAction(void)
{
	Clay_TextElementConfig body = getFontBody();

	body.textColor = COLOR_TEXT_MUTED;

	return body;
}

static Clay_ElementDeclaration getBody(void)
{
	return (Clay_ElementDeclaration) {
		.layout = {
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.padding = CLAY_PADDING_ALL(22),
			.childGap = 21,
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_GROW(0)
			}
		},
		.backgroundColor = COLOR_BACKGROUND
	};
}

static Clay_ElementDeclaration getPanel(void)
{
	return (Clay_ElementDeclaration) {
		.layout = {
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.padding = CLAY_PADDING_ALL(18),
			.childGap = 21,
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_FIT(0)
			}
		},
		.backgroundColor = COLOR_PANEL,
		.cornerRadius = CLAY_CORNER_RADIUS(8),
		.border = {
			.color = COLOR_OUTLINE,
			.width = CLAY_BORDER_OUTSIDE(1)
		},
	};
}

static Clay_ElementDeclaration getButton(bool *hoveredOut)
{
	Clay_ElementDeclaration panel = getPanel();
	bool hovered = Clay_Hovered() && IsCursorOnScreen();

	if (hovered) {
		SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
	}

	panel.backgroundColor = hovered ? COLOR_OUTLINE : COLOR_PANEL;
	panel.border.color = hovered ? COLOR_OUTLINE_BRIGHT : COLOR_OUTLINE;
	panel.layout.childGap = 8;

	if (hoveredOut != NULL) {
		*hoveredOut = hovered;
	}

	return panel;
}

static Clay_ElementDeclaration getActionSection(void)
{
	return (Clay_ElementDeclaration) {
		.layout = {
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.childGap = 11,
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_FIT(0)
			}
		}
	};
}

static Clay_ElementDeclaration getActionButton(bool *hoveredOut)
{
	Clay_ElementDeclaration button = getButton(hoveredOut);

	button.layout.layoutDirection = CLAY_LEFT_TO_RIGHT;

	return button;
}

static Clay_ElementDeclaration getMainContent(void)
{
	Clay_ElementDeclaration panel = getPanel();

	panel.clip = (Clay_ClipElementConfig) {
		.vertical = true,
		.childOffset = Clay_GetScrollOffset()
	};
	panel.layout.sizing.height = CLAY_SIZING_GROW(0);
	panel.layout.padding = (Clay_Padding) { 27, 27, 31, 31 };

	return panel;
}

static Clay_ElementDeclaration getScrollBar(
	Clay_String parent,
	Clay_ScrollContainerData scrollData
)
{
	return (Clay_ElementDeclaration) {
		.floating = {
			.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
			.offset = {
				.y = -(scrollData.scrollPosition->y
				       / scrollData.contentDimensions.height)
				* scrollData.scrollContainerDimensions.height
				+ 7
			},
			.zIndex = 1,
			.parentId = Clay_GetElementId(parent).id,
			.attachPoints = {
				.element = CLAY_ATTACH_POINT_RIGHT_TOP,
				.parent = CLAY_ATTACH_POINT_RIGHT_TOP
			}
		}
	};
}

static Clay_ElementDeclaration getScrollBarButton(
	Clay_String scrollBar,
	Clay_ScrollContainerData scrollData
)
{
	float scrollHeight = scrollData.scrollContainerDimensions.height;
	float contentHeight = scrollData.contentDimensions.height;
	float size = 0.0f;
	if (scrollHeight < contentHeight) {
		size = (scrollHeight / contentHeight) * scrollHeight - 14;
	}

	return (Clay_ElementDeclaration) {
		.layout = {
			.sizing = {
				Clay_PointerOver(Clay_GetElementId(scrollBar))
				? CLAY_SIZING_FIXED(6)
				: CLAY_SIZING_FIXED(3),
				CLAY_SIZING_FIXED(size)
			}
		},
		.backgroundColor = Clay_PointerOver(Clay_GetElementId(scrollBar))
		? COLOR_OUTLINE_BRIGHT
		: COLOR_OUTLINE,
		.cornerRadius = CLAY_CORNER_RADIUS(2)
	};
}

static Clay_String getActionOrderString(u8 index)
{
	switch (index) {
	case 0: return CLAY_STRING("0.");
	case 1: return CLAY_STRING("1.");
	case 2: return CLAY_STRING("2.");
	case 3: return CLAY_STRING("3.");
	case 4: return CLAY_STRING("4.");
	case 5: return CLAY_STRING("5.");
	case 6: return CLAY_STRING("6.");
	case 7: return CLAY_STRING("7.");
	case 8: return CLAY_STRING("8.");
	case 9: return CLAY_STRING("9.");
	case 10: return CLAY_STRING("10.");
	default: return CLAY_STRING("??.");
	}
}
