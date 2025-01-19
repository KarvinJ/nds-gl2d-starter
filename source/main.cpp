#include "graphics.h"
#include <iostream>

// sounds
#include <maxmod9.h>
#include "soundbank.h"
#include "soundbank_bin.h"

// fonts
#include "Cglfont.h"
#include "font_si.h"
#include "font_16x16.h"

// Texture Packer auto-generated UV coords
#include "uvcoord_font_si.h"
#include "uvcoord_font_16x16.h"

#include "shuttle.h" // load shuttle png data

// This imageset would use our texture packer generated coords so it's kinda safe and easy to use
// FONT_SI_NUM_IMAGES is a value #defined from "uvcoord_font_si.h"
glImage FontImages[FONT_SI_NUM_IMAGES];
glImage FontBigImages[FONT_16X16_NUM_IMAGES];
glImage shuttleImage[1]; // single image

// Our fonts
Cglfont Font;
Cglfont FontBig;

bool isGamePaused;

int collisionCounter;

mm_sound_effect collisionSound;

Rectangle player = {HALF_WIDTH, HALF_HEIGHT, 64, 64, WHITE};
Rectangle bottomBounds = {HALF_WIDTH, HALF_HEIGHT, 32, 32, GREEN};
Rectangle ball = {HALF_WIDTH - 50, HALF_HEIGHT, 20, 20, WHITE};
Rectangle touchBounds = {0, 0, 8, 8, WHITE};

const int PLAYER_SPEED = 5;

int ballVelocityX = 2;
int ballVelocityY = 2;

void update()
{
	int keyHeld = keysHeld();

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

		// Play sound effects
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

	drawRectangle(ball);

	// draw image
	glSprite(player.x, player.y, GL_FLIP_NONE, shuttleImage);

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

	// If I put to much text the screen order will get swapped, the top will become bottom need to be careful.
	std::string collisionCounterString = "COUNTER: " + std::to_string(collisionCounter);

	Font.PrintCentered(0, 20, collisionCounterString.c_str());

	glEnd2D();
}

int main(int argc, char *argv[])
{
	// Mode 5: 2 Static layers + 2 Affine Extended layers. This is the most common mode used since it’s incredibly flexible.
	// Set video mode top screen
	videoSetMode(MODE_5_3D);
	// Set video mode bottom screen
	videoSetModeSub(MODE_5_2D);

	// Set Bank A to texture (128 kb)
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankE(VRAM_E_TEX_PALETTE); // Allocate VRAM bank for all the palettes

	// init oam to capture 3D scene
	initSubSprites();

	// sub background holds the top image when 3D directed to bottom
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	// Initialize GL in 3d mode
	glScreen2D();

	// The png file has to be a power of 2 dimensions 8, 16, 32, 64, etc...
	glLoadTileSet(shuttleImage,	   // pointer to glImage array
				  64,			   // sprite width
				  64,			   // sprite height
				  64,			   // bitmap image width
				  64,			   // bitmap image height
				  GL_RGB16,		   // texture type for glTexImage2D() in videoGL.h
				  TEXTURE_SIZE_64, // sizeX for glTexImage2D() in videoGL.h
				  TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				  GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				  16,				  // Length of the palette to use (16 colors)
				  (u16 *)shuttlePal,  // Load our 256 color tiles palette
				  (u8 *)shuttleBitmap // image data generated by GRIT
	);

	Font.Load(FontImages,
			  FONT_SI_NUM_IMAGES,
			  font_si_texcoords,
			  GL_RGB256,
			  TEXTURE_SIZE_64,
			  TEXTURE_SIZE_128,
			  GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
			  256,
			  (u16 *)font_siPal,
			  (u8 *)font_siBitmap);

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

	// maxmod audio library only support music from mod/it/s3m files extension. load the module
	// line of code to initialize audio
	mmInitDefaultMem((mm_addr)soundbank_bin);

	// load music in .mod format
	mmLoad(MOD_FLATOUTLIES);

	// Start playing module
	mmStart(MOD_FLATOUTLIES, MM_PLAY_LOOP);

	// set music volume
	mmSetModuleVolume(200);

	// load sound effects
	mmLoadEffect(SFX_MAGIC);

	// setting up sound
	collisionSound = {
		{SFX_MAGIC},			 // id
		(int)(1.0f * (1 << 10)), // rate
		0,						 // handle
		100,					 // volume
		255,					 // panning
	};

	// our ever present frame counter
	int frameCounter = 0;

	touchPosition touch;

	while (true)
	{
		// get all nintendo ds inputs
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

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;

			mmEffectEx(&collisionSound);
		}

		if (!isGamePaused)
		{
			update();
		}

		// Alternate rendering every other frame. This limits your FPS to 30
		// setting up top and bottom screen
		if ((frameCounter & 1) == 0)
		{
			renderTopScreen();
		}
		else
		{
			renderBottomScreen();
		}

		frameCounter++; // increment frame counter

		glFlush(0);
		swiWaitForVBlank();
	}
}
