#include "view.h"

#include "list/list.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/clay_renderer_raylib.c"

#define CLAY_CSTRING(str) (Clay_String) { .length = strlen(str), .chars = str }
#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector)\
	(Clay_Vector2) { .x = vector.x, .y = vector.y }
#define COLOR_ORANGE (Clay_Color) {225, 138, 50, 255}
#define COLOR_BLUE (Clay_Color) {111, 173, 162, 255}

enum {
	SCREEN_WIDTH = 800,
	SCREEN_HEIGHT = 450,
	FONT_ID_BODY_24 = 0,
	FONT_ID_BODY_16 = 1,
};

typedef struct ScrollBarData {
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    bool mouseDown;
} ScrollbarData;

static u32 clayMemorySize;
static Clay_Arena clayMemory;
static bool mustReinitializeClay;
static Font fonts[2];
static bool debugEnabled;
static ScrollbarData scrollbarData;
static Clay_LayoutConfig dropdownTextItemLayout = { .padding = {8, 8, 4, 4} };
static Clay_TextElementConfig dropdownTextElementConfig = {
	.fontSize = 24,
	.textColor = {255,255,255,255}
};
static Texture2D profilePicture;
static Clay_String profileText = CLAY_STRING_CONST("Profile Page one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen");
static Clay_TextElementConfig headerTextConfig = {
	.fontId = 1,
	.letterSpacing = 5,
	.fontSize = 16,
	.textColor = {0,0,0,255}
};
static ListString *messages;
static const Clay_TextElementConfig messageConfig = {
	.fontSize = 16,
	.fontId = FONT_ID_BODY_24,
	.textColor = {0, 0, 0, 255}
};

static void HandleClayErrors(Clay_ErrorData errorData) {
	errorf("%s", errorData.errorText.chars);
	if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
		mustReinitializeClay = true;
		Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
	} else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
		mustReinitializeClay = true;
		Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
	}
}

static Error initViewClay(void)
{
	clayMemorySize = Clay_MinMemorySize();
	clayMemory = Clay_CreateArenaWithCapacityAndMemory(
		clayMemorySize,
		malloc_try(clayMemorySize));

	Clay_Context *ctx = Clay_Initialize(
		clayMemory,
		(Clay_Dimensions) {
			(float)GetScreenWidth(),
			(float)GetScreenHeight()
		},
		(Clay_ErrorHandler) { HandleClayErrors, 0 });
	if (ctx == NULL) {
		/* TODO: verify proper memory management,
		 * but crash and burn for now */
		return ERR_OUT_OF_MEMORY;
	}

	Clay_Raylib_Initialize(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		"Clay - Raylib Renderer Example",
		FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

	return ERR_OK;
}

static Error initFonts(void)
{
	fonts[FONT_ID_BODY_24]
		= LoadFontEx(RESOURCES_DIR "/Roboto-Regular.ttf", 48, 0, 400);
	if (!IsFontValid(fonts[FONT_ID_BODY_24])) {
		return ERR_RESOURCE_LOADING_FAILED;
	}
	SetTextureFilter(
		fonts[FONT_ID_BODY_24].texture,
		TEXTURE_FILTER_BILINEAR);

	fonts[FONT_ID_BODY_16]
		= LoadFontEx(RESOURCES_DIR "/Roboto-Regular.ttf", 32, 0, 400);
	if (!IsFontValid(fonts[FONT_ID_BODY_16])) {
		/* TODO: unload fonts when loading fails */
		return ERR_RESOURCE_LOADING_FAILED;
	}
	SetTextureFilter(
		fonts[FONT_ID_BODY_16].texture,
		TEXTURE_FILTER_BILINEAR);

	Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

	return ERR_OK;
}

static Error initTextures(void)
{
	profilePicture = LoadTexture(RESOURCES_DIR "/profile-picture.png");
	if (!IsTextureValid(profilePicture)) {
		return ERR_RESOURCE_LOADING_FAILED;
	}

	return ERR_OK;
}

Error initView(void)
{
	messages = (ListString*)newList();
	list_string_push(messages, duplicateString("lmao1"));
	list_string_push(messages, duplicateString("lmao2"));
	list_string_push(messages, duplicateString("lmao3"));

	Step steps[] = {
		initViewClay,
		initFonts,
		initTextures,
	};

	for (size_t i = 0; i < ARRAY_LENGTH(steps); i++) {
		Error err = steps[i]();
		if (err != ERR_OK) {
			return err;
		}
	}

	return ERR_OK;
}

static void reinitializeClay(void)
{
	Clay_SetMaxElementCount(8192);
	clayMemorySize = Clay_MinMemorySize();
	clayMemory = Clay_CreateArenaWithCapacityAndMemory(
		clayMemorySize, malloc_try(clayMemorySize));
	Clay_Initialize(
		clayMemory,
		(Clay_Dimensions) {
			(float)GetScreenWidth(),
			(float)GetScreenHeight()
		},
		(Clay_ErrorHandler) { HandleClayErrors, 0 });
	mustReinitializeClay = false;
}

static void updateAndGetMouseData(
	float *mouseWheelX,
	float *mouseWheelY,
	Clay_Vector2 *mousePosition
)
{
	/* Mouse wheel */
	Vector2 mouseWheelDelta = GetMouseWheelMoveV();
	*mouseWheelX = mouseWheelDelta.x;
	*mouseWheelY = mouseWheelDelta.y;

	/* Mouse position */
	*mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
	Clay_SetPointerState(
		*mousePosition,
		IsMouseButtonDown(0) && !scrollbarData.mouseDown);
	Clay_SetLayoutDimensions((Clay_Dimensions) {
		(float)GetScreenWidth(),
		(float)GetScreenHeight()
	});

	/* Mouse button */
	if (!IsMouseButtonDown(0)) {
		scrollbarData.mouseDown = false;
	}
}

static void updateScrollContainers(Clay_Vector2 mousePosition)
{
	Clay_ScrollContainerData scrollContainerData
		= Clay_GetScrollContainerData(
			Clay_GetElementId(CLAY_STRING("MainContent")));

	if (IsMouseButtonDown(0)
	    && !scrollbarData.mouseDown
	    && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar")))) {
		scrollbarData.clickOrigin = mousePosition;
		scrollbarData.positionOrigin
			= *scrollContainerData.scrollPosition;
		scrollbarData.mouseDown = true;
		return;
	} else if (!scrollbarData.mouseDown) {
		return;
	}


	if (scrollContainerData.contentDimensions.height <= 0) {
		return;
	}

	float contentWidth = scrollContainerData.contentDimensions.width;
	float contentHeight = scrollContainerData.contentDimensions.height;
	float scWidth = scrollContainerData.scrollContainerDimensions.width;
	float scHeight = scrollContainerData.scrollContainerDimensions.height;

	Clay_Vector2 ratio = (Clay_Vector2) {
		contentWidth / scWidth,
		contentHeight / scHeight,
	};

	if (scrollContainerData.config.vertical) {
		scrollContainerData.scrollPosition->y
			= scrollbarData.positionOrigin.y
			+ (scrollbarData.clickOrigin.y - mousePosition.y)
			* ratio.y;
	}

	if (scrollContainerData.config.horizontal) {
		scrollContainerData.scrollPosition->x
			= scrollbarData.positionOrigin.x
			+ (scrollbarData.clickOrigin.x - mousePosition.x)
			* ratio.x;
	}
}

static void RenderDropdownTextItem(int index) {
	(void)index;
	CLAY_AUTO_ID({ .layout = dropdownTextItemLayout, .backgroundColor = {180, 180, 180, 255} }) {
		CLAY_TEXT(CLAY_STRING("I'm a text field in a scroll container."), dropdownTextElementConfig);
	}
}

static Clay_ElementDeclaration HeaderButtonStyle(bool hovered) {
    return (Clay_ElementDeclaration) {
        .layout = {.padding = {16, 16, 8, 8}},
        .backgroundColor = hovered ? COLOR_ORANGE : COLOR_BLUE,
    };
}

static void RenderHeaderButton(Clay_String text) {
    CLAY_AUTO_ID(HeaderButtonStyle(Clay_Hovered())) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG(headerTextConfig));
    }
}

static Clay_RenderCommandArray CreateLayout(void)
{
	Clay_BeginLayout();

	CLAY(CLAY_ID("Container"), {
			.layout = {
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.padding = CLAY_PADDING_ALL(8),
				.childGap = 16,
				.sizing = {
					.width = CLAY_SIZING_GROW(0),
					.height = CLAY_SIZING_GROW(0)
				}
			},
			.backgroundColor = {200, 200, 200, 255}
		}) {
		for (char **str = messages->begin; str < messages->end; str++) {
			char *string = *str;
			CLAY_TEXT(CLAY_CSTRING(string), messageConfig);
		}

	}

	return Clay_EndLayout(GetFrameTime());
}

static void updateDrawFrame(void)
{
	/* Mouse wheel */
	float mouseWheelX = 0.0f;
	float mouseWheelY = 0.0f;
	Clay_Vector2 mousePosition = { 0 };

	updateAndGetMouseData(&mouseWheelX, &mouseWheelY, &mousePosition);

	/* Debug */
	if (IsKeyPressed(KEY_F3)) {
		debugEnabled = !debugEnabled;
		Clay_SetDebugModeEnabled(debugEnabled);
	}

	updateScrollContainers(mousePosition);
	Clay_UpdateScrollContainers(
		true,
		(Clay_Vector2) {mouseWheelX, mouseWheelY}, GetFrameTime());

	Clay_RenderCommandArray renderCommands = CreateLayout();

	/* Rendering */
	BeginDrawing(); {
		ClearBackground(BLACK);
		Clay_Raylib_Render(renderCommands, fonts);
	} EndDrawing();
}

Error updateView(void)
{
	if (WindowShouldClose()) {
		exitGameLoop();
		return ERR_OK;
	}

	if (mustReinitializeClay) {
		reinitializeClay();
        }

        updateDrawFrame();

	return ERR_OK;
}

Error cleanupView(void)
{
	/* Fonts */
	for (size_t i = 0; i < ARRAY_LENGTH(fonts); i++) {
		UnloadFont(fonts[i]);
	}

	/* Textures */
	UnloadTexture(profilePicture);

	Clay_Raylib_Close();

	return ERR_OK;
}
