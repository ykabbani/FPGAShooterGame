#include "lcd_graphic.h"

typedef struct car {	
	int x;
	int y;
	int length;
	int fill;
	int alive;
} Car ;

void initializeCar(Car * car, int x, int y, int length, int fill, int alive){
	car -> x = x;
	car -> y = y;
	car -> length = length;
	car -> fill = fill;
	car -> alive = alive;
	LCD_rect(x, y, length, length, 1, fill);
}

void moveCarX(Car *vehicle, int x) {
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 0, vehicle->fill);
	vehicle->x = x;
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 1, vehicle->fill);
}

void moveCarY(Car *vehicle, int y) {
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 0, vehicle->fill);
	vehicle->y = y; 
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 1, vehicle->fill);
}

void incrementCarY(Car *vehicle, int y) {
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 0, vehicle->fill);
	vehicle->y = vehicle->y + y; 
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 1, vehicle->fill);
}

int getCarYPosition(Car *vehicle){
	return vehicle->y;
}

int getCarXPosition(Car *vehicle){
	return vehicle->x;
}

void setCarStatus(Car *vehicle, int status){
	vehicle->alive = status;
}

void hideCar(Car *vehicle){
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 0, vehicle->fill);
}

void showCar(Car *vehicle){
	LCD_rect(vehicle->x, vehicle->y, vehicle->length, vehicle->length, 1, vehicle->fill);
}

int getCarStatus(Car *vehicle){
	return vehicle->alive;
}
