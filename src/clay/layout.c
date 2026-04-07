#include "clay.h"
#include "clay_renderer_raylib.c"

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

static Clay_TextElementConfig getFontBodyConfig(void)
{
	return (Clay_TextElementConfig) {
		.fontSize = 24,
		.fontId = FONT_ID_BODY_24,
		.textColor = COLOR_TEXT
	};
}

static Clay_ElementDeclaration getBodyConfig(void)
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
			.padding = CLAY_PADDING_ALL(22),
			.childGap = 21,
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_GROW(0)
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

static Clay_ElementDeclaration getMainContent(void)
{
	Clay_ElementDeclaration panel = getPanel();
	panel.clip = (Clay_ClipElementConfig) {
		.vertical = true,
		.childOffset = Clay_GetScrollOffset()
	};
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
	return (Clay_ElementDeclaration) {
		.layout = {
			.sizing = {
				Clay_PointerOver(Clay_GetElementId(scrollBar))
				? CLAY_SIZING_FIXED(6)
				: CLAY_SIZING_FIXED(3),
				CLAY_SIZING_FIXED((scrollData.scrollContainerDimensions.height / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height)
			}
		},
		.backgroundColor = Clay_PointerOver(Clay_GetElementId(scrollBar))
		? COLOR_OUTLINE_BRIGHT
		: COLOR_OUTLINE,
		.cornerRadius = CLAY_CORNER_RADIUS(2)
	};
}
