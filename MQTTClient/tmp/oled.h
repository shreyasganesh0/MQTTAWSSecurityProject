// #ifndef __OLED_H__
// #define __OLED_H__

// /***************************************************************************************************/
// /* Include Files */
// /***************************************************************************************************/
// #include <stdint.h>
// #include <stdbool.h>

// //------------------------------ Definitions ---------------------------------//

// #ifndef SSD1306_I2C_ADDRESS
//   #define SSD1306_I2C_ADDRESS   0x78
// #endif

// #if !defined SSD1306_128_32 && !defined SSD1306_96_16
// #define SSD1306_128_64
// #endif
// #if defined SSD1306_128_32 && defined SSD1306_96_16
//   #error "Only one SSD1306 display can be specified at once"
// #endif

// #if defined SSD1306_128_64
//   #define SSD1306_LCDWIDTH            128
//   #define SSD1306_LCDHEIGHT            64
// #endif
// #if defined SSD1306_128_32
//   #define SSD1306_LCDWIDTH            128
//   #define SSD1306_LCDHEIGHT            32
// #endif
// #if defined SSD1306_96_16
//   #define SSD1306_LCDWIDTH             96
//   #define SSD1306_LCDHEIGHT            16
// #endif

// #define SSD1306_SETCONTRAST          0x81
// #define SSD1306_DISPLAYALLON_RESUME  0xA4
// #define SSD1306_DISPLAYALLON         0xA5
// #define SSD1306_NORMALDISPLAY        0xA6
// #define SSD1306_INVERTDISPLAY_       0xA7
// #define SSD1306_DISPLAYOFF           0xAE
// #define SSD1306_DISPLAYON            0xAF
// #define SSD1306_SETDISPLAYOFFSET     0xD3
// #define SSD1306_SETCOMPINS           0xDA
// #define SSD1306_SETVCOMDETECT        0xDB
// #define SSD1306_SETDISPLAYCLOCKDIV   0xD5
// #define SSD1306_SETPRECHARGE         0xD9
// #define SSD1306_SETMULTIPLEX         0xA8
// #define SSD1306_SETLOWCOLUMN         0x00
// #define SSD1306_SETHIGHCOLUMN        0x10
// #define SSD1306_SETSTARTLINE         0x40
// #define SSD1306_MEMORYMODE           0x20
// #define SSD1306_COLUMNADDR           0x21
// #define SSD1306_PAGEADDR             0x22
// #define SSD1306_COMSCANINC           0xC0
// #define SSD1306_COMSCANDEC           0xC8
// #define SSD1306_SEGREMAP             0xA0
// #define SSD1306_CHARGEPUMP           0x8D
// #define SSD1306_EXTERNALVCC          0x01
// #define SSD1306_SWITCHCAPVCC         0x02

// // Scrolling #defines
// #define SSD1306_ACTIVATE_SCROLL                      0x2F
// #define SSD1306_DEACTIVATE_SCROLL                    0x2E
// #define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3
// #define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
// #define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
// #define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
// #define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A

// #define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }

// uint8_t _i2caddr, _vccstate, x_pos, y_pos, text_size;
// bool wrap;

// typedef enum {
//   WELCOME = 0,
//   IP_INFO,
//   ENV_INFO
// } ssd1306_screen_t;

// typedef struct {
//   ssd1306_screen_t state;
//   TaskHandle_t thread;
// } ssd1306_t;

// /**
//  * @brief Send command to the SSD1306 over I2C
//  * 
//  * @param c Command to send
//  */
// void ssd1306_command(uint8_t c);
// /**
//  * @brief Send initialization sequence to the SSD1306 over I2C
//  * 
//  * @param vccstate 
//  * @param i2caddr The I2C addr of the OLED without the shift
//  * @note The I2C addr on the SSD1306 is either 0x3C or 0x3D if you look at the screen it will be 0x78 or 0x7A
//  *       this is the shifted value (i.e. 0x3C << 1) because of the I2C protocol.
//  */
// void SSD1306_Begin(uint8_t vccstate, uint8_t i2caddr);
// /**
//  * @brief Draw a pixel at the specified coordinates
//  * 
//  * @param x The x location on the OLED
//  * @param y The y location on the OLED
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool color);
// /**
//  * @brief Have the OLED scroll to the right
//  * 
//  * @param start Starting point for scroll
//  * @param stop Stopping point for scroll
//  */
// void SSD1306_StartScrollRight(uint8_t start, uint8_t stop);
// /**
//  * @brief 
//  * 
//  * @param start Starting point for scroll
//  * @param stop Stopping point for scroll
//  */
// void SSD1306_StartScrollLeft(uint8_t start, uint8_t stop);
// /**
//  * @brief 
//  * 
//  * @param start Starting point for scroll
//  * @param stop Stopping point for scroll
//  */
// void SSD1306_StartScrollDiagRight(uint8_t start, uint8_t stop);
// /**
//  * @brief 
//  * 
//  * @param start Starting point for scroll
//  * @param stop Stopping point for scroll
//  */
// void SSD1306_StartScrollDiagLeft(uint8_t start, uint8_t stop);
// /**
//  * @brief Stop the OLED from scrolling
//  * 
//  */
// void SSD1306_StopScroll(void);
// /**
//  * @brief Dim the OLED display
//  * 
//  * @param dim Dim status
//  *            - true dim dipslay
//  *            - false set display to normal brightness
//  */
// void SSD1306_Dim(bool dim);
// /**
//  * @brief Display the buffer on the OLED
//  * @note This function must be called after any draw function
//  */
// void SSD1306_Display(void);
// /**
//  * @brief Clear the display
//  * Draws a rectangle with no color to all the display
//  */
// void SSD1306_ClearDisplay(void);
// /**
//  * @brief Draw a line on the display
//  * 
//  * @param x0 Starting x coordinate for the line
//  * @param y0 Starting y coordinate for the line
//  * @param x1 Ending x coordinate for the line
//  * @param y1 Ending y coordinate for the line
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color);
// /**
//  * @brief Draw a horizontal line using burst I2C transmission
//  * 
//  * @param x X coordinate of the line
//  * @param y y coordinate of the line
//  * @param w horizontal width of the line
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_DrawFastHLine(uint8_t x, uint8_t y, uint8_t w, bool color);
// /**
//  * @brief Draw a verticel line using burst I2C transmission
//  * 
//  * @param x x coordinate of the line
//  * @param y y coordinate of the line
//  * @param h verticle hight of the line
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_DrawFastVLine(uint8_t x, uint8_t y, uint8_t h, bool color);
// /**
//  * @brief Draw a filled in rectangle
//  * 
//  * @param x lower left x coordinate of the rectangle
//  * @param y lower left y coordinate of the rectangle
//  * @param w horizontal width of the rectangle
//  * @param h verticle height of the rectangle
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color);
// /**
//  * @brief Fill the screen of the OLED with one color
//  * 
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_FillScreen(bool color);
// /**
//  * @brief Draws a circle at the specified coordinates
//  * 
//  * @param x0 center point of the circle x value
//  * @param y0 center point of the circle y value
//  * @param r radius of the circle in pixels
//  */
// void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r);
// /**
//  * @brief Helper function for the draw circle function
//  * 
//  * @param x0 
//  * @param y0 
//  * @param r 
//  * @param cornername 
//  */
// void SSD1306_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername);
// /**
//  * @brief Fill a circle
//  * 
//  * @param x0 center point of the circle x value
//  * @param y0 center point of the circle y value
//  * @param r radius of the circle in pixels
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_FillCircle(int16_t x0, int16_t y0, int16_t r, bool color);
// /**
//  * @brief Function for helping draw circle
//  * 
//  * @param x0 
//  * @param y0 
//  * @param r 
//  * @param cornername 
//  * @param delta 
//  * @param color 
//  */
// void SSD1306_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, bool color);
// /**
//  * @brief Draw rectangle
//  * 
//  * @param x lower left x coordinate of the rectangle
//  * @param y lower left y coordinate of the rectangle
//  * @param w horizontal width of the rectangle
//  * @param h verticle height of the rectangle
//  */
// void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
// /**
//  * @brief Draw a rectangle with rounded corners
//  * 
//  * @param x lower left x coordinate of the rectangle
//  * @param y lower left y coordinate of the rectangle
//  * @param w horizontal width of the rectangle
//  * @param h verticle height of the rectangle
//  * @param r radius of the corners
//  */
// void SSD1306_DrawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r);
// /**
//  * @brief Draw filled rectangle with rounded corners
//  * 
//  * @param x lower left x coordinate of the rectangle
//  * @param y lower left y coordinate of the rectangle
//  * @param w horizontal width of the rectangle
//  * @param h verticle height of the rectangle
//  * @param r radius of the corners
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_FillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, bool color );
// /**
//  * @brief Draw triangle
//  * 
//  * @param x0 First vertex of the triangle x value
//  * @param y0 First vertex of the triangle y value
//  * @param x1 Second vertex of the triangle x value
//  * @param y1 Second vertex of the triangle y value
//  * @param x2 Third vertex of the triangle x value
//  * @param y2 Third vertex of the triangle y value
//  */
// void SSD1306_DrawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
// /**
//  * @brief Fill the triangle
//  * 
//  * @param x0 First vertex of the triangle x value
//  * @param y0 First vertex of the triangle y value
//  * @param x1 Second vertex of the triangle x value
//  * @param y1 Second vertex of the triangle y value
//  * @param x2 Third vertex of the triangle x value
//  * @param y2 Third vertex of the triangle y value
//  * @param color true for white (draw) false for black (clear)
//  */
// void SSD1306_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool color);
// /**
//  * @brief Draw a single character on the oled
//  * 
//  * @param x X coordinate for the character
//  * @param y y coordinate for the character
//  * @param c The character to draw
//  * @param size Size of the character to draw (use 1 as default)
//  */
// void SSD1306_DrawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t size);
// /**
//  * @brief Draw a null terminated string of characters to the oled
//  * 
//  * @param x X coordinate to start the string at
//  * @param y Y coordinate to start the string at
//  * @param _text A pointer to the string to draw
//  * @param size The size of each character
//  */
// void SSD1306_DrawText(uint8_t x, uint8_t y, char *_text, uint8_t size);
// /**
//  * @brief Set the text size on the oled
//  * 
//  * @param t_size Size of the text
//  */
// void SSD1306_TextSize(uint8_t t_size);
// /**
//  * @brief Set the next value to write to
//  * 
//  * @param x x coordinate
//  * @param y y coordinate
//  */
// void SSD1306_GotoXY(uint8_t x, uint8_t y);
// /**
//  * @brief print single char
//  * 
//  * @param c Character to print
//  */
// void SSD1306_Print(uint8_t c);
// /**
//  * @brief Set the text to wrap around the screen
//  * 
//  * @param w true to wrap false to not
//  */
// void SSD1306_SetTextWrap(bool w);
// /**
//  * @brief Set the display to be inverted
//  * 
//  * @param i true if inverted false if normal
//  */
// void SSD1306_InvertDisplay(bool i);


// #endif