#include "lcd_graphic.h"

typedef struct bullet {	
	int x;
	int y;
} Bullet ;

void InitializeBullet(Bullet * bullet, int x, int y){
	bullet -> x = x;
	bullet -> y = y;
	LCD_line(x, y, 2, 1, 1);
}

void moveBullet(Bullet *bul, int x) {
	LCD_line(bul->x, bul->y, 2, 0, 1);
	bul->x = bul->x + x;
	LCD_line(bul->x, bul->y, 2, 1, 1);
}
