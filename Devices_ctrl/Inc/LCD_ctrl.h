#ifndef INC_LCD_CTRL_H_
#define INC_LCD_CTRL_H_

#include "main.h"
#include "lcd.h"     // WeAct LCD
#include "st7735.h"  // Sterownik


extern ST7735_IO_t st7735_pIO;

typedef enum {
    LCD_OK = 0,
    LCD_ERR = 1
} LCD_Status_t;

// Funkcje
LCD_Status_t LCDCtrl_Init(void);
LCD_Status_t LCDCtrl_ShowSplashScreen(void);
LCD_Status_t LCDCtrl_ShowMenu(void);
LCD_Status_t LCDCtrl_UpdateParamValue(uint8_t line, char* label, int value);
LCD_Status_t LCDCtrl_DrawFrame(uint16_t *image_buffer);

#endif /* INC_LCD_CTRL_H_ */
