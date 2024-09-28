#include <nds.h>
#include <gl2d.h>
#include <iostream>
#include <maxmod9.h>
#include "soundbank.h"
#include "soundbank_bin.h"

#include "Cglfont.h"
#include "font_si.h"
#include "font_16x16.h"

// Texture Packer auto-generated UV coords
#include "uvcoord_font_si.h"
#include "uvcoord_font_16x16.h"

// some useful defines
#define HALF_WIDTH (SCREEN_WIDTH / 2)
#define HALF_HEIGHT (SCREEN_HEIGHT / 2)

// This imageset would use our texture packer generated coords so it's kinda
// safe and easy to use
// FONT_SI_NUM_IMAGES is a value #defined from "uvcoord_font_si.h"
glImage FontImages[FONT_SI_NUM_IMAGES];
glImage FontBigImages[FONT_16X16_NUM_IMAGES];

// Our fonts
Cglfont Font;
Cglfont FontBig;

mm_sound_effect collisionSound;

const int WHITE = RGB15(255, 255, 255);
const int RED = RGB15(255, 0, 0);
const int GREEN = RGB15(0, 255, 0);
const int BLUE = RGB15(0, 0, 255);

bool isGamePaused;

int collisionCounter;

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

Rectangle player = {HALF_WIDTH, HALF_HEIGHT, 32, 32, WHITE};
Rectangle bottomBounds = {HALF_WIDTH, HALF_HEIGHT, 32, 32, GREEN};
Rectangle ball = {HALF_WIDTH - 50, HALF_HEIGHT, 20, 20, WHITE};
Rectangle touchBounds = {0, 0, 8, 8, WHITE};

const int PLAYER_SPEED = 5;

int ballVelocityX = 2;
int ballVelocityY = 2;

void drawRectangle(Rectangle rectangle)
{
	glBoxFilled(rectangle.x, rectangle.y, rectangle.x + rectangle.w, rectangle.y + rectangle.h, rectangle.color);
}

bool hasCollision(Rectangle bounds, Rectangle ball)
{
	return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
		   bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}

void update(int keyHeld)
{
	if (keyHeld & KEY_UP && player.y > 0)
	{
		player.y -= PLAYER_SPEED;
	}

	else if (keyHeld & KEY_DOWN && player.y < SCREEN_HEIGHT - player.h)
	{
		player.y += PLAYER_SPEED;
	}

	else if (keyHeld & KEY_RIGHT && player.x < SCREEN_WIDTH - player.w)
	{
		player.x += PLAYER_SPEED;
	}

	else if (keyHeld & KEY_LEFT && player.x > 0)
	{
		player.x -= PLAYER_SPEED;
	}

	if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
	{
		ballVelocityX *= -1;

		ball.color = GREEN;

		mmEffectEx(&collisionSound);
	}

	else if (ball.y < 0 || ball.y > SCREEN_HEIGHT - ball.h)
	{
		ballVelocityY *= -1;

		ball.color = RED;

		mmEffectEx(&collisionSound);
	}

	else if (hasCollision(player, ball))
	{
		ballVelocityX *= -1;
		ballVelocityY *= -1;

		ball.color = BLUE;

		collisionCounter++;

		// Play collision sound effect
		mmEffectEx(&collisionSound);
	}

	ball.x += ballVelocityX;
	ball.y += ballVelocityY;
}

void renderTopScreen()
{
	lcdMainOnTop();
	vramSetBankD(VRAM_D_LCD);
	vramSetBankC(VRAM_C_SUB_BG);
	REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);

	glBegin2D();

	drawRectangle(player);
	drawRectangle(ball);

	if (isGamePaused)
	{
		// change fontsets and  print some spam
		glColor(RGB15(0, 31, 31));
		Font.PrintCentered(0, 20, "GAME PAUSED");
	}

	glEnd2D();
}

void renderBottomScreen()
{
	lcdMainOnBottom();
	vramSetBankC(VRAM_C_LCD);
	vramSetBankD(VRAM_D_SUB_SPRITE);
	REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);

	glBegin2D();

	drawRectangle(bottomBounds);

	glColor(RGB15(0, 31, 31));

	// If I put to much text the screen order will get swapped, the top will become bottom
	// need to be carefull.
	std::string collisionCounterString = "COUNTER: " + std::to_string(collisionCounter);

	Font.PrintCentered(0, 20, collisionCounterString.c_str());

	glEnd2D();
}

void initSubSprites(void);

int main(int argc, char *argv[])
{
	// Mode 5: 2 Static layers + 2 Affine Extended layers. This is the most common mode used since it’s incredibly flexible.
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);

	// init oam to capture 3D scene
	initSubSprites();
	mmInitDefaultMem((mm_addr)soundbank_bin);

	// sub background holds the top image when 3D directed to bottom
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	// Initialize GL in 3d mode
	glScreen2D();

	// Set Bank A to texture (128 kb)
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankE(VRAM_E_TEX_PALETTE); // Allocate VRAM bank for all the palettes

	// Load our font texture
	// We used glLoadSpriteSet since the texture was made
	// with my texture packer.
	// no need to save the return value since
	// we don't need  it at all
	Font.Load(FontImages,																		  // pointer to glImage array
			  FONT_SI_NUM_IMAGES,																  // Texture packer auto-generated #define
			  font_si_texcoords,																  // Texture packer auto-generated array
			  GL_RGB256,																		  // texture type for glTexImage2D() in videoGL.h
			  TEXTURE_SIZE_64,																	  // sizeX for glTexImage2D() in videoGL.h
			  TEXTURE_SIZE_128,																	  // sizeY for glTexImage2D() in videoGL.h
			  GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
			  256,																				  // Length of the palette (256 colors)
			  (u16 *)font_siPal,																  // Palette Data
			  (u8 *)font_siBitmap																  // image data generated by GRIT
	);

	// Do the same with our bigger texture
	FontBig.Load(FontBigImages,
				 FONT_16X16_NUM_IMAGES,
				 font_16x16_texcoords,
				 GL_RGB256,
				 TEXTURE_SIZE_64,
				 TEXTURE_SIZE_512,
				 GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				 256,
				 (u16 *)font_siPal,
				 (u8 *)font_16x16Bitmap);

	// maxmod audio library only support music from mod/it/s3m files extension.
	// load the module
	mmLoad(MOD_FLATOUTLIES);

	// Start playing module
	mmStart(MOD_FLATOUTLIES, MM_PLAY_LOOP);

	// set music volume
	mmSetModuleVolume(200);

	// load sound effects
	mmLoadEffect(SFX_MAGIC);

	collisionSound = {
		{SFX_MAGIC},			 // id
		(int)(1.0f * (1 << 10)), // rate
		0,						 // handle
		100,					 // volume
		255,					 // panning
	};

	// our ever present frame counter
	int frame = 0;

	touchPosition touch;

	while (true)
	{
		// increment frame counter
		frame++;
		// get input
		scanKeys();

		// read the touchscreen coordinates
		touchRead(&touch);

		if (touch.px > 0 && touch.py > 0 && touch.px < SCREEN_WIDTH - bottomBounds.w && touch.py < SCREEN_HEIGHT - bottomBounds.h)
		{
			bottomBounds.x = touch.px;
			bottomBounds.y = touch.py;
		}

		touchBounds.x = touch.px;
		touchBounds.y = touch.py;

		if (hasCollision(touchBounds, bottomBounds))
		{
			bottomBounds.color = RED;
		}
		else
		{
			bottomBounds.color = BLUE;
		}

		int keyDown = keysDown();
		int keyHeld = keysHeld();

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;

			mmEffectEx(&collisionSound);
		}

		if (!isGamePaused)
		{
			update(keyHeld);
		}

		// wait for capture unit to be ready
		while (REG_DISPCAPCNT & DCAP_ENABLE);

		// Alternate rendering every other frame
		// Limits your FPS to 30
		// setting up top and bottom screen
		if ((frame & 1) == 0)
		{
			renderTopScreen();
		}
		else
		{
			renderBottomScreen();
		}

		glFlush(0);
		swiWaitForVBlank();
	}
}

// necessary function for the rendering
// set up a 2D layer construced of bitmap sprites
// this holds the image when rendering to the top screen
void initSubSprites(void)
{
	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);

	int x = 0;
	int y = 0;

	int id = 0;

	// set up a 4x3 grid of 64x64 sprites to cover the screen
	for (y = 0; y < 3; y++)
	{
		for (x = 0; x < 4; x++)
		{
			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			id++;
		}
	}

	swiWaitForVBlank();
	oamUpdate(&oamSub);
}
