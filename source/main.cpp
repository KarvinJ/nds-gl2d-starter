#include <nds.h>
#include <stdio.h>
#include <gl2d.h>

// some useful defines
#define HALF_WIDTH (SCREEN_WIDTH / 2)
#define HALF_HEIGHT (SCREEN_HEIGHT / 2)
#define BRAD_PI (1 << 14)

const int WHITE = RGB15(255, 255, 255);

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

Rectangle player = {HALF_WIDTH, HALF_HEIGHT, 32, 32, WHITE};

const int PLAYER_SPEED = 5;

void drawRectangle(Rectangle rectangle)
{
	glBoxFilled(rectangle.x, rectangle.y, rectangle.x + rectangle.w, rectangle.y + rectangle.h, rectangle.color);
}

void lines(int frame);

// Example file function to set up
// dual screen
void initSubSprites(void);

int main(int argc, char *argv[])
{
	// Mode 5: 2 Static layers + 2 Affine Extended layers. This is the most common mode used since it’s incredibly flexible.
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);

	// init oam to capture 3D scene
	initSubSprites();

	// sub background holds the top image when 3D directed to bottom
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	// Initialize GL in 3d mode
	glScreen2D();

	// our ever present frame counter
	int frame = 0;

	int key;	 // for key input down
	int keyHeld; // for key input down

	while (1)
	{
		// increment frame counter
		frame++;

		// get input
		scanKeys();
		key = keysDown();
		keyHeld = keysHeld();

		if (key & KEY_START)
			break;

		if (keyHeld & KEY_UP && player.y > 0)
		{
			player.y -= PLAYER_SPEED;
		}

		else if (keyHeld & KEY_DOWN && player.y < SCREEN_HEIGHT - player.h)
		{
			player.y += PLAYER_SPEED;
		}
		// process input
		else if (keyHeld & KEY_RIGHT && player.x < SCREEN_WIDTH - player.w)
		{
			player.x += PLAYER_SPEED;
		}

		else if (keyHeld & KEY_LEFT && player.x > 0)
		{
			player.x -= PLAYER_SPEED;
		}

		// wait for capture unit to be ready
		while (REG_DISPCAPCNT & DCAP_ENABLE)
			;

		// Alternate rendering every other frame
		// Limits your FPS to 30
		// setting up top and bottom screen
		if ((frame & 1) == 0)
		{
			lcdMainOnBottom();
			vramSetBankC(VRAM_C_LCD);
			vramSetBankD(VRAM_D_SUB_SPRITE);
			REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);
		}
		else
		{
			lcdMainOnTop();
			vramSetBankD(VRAM_D_LCD);
			vramSetBankC(VRAM_C_SUB_BG);
			REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);
		}

		if ((frame & 1) == 0)
		{
			glBegin2D();

			drawRectangle(player);

			glEnd2D();
		}
		else
			lines(frame);

		glFlush(0);
		swiWaitForVBlank();
	}

	return 0;
}

// oldskool lines demo
void lines(int frame)
{

	// Do some funky color cycling
	int red = abs(sinLerp(frame * 220) * 31) >> 12;
	int green = abs(sinLerp(frame * 140) * 31) >> 12;
	int blue = abs(sinLerp(frame * 40) * 31) >> 12;

	// set up GL2D for 2d mode
	glBegin2D();

	// draw a bunch (4096/32) of colored lines
	// using some funky trig equations
	int i;
	for (i = frame; i < ((1 << 12) + frame); i += 32)
	{
		int px = ((sinLerp(frame * 130) * 130) >> 12) * cosLerp((i * 100));
		int py = ((sinLerp(frame * 280) * 70) >> 12) * sinLerp((i * 200));
		int px2 = ((sinLerp(frame * 330) * 100) >> 12) * cosLerp(((i * 300 + BRAD_PI)));
		int py2 = ((sinLerp(frame * 140) * 80) >> 12) * sinLerp(((i * 400 + BRAD_PI)));
		glLine(HALF_WIDTH + (px >> 12), HALF_HEIGHT + (py >> 12),
			   HALF_WIDTH + (px2 >> 12), HALF_HEIGHT + (py2 >> 12),
			   RGB15(red, green, blue));
		glLine(HALF_WIDTH + (py2 >> 12), HALF_HEIGHT + (px >> 12),
			   HALF_WIDTH + (py >> 12), HALF_HEIGHT + (px2 >> 12),
			   RGB15(green, blue, red));
	}

	glEnd2D();
}

// necessary function for the rendering
//-------------------------------------------------------
// set up a 2D layer construced of bitmap sprites
// this holds the image when rendering to the top screen
//-------------------------------------------------------
void initSubSprites(void)
{
	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);

	int x = 0;
	int y = 0;

	int id = 0;

	// set up a 4x3 grid of 64x64 sprites to cover the screen
	for (y = 0; y < 3; y++)
		for (x = 0; x < 4; x++)
		{
			oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			id++;
		}

	swiWaitForVBlank();

	oamUpdate(&oamSub);
}
