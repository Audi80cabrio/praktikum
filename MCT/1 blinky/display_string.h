#ifndef DISPLAY_WRITESTRING
#define DISPLAY_WRITESTRING
#include <inttypes.h>

void LCD_WriteLetter( uint16_t x, uint16_t y, uint16_t colForeground, uint16_t colBackground, char letter );
void LCD_WriteString( uint16_t x, uint16_t y, uint16_t colForeground, uint16_t colBackground, char * text );


#endif
