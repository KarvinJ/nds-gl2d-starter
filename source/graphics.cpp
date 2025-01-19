#include "graphics.h"

void drawRectangle(Rectangle &rectangle)
{
	glBoxFilled(rectangle.x, rectangle.y, rectangle.x + rectangle.w, rectangle.y + rectangle.h, rectangle.color);
}

bool hasCollision(Rectangle &bounds, Rectangle &ball)
{
	return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
		   bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}

// necessary function for the rendering
// set up a 2D layer construced of bitmap sprites
// this holds the image when rendering to the top screen
void initSubSprites()
{
	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);

	int x = 0;
	int y = 0;

	int id = 0;

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