#include "lcd_driver.h"
#include "lcd_graphic.h"
#include "address_map_arm.h"
#include "GSInterface.h"
#include "car.h"
#include "bullet.h"
#include <time.h>

#define BULLET_CAPACITY 2
#define ENEMY_FREQUENCY 7 //3 seconds 30*0.1s
#define ENEMY_SPEED 1 //0.2 seconds 2*0.1s
#define ENEMY_INSTANCE 10
#define ENEMY_LENGTH 7
#define CAR_LENGTH 11
// Initialize Border Dimensions
#define BORDER_WIDTH 2
#define BORDER_DISTANCE 24
#define MAX_HEALTH 10


volatile int * HPS_timer_ptr = (int *)HPS_TIMER0_BASE; // timer base address
volatile int * Button_ptr = (int *)KEY_BASE; // Push button addresses
volatile int * LED_ptr = (int *)LED_BASE; // LED address

volatile char lookUpTable_1_10[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67}; //look up table for the 7 segment display
volatile int * Segment3_0_ptr = (int *)HEX3_HEX0_BASE; // lower order 7 segment displays (0-3)
volatile int * Segment5_4_ptr = (int *)HEX5_HEX4_BASE; // higher order 7 segment displays (4-5)

/*******************************************************************************
 * The program performs the following:
 * 1. Writes INTEL FPGA COMPUTER SYSTEMS to the top of the LCD.
 * 2. Bounces a filled in rectangle around the display and off the displayed
 * text.
 ******************************************************************************/

int ReadButton()
{
	int Button_value = *(Button_ptr); //gets button value
	return Button_value; //returns last 2 bits of the buttons
}

void config_HPS_timer()
{
    *(HPS_timer_ptr + 0x2) = 0; // write to control register to stop timer
    /* set the timer period */
    int counter = 10000000; // timeout = 1/(100MHz) x 1x10^7 = 0.1 sec
    *(HPS_timer_ptr) = counter; // write to timer load register
    /* write to control register to start timer, with interrupts */
    *(HPS_timer_ptr + 2) = 0b011; // int mask = 0, mode = 1, enable = 1
}

void GSInit(void)
{
    // Configure for full resolution, +/- 2G operation
    WriteGSRegister(GS_DATA_FORMAT, 0x08);
    // Configure for 200Hz sampling rate.  Sampling rate of 10 == 100Hz.  Each increment after that is a doubling.
    WriteGSRegister(GS_BW_RATE, 0x0B);
    // Configure to begin measurement
    WriteGSRegister(GS_POWER_CTL, 0x08);
    // LED address initialize
    
}

int linearGyroReading(int value, int sign){
    int temp_reading;
    if (sign>0){ //negative
        temp_reading = value;
        if (temp_reading < 127) {
            temp_reading = 127;
            return temp_reading;
        }
        else return temp_reading;
    } else { //positive
        temp_reading = 255 + value;
        if (temp_reading > 383) {
            temp_reading = 383;
            return temp_reading;
        }
        else return temp_reading;
    }
}

// Convert linear gyro reading to value within range of borders
int gyroXReadingToCarPosition(int linearGyroReading, int carLength, int borderWidth, int borderDist){
    double y1 = borderDist + borderWidth + 1;
    double x1 = 127;
    double y2 = SCREEN_WIDTH - borderDist - borderWidth - carLength;
    double x2 = 383;
    double slope = (y2 - y1)/(x2 - x1);
    double intercept = y1 - (slope*x1);
    return (slope*linearGyroReading + intercept);
}

void updateHealth (int health) {
    int leds = 0b1111111111;
    int i = 0;
    /*
    for (i; i <= health; i++) {
        leds*=leds;
    }
    leds--;*/
    for (i; i < (MAX_HEALTH - health); i++){
        leds = leds << 1;
    }
    *(LED_ptr) = leds;
}

// Random Number Generator
int randomNumber(int lower, int upper){
    int num = ((rand() % (upper - lower + 1)) + lower);
    return num;
}

void updateScore(int value1, int value2)
{
	* Segment3_0_ptr = ((lookUpTable_1_10[value1 % 10] << 24) | (lookUpTable_1_10[(value2/100) % 10] << 16) | (lookUpTable_1_10[(value2/10) % 10] << 8) | lookUpTable_1_10[value2 % 10]);
	* Segment5_4_ptr = ((lookUpTable_1_10[(value1/100) % 10] << 8) | (lookUpTable_1_10[(value1/10) % 10]));
}

int main(void) {

    // Initialize the SPI interface
    init_spim0();
    // Initialize the I2C interface
    I2C0Init();
    // Initialize the GS unit for sampling
    GSInit();
    // Initialize the LCD
    init_lcd();

    int highScore = 0;
 
    startGame:

    // Check that the gyro is available
    if (ReadGSRegister(GS_DEVID) == 0xE5) {
        
        srand(time(0));

        // Initialize the gyro data array
        unsigned char data[6];

        // Configure HPS Timer
        config_HPS_timer();

        clear_screen();
        
        // Initialize buttons
        int buttons = 0;

        // Initialize bulletOnScreen
        int bulletOnScreen = 0;

        // Initialize player's life status
        int playerAlive = 1;

        // Make player's Car object
        Car myCar;                        // or struct Car myCar?????

        int car_x;
        int car_y = SCREEN_HEIGHT - CAR_LENGTH;
        
        int highScore = 0;
        int playerScore = 0;
        int playerHealth = 10;

        // Enemy initialization
        int spawnTimePeriod = 0;
        int enemySpeedTimePeriod = 0;

        Car enemies[ENEMY_INSTANCE];

        int i = 0;
        for (i; i < ENEMY_INSTANCE; i++){
            setCarStatus(&enemies[i], 0);
        }

        // FINDING RANGE FOR LANES
        int minRange = BORDER_DISTANCE + BORDER_WIDTH + 1;
        int maxRange = SCREEN_WIDTH - BORDER_DISTANCE - BORDER_WIDTH - ENEMY_LENGTH;

        updateHealth(playerHealth);

        int playerScore = 0;

        /////////////////////////////////////////////////////////////////////////////////////////////////

        // Check if gyro is done sampling
        if (((ReadGSRegister(GS_INT_SOURCE) >> 7) & 1)) {

            
            // Reading and storing gyro readings
            MultiReadGS(GS_DATAX0, data, 6);
            
            // Initialize player's Car position
            
            car_x = gyroXReadingToCarPosition(linearGyroReading(data[0], data[1]), CAR_LENGTH, BORDER_WIDTH, BORDER_DISTANCE);
            initializeCar(&myCar, car_x, car_y, CAR_LENGTH, 1, 1);

            // Drawing game borders
            LCD_line((BORDER_DISTANCE + 1),0,SCREEN_HEIGHT,1,1);
            LCD_line((BORDER_DISTANCE + BORDER_WIDTH),0,SCREEN_HEIGHT,1,1);
            LCD_line((SCREEN_WIDTH - BORDER_DISTANCE - 1),0,SCREEN_HEIGHT,1,1);
            LCD_line((SCREEN_WIDTH - BORDER_DISTANCE - BORDER_WIDTH),0,SCREEN_HEIGHT,1,1);

            // Refresh screen
            refresh_buffer();
        }
        
        while (1) {
            if (playerAlive){
                if (playerHealth == 0) {
                    playerAlive = 0;
                    LCD_rect(10, 10, SCREEN_WIDTH - (2*10), SCREEN_HEIGHT - (2*10), 0, 1);
                    refresh_buffer();
                    LCD_rect(10, 10, SCREEN_WIDTH - (2*10), SCREEN_HEIGHT - (2*10), 1, 0);
                    refresh_buffer();
                    char text_top_lcd[17]       = "    GAME OVER!  \0";
                    char text_mid_lcd[17]       = " PRESS A BUTTON \0";
                    char text_bottom_lcd[17]    = "   TO RESTART   \0";
                    LCD_text(text_top_lcd, 1);
                    LCD_text(text_mid_lcd, 2);
                    LCD_text(text_bot_lcd, 3);
                    refresh_buffer();
                } else {
                    //check when timer's counter is triggered and reset timer
                    if (*(HPS_timer_ptr + 4) == 1){

                        // Check if gyro is done sampling
                        if (((ReadGSRegister(GS_INT_SOURCE) >> 7) & 1)) {
                            
                            // Reading and storing gyro readings
                            MultiReadGS(GS_DATAX0, data, 6);

                            // Update car x position
                            int tempCar_x = car_x;
                            car_x = gyroXReadingToCarPosition(linearGyroReading(data[0], data[1]), CAR_LENGTH, BORDER_WIDTH, BORDER_DISTANCE);
                            
                            if (tempCar_x != car_x){
                                moveCarX(&myCar, car_x);
                            }
                        }

                        ////////////////////////////////////////////////////////////////// SPAWN ENEMIES
                        
                        // Increment until one second interval
                        if (spawnTimePeriod < ENEMY_FREQUENCY) spawnTimePeriod++;

                        // Check for one second interval, spawn enemy, reset timer
                        if (spawnTimePeriod == ENEMY_FREQUENCY) {
                            spawnTimePeriod = 0;
                            
                            // Checking if there are 3 enemies on screen and if not adding one
                            int i = 0;
                            for (i; i < ENEMY_INSTANCE; i++){
                                if (getCarStatus(&enemies[i]) == 0){
                                    int enemyX = randomNumber(minRange, maxRange);
                                    initializeCar(&enemies[i], enemyX, 1, ENEMY_LENGTH, 1,1);
                                    showCar(&enemies[i]);
                                    break;
                                }
                            }
                            if (i == ENEMY_INSTANCE) {
                                spawnTimePeriod = ENEMY_FREQUENCY;
                            }
                        }

                        ///////////////////////////////////////////////////////////////// MOVE ENEMY CARS
                        if (enemySpeedTimePeriod < ENEMY_SPEED) enemySpeedTimePeriod++;

                        // Check for interval, move, reset timer
                        if (enemySpeedTimePeriod == ENEMY_SPEED) {
                            enemySpeedTimePeriod = 0;
                            
                            // Checking if there are 3 enemies on screen and if not adding one
                            int i = 0;
                            for (i; i < ENEMY_INSTANCE; i++){
                                if (getCarStatus(&enemies[i]) == 1){
                                    //check if car hit the bottom or hit the player
                                    int enemyX = getCarXPosition(&enemies[i]);
                                    int enemyY = getCarYPosition(&enemies[i]);
                                    if ( (enemyY > SCREEN_HEIGHT) ) {
                                        setCarStatus(&enemies[i], 0);
                                        hideCar(&enemies[i]);
                                        showCar(&myCar);
                                        playerScore++;
                                        if (playerScore > highScore) highScore = playerScore;
                                        updateScore(highScore, playerScore);
                                    } else if ( (((enemyX + ENEMY_LENGTH) >= car_x) && (enemyX <= car_x + CAR_LENGTH)) && ((enemyY + ENEMY_LENGTH) >= (car_y - 1)) ){
                                        setCarStatus(&enemies[i], 0);
                                        hideCar(&enemies[i]);
                                        showCar(&myCar);
                                        playerHealth--;
                                        updateHealth(playerHealth);
                                    } else {
                                        incrementCarY(&enemies[i],2);

                                    }
                                }
                            }
                            if (i == ENEMY_INSTANCE) {
                                enemySpeedTimePeriod = ENEMY_SPEED;
                            }
                        }
                        /////////////////////////////////////////////////////////////////
                        
                        // Read buttons
                        buttons = ReadButton();

                        // Reset timer
                        // Refresh screen buffer
                        refresh_buffer();
                        int temp = *(HPS_timer_ptr + 3);
                    }
                }  
            }

            // If button is pressed
            if (((buttons)&1) && (playerAlive == 0)) {
                clear_screen();
                goto startGame;
            }
        }
    }
}

