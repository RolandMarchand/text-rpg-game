#include "view.h"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/clay_renderer_raylib.c"

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

Error initView(void)
{
	/* Raylib */
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
		   "raylib [text] example - input box");

	/* Clay */
	clayMemorySize = Clay_MinMemorySize();
	clayMemory = Clay_CreateArenaWithCapacityAndMemory(
		clayMemorySize,
		malloc_try(clayMemorySize));
	Clay_Initialize(
		clayMemory,
		(Clay_Dimensions) {
			(float)GetScreenWidth(),
			(float)GetScreenHeight()
		},
		(Clay_ErrorHandler) { HandleClayErrors, 0 });
	Clay_Raylib_Initialize(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		"Clay - Raylib Renderer Example",
		FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

	/* Fonts */
	fonts[FONT_ID_BODY_24]
		= LoadFontEx(RESOURCES_DIR "/Roboto-Regular.ttf", 48, 0, 400);
	SetTextureFilter(
		fonts[FONT_ID_BODY_24].texture,
		TEXTURE_FILTER_BILINEAR);
	fonts[FONT_ID_BODY_16]
		= LoadFontEx(RESOURCES_DIR "/Roboto-Regular.ttf", 32, 0, 400);
	SetTextureFilter(
		fonts[FONT_ID_BODY_16].texture,
		TEXTURE_FILTER_BILINEAR);
	Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

	/* Textures */
	profilePicture = LoadTexture(RESOURCES_DIR "/profile-picture.png");

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

// Examples of re-usable "Components"
static void RenderHeaderButton(Clay_String text) {
    CLAY_AUTO_ID(HeaderButtonStyle(Clay_Hovered())) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG(headerTextConfig));
    }
}

static Clay_RenderCommandArray CreateLayout(void)
{
	Clay_BeginLayout();
	CLAY(CLAY_ID("OuterContainer"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, .padding = { 16, 16, 16, 16 }, .childGap = 16 }, .backgroundColor = {200, 200, 200, 255} }) {
		CLAY(CLAY_ID("SideBar"), { .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = { .width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0) }, .padding = {16, 16, 16, 16 }, .childGap = 16 }, .backgroundColor = {150, 150, 255, 255} }) {
			CLAY(CLAY_ID("ProfilePictureOuter"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = { 8, 8, 8, 8 }, .childGap = 8, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } }, .backgroundColor = {130, 130, 255, 255} }) {
				CLAY(CLAY_ID("ProfilePicture"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60), .height = CLAY_SIZING_FIXED(60) } }, .image = { .imageData = &profilePicture }}) {}
				CLAY_TEXT(profileText, CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {0, 0, 0, 255}, .textAlignment = CLAY_TEXT_ALIGN_RIGHT }));
			}
			CLAY(CLAY_ID("SidebarBlob1"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }}, .backgroundColor = {110, 110, 255, 255} }) {}
			CLAY(CLAY_ID("SidebarBlob2"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }}, .backgroundColor = {110, 110, 255, 255} }) {}
			CLAY(CLAY_ID("SidebarBlob3"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }}, .backgroundColor = {110, 110, 255, 255} }) {}
			CLAY(CLAY_ID("SidebarBlob4"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }}, .backgroundColor = {110, 110, 255, 255} }) {}
		}

		CLAY(CLAY_ID("RightPanel"), { .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, .childGap = 16 }}) {
			CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }, .padding = {8, 8, 8, 8 }, .childGap = 8 }, .backgroundColor =  {180, 180, 180, 255} }) {
				RenderHeaderButton(CLAY_STRING("Header Item 1"));
				RenderHeaderButton(CLAY_STRING("Header Item 2"));
				RenderHeaderButton(CLAY_STRING("Header Item 3"));
			}
			CLAY(CLAY_ID("MainContent"), {
					.layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .padding = {16, 16, 16, 16}, .childGap = 16, .sizing = { .width = CLAY_SIZING_GROW(0) } },
					.backgroundColor = {200, 200, 255, 255},
					.clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() },
				})
			{
				CLAY(CLAY_ID("FloatingContainer"), {
						.layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.5), .height = CLAY_SIZING_FIXED(300) }, .padding = { 16, 16, 16, 16 }},
						.backgroundColor = { 140, 80, 200, 200 },
						.floating = { .attachTo = CLAY_ATTACH_TO_PARENT, .zIndex = 1, .attachPoints = { CLAY_ATTACH_POINT_CENTER_TOP, CLAY_ATTACH_POINT_CENTER_TOP }, .offset = {0, 0} },
						.border = { .width = CLAY_BORDER_OUTSIDE(2), .color = {80, 80, 80, 255} },
					}) {
					CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {255,255,255,255} }));
				}

				CLAY_TEXT(CLAY_STRING("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt."),
					  CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_24, .fontSize = 24, .textColor = {0,0,0,255} }));

				CLAY(CLAY_ID("Photos2"), { .layout = { .childGap = 16, .padding = { 16, 16, 16, 16 }}, .backgroundColor = {180, 180, 220, Clay_Hovered() ? 120 : 255} }) {
					CLAY(CLAY_ID("Picture4"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(120), .height = CLAY_SIZING_FIXED(120) }}, .image = { .imageData = &profilePicture }}) {}
					CLAY(CLAY_ID("Picture5"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(120), .height = CLAY_SIZING_FIXED(120) }}, .image = { .imageData = &profilePicture }}) {}
					CLAY(CLAY_ID("Picture6"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(120), .height = CLAY_SIZING_FIXED(120) }}, .image = { .imageData = &profilePicture }}) {}
				}

				CLAY_TEXT(CLAY_STRING("Faucibus purus in massa tempor nec. Nec ullamcorper sit amet risus nullam eget felis eget nunc. Diam vulputate ut pharetra sit amet aliquam id diam. Lacus suspendisse faucibus interdum posuere lorem. A diam sollicitudin tempor id. Amet massa vitae tortor condimentum lacinia. Aliquet nibh praesent tristique magna."),
					  CLAY_TEXT_CONFIG({ .fontSize = 24, .lineHeight = 60, .textColor = {0,0,0,255}, .textAlignment = CLAY_TEXT_ALIGN_CENTER }));

				CLAY_TEXT(CLAY_STRING("Suspendisse in est ante in nibh. Amet venenatis urna cursus eget nunc scelerisque viverra. Elementum sagittis vitae et leo duis ut diam quam nulla. Enim nulla aliquet porttitor lacus. Pellentesque habitant morbi tristique senectus et. Facilisi nullam vehicula ipsum a arcu cursus vitae.\nSem fringilla ut morbi tincidunt. Euismod quis viverra nibh cras pulvinar mattis nunc sed. Velit sed ullamcorper morbi tincidunt ornare massa. Varius quam quisque id diam vel quam. Nulla pellentesque dignissim enim sit amet venenatis. Enim lobortis scelerisque fermentum dui faucibus in. Pretium viverra suspendisse potenti nullam ac tortor vitae. Lectus vestibulum mattis ullamcorper velit sed. Eget mauris pharetra et ultrices neque ornare aenean euismod elementum. Habitant morbi tristique senectus et. Integer vitae justo eget magna fermentum iaculis eu. Semper quis lectus nulla at volutpat diam. Enim praesent elementum facilisis leo. Massa vitae tortor condimentum lacinia quis vel."),
					  CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {0,0,0,255} }));

				CLAY(CLAY_ID("Photos"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, .childGap = 16, .padding = {16, 16, 16, 16} }, .backgroundColor = {180, 180, 220, 255} }) {
					CLAY(CLAY_ID("Picture2"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(120) }}, .aspectRatio = { 0.5 }, .image = { .imageData = &profilePicture }}) {}
					CLAY(CLAY_ID("Picture1"), { .layout = { .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, .layoutDirection = CLAY_TOP_TO_BOTTOM, .padding = {8, 8, 8, 8} }, .backgroundColor = {170, 170, 220, 255} }) {
						CLAY(CLAY_ID("ProfilePicture2"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60), .height = CLAY_SIZING_FIXED(60) }}, .image = { .imageData = &profilePicture }}) {}
						CLAY_TEXT(CLAY_STRING("Image caption below"), CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {0,0,0,255} }));
					}
					CLAY(CLAY_ID("Picture3"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(120) }}, .aspectRatio = { 2 }, .image = { .imageData = &profilePicture }}) {}
				}

				CLAY_TEXT(CLAY_STRING("Text"),
					  CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {0,0,0,255} }));
			}
		}

		CLAY(CLAY_ID("Blob4Floating2"), { .floating = { .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, .zIndex = 1, .parentId = Clay_GetElementId(CLAY_STRING("SidebarBlob4")).id } }) {
			CLAY(CLAY_ID("ScrollContainer"), { .layout = { .sizing = { .height = CLAY_SIZING_FIXED(200) }, .childGap = 2 }, .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() } }) {
				CLAY(CLAY_ID("FloatingContainer2"), { .layout.sizing.height = CLAY_SIZING_GROW(0), .floating = { .attachTo = CLAY_ATTACH_TO_PARENT, .zIndex = 1 } }) {
					CLAY(CLAY_ID("FloatingContainerInner"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0) }, .padding = {16, 16, 16, 16} }, .backgroundColor = {140,80, 200, 200} }) {
						CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = {255,255,255,255} }));
					}
				}
				CLAY(CLAY_ID("ScrollContainerInner"), { .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = {160, 160, 160, 255} }) {
					for (int i = 0; i < 100; i++) {
						RenderDropdownTextItem(i);
					}
				}
			}
		}
		Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("MainContent")));
		if (scrollData.found) {
			CLAY(CLAY_ID("ScrollBar"), {
					.floating = {
						.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
						.offset = { .y = -(scrollData.scrollPosition->y / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height },
						.zIndex = 1,
						.parentId = Clay_GetElementId(CLAY_STRING("MainContent")).id,
						.attachPoints = { .element = CLAY_ATTACH_POINT_RIGHT_TOP, .parent = CLAY_ATTACH_POINT_RIGHT_TOP }
					}
				}) {
				CLAY(CLAY_ID("ScrollBarButton"), {
						.layout = { .sizing = {CLAY_SIZING_FIXED(12), CLAY_SIZING_FIXED((scrollData.scrollContainerDimensions.height / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height) }},
						.backgroundColor = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar"))) ? (Clay_Color){100, 100, 140, 150} : (Clay_Color){120, 120, 160, 150} ,
						.cornerRadius = CLAY_CORNER_RADIUS(6)
					}) {}
			}
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
