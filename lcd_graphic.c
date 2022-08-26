#include "chars.h"
#include "lcd_driver.h"
#include "lcd_graphic.h"
#include <stdio.h>
#include <string.h>

char frame_buffer[8][128];

/*
 * Clears the entire LCD display.
 */
void clear_screen(void)
{
    int i, j;
    for (i = 0; i < FRAME_HEIGHT; i++)
    {
        for (j = 0; j < FRAME_WIDTH; j++)
        {
            frame_buffer[i][j] = 0;
        }
    }

    refresh_buffer();
}

/*
 * Writes the contents of the frame buffer to the LCD display.
 */
void refresh_buffer(void)
{
    int i, j;
    for (i = 0; i < FRAME_HEIGHT; i++)
    {
        set_mode(1);

        // Set page address
        spim_write(0x00B0 | i);
        // Set column address
        spim_write(0x0000);
        spim_write(0x0010);

        set_mode(0);
        for (j = 0; j < FRAME_WIDTH; j++)
            spim_write(frame_buffer[i][j]);
    }
}

/*
 * Draws a line starting at (x, y) with the given color to (x, y+length) if
 * vert, else to (x+length, y) to the frame buffer.
 *
 * x: x coordinate of line start.
 * y: y coordinate of line start.
 * length: length of line.
 * color: color of line (0 for white, 1 for black).
 * vert: orientation of line (0 for horizontal, 1 for vertical).
 */
void LCD_line(int x, int y, int length, int color, int vert)
{
    int  x_start, x_end, y_start, y_end;
    int  i, page;
    char mask;

    if (vert)
    {
        y_start = y;
        y_end   = y + length;

        for (i = y_start; i < y_end; i++)
        {
            page = i >> 3; // y/8
            mask = 0x01 << (i % 8);
            if (color)
                frame_buffer[page][x] |= mask;
            else
                frame_buffer[page][x] &= ~mask;
        }
    }
    else
    {
        x_start = x;
        x_end   = x + length;

        page = y >> 3; // y/8
        mask = 0x01 << (y % 8);
        for (i = x_start; i < x_end; i++)
        {
            if (color)
                frame_buffer[page][i] |= mask;
            else
                frame_buffer[page][i] &= ~mask;
        }
    }
}

/*
 * Draws a width x height rectangle with the top left corner at (x, y) to the
 * frame buffer.
 *
 * x1: x coordinate of top left corner.
 * y1: y coordinate of top left corner.
 * width: width of rectangle.
 * height: height of rectangle.
 * color: color of rectangle (o for white, 1 for black).
 * fill: 1 if rectangle should be filled in, 0 to only draw rectangle outline.
 */
void LCD_rect(int x1, int y1, int width, int height, int color, int fill)
{
    int x2 = x1 + width;
    int y2 = y1 + width;
    int i;

    if (!fill)
    {
        LCD_line(x1, y1, width, color, 0);
        LCD_line(x1, y2, width, color, 0);
        LCD_line(x1, y1, height, color, 1);
        LCD_line(x2, y1, height, color, 1);
    }
    else
    {
        for (i = y1; i <= y2; i++)
            LCD_line(x1, i, width, color, 0);
    }
}

/*
 * Writes a string to the frame buffer starting at the given row.
 *
 * str: the string to write.
 * page: the row on the LCD to start writing at.
 */
void LCD_text(char * str, int page)
{
    int i, j, c, offset = 0, len = strlen(str);
    if (len > (8 - page) * 16)
    {
        printf("String is too long for LCD display.\n");
        return;
    }

    for (i = 0; i < len; i++)
    {
        if (offset >= FRAME_WIDTH)
        {
            page++;
            offset = 0;
        }
        c = (unsigned int)str[i];
        for (j                             = 0; j < 8; j++)
            frame_buffer[page][offset + j] = chars[c][j];
        offset += 8;
    }
}
