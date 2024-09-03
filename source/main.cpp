#include <nds.h>
#include <stdio.h>
#include <gl2d.h>

// some useful defines
#define HALF_WIDTH (SCREEN_WIDTH / 2)
#define HALF_HEIGHT (SCREEN_HEIGHT / 2)
#define BRAD_PI (1 << 14)

const int WHITE = RGB15(255, 255, 255);
const int RED = RGB15(255, 0, 0);
const int GREEN = RGB15(0, 255, 0);
const int BLUE = RGB15(0, 0, 255);

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

Rectangle player = {HALF_WIDTH, HALF_HEIGHT, 32, 32, WHITE};
Rectangle ball = {HALF_WIDTH - 50, HALF_HEIGHT, 20, 20, WHITE};

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

		if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
		{
			ballVelocityX *= -1;

			ball.color = GREEN;
		}

		else if (ball.y < 0 || ball.y > SCREEN_HEIGHT - ball.h)
		{
			ballVelocityY *= -1;

			ball.color = RED;
		}

		else if (hasCollision(player, ball))
		{
			ballVelocityX *= -1;
			ballVelocityY *= -1;

			ball.color = BLUE;
		}

		ball.x += ballVelocityX;
		ball.y += ballVelocityY;

		// wait for capture unit to be ready
		while (REG_DISPCAPCNT & DCAP_ENABLE)
			;

		// Alternate rendering every other frame
		// Limits your FPS to 30
		// setting up top and bottom screen
		if ((frame & 1) == 0)
		{
			lcdMainOnTop();
			vramSetBankD(VRAM_D_LCD);
			vramSetBankC(VRAM_C_SUB_BG);
			REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);
		}
		else
		{
			lcdMainOnBottom();
			vramSetBankC(VRAM_C_LCD);
			vramSetBankD(VRAM_D_SUB_SPRITE);
			REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);
		}

		if ((frame & 1) == 0)
		{
			glBegin2D();

			drawRectangle(player);
			drawRectangle(ball);

			glEnd2D();
		}
		else
		{
			glBegin2D();

			drawRectangle(player);

			glEnd2D();
		}

		glFlush(0);
		swiWaitForVBlank();
	}

	return 0;
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
