#include "LCD_ctrl.h"
#include "Camera_ctrl.h"
#include <stdio.h>

// Bufor na teksty - static, żeby nie obciążać stosu!
static char lcd_text_buf[32];

LCD_Status_t LCDCtrl_Init(void) {
    // Kod inicjalizacji skopiowany i uporządkowany z Twojego LCD_Test

    #ifdef TFT96
    ST7735Ctx.Orientation = ST7735_ORIENTATION_LANDSCAPE_ROT180;
    ST7735Ctx.Panel = HannStar_Panel;
    ST7735Ctx.Type = ST7735_0_9_inch_screen;
    #elif TFT18
    ST7735Ctx.Orientation = ST7735_ORIENTATION_PORTRAIT;
    ST7735Ctx.Panel = BOE_Panel;
    ST7735Ctx.Type = ST7735_1_8a_inch_screen;
    #endif

    ST7735_RegisterBusIO(&st7735_pObj, &st7735_pIO);
    ST7735_LCD_Driver.Init(&st7735_pObj, ST7735_FORMAT_RBG565, &ST7735Ctx);

    LCD_Light(50, 10); // 50% jasności na start

    return LCD_OK;
}

LCD_Status_t LCDCtrl_ShowSplashScreen(void) {
    ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

    LCD_ShowString(10, 20, ST7735Ctx.Width, 16, 16, "Security");
    LCD_ShowString(10, 40, ST7735Ctx.Width, 16, 16, "Camera App");
    LCD_ShowString(10, 70, ST7735Ctx.Width, 16, 12, "PMIK Project");

    HAL_Delay(1000); // Krótkie opóźnienie dla efektu
    return LCD_OK;
}

LCD_Status_t LCDCtrl_ShowMenu(void) {
    // Czyści ekran i rysuje statyczne menu
    ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

    LCD_ShowString(0, 0, ST7735Ctx.Width, 16, 12, "--- MENU ---");

    // Wyświetl aktualne wartości
    LCDCtrl_UpdateParamValue(1, "1.Jasnosc:", AppCamSettings.brightness);
    LCDCtrl_UpdateParamValue(2, "2.Kontrast:", AppCamSettings.contrast);
    LCDCtrl_UpdateParamValue(3, "3.Nocny:", AppCamSettings.night_mode);

    LCD_ShowString(0, 60, ST7735Ctx.Width, 16, 12, "s - Start Cam");
    LCD_ShowString(0, 75, ST7735Ctx.Width, 16, 12, "UART Control");

    return LCD_OK;
}

LCD_Status_t LCDCtrl_UpdateParamValue(uint8_t line, char* label, int value) {
    // Formatowanie stringa bezpiecznie (snprintf)
    snprintf(lcd_text_buf, sizeof(lcd_text_buf), "%s %d  ", label, value);
    // Y = 15 * line (proste pozycjonowanie)
    LCD_ShowString(0, 15 * line, ST7735Ctx.Width, 16, 12, (uint8_t*)lcd_text_buf);
    return LCD_OK;
}

LCD_Status_t LCDCtrl_DrawFrame(uint16_t *image_buffer) {
    // Rysowanie klatki z kamery
    // Zakładamy format pasujący do ekranu
    #ifdef TFT96
    // Offsety, żeby wycentrować 80px wysokości na ekranie 80px (dla 160x80)
    ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)image_buffer, 160, 80);
    #elif TFT18
    ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)image_buffer, 128, 160);
    #endif
    return LCD_OK;
}
