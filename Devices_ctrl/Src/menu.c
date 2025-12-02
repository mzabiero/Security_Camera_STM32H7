#include "menu.h"
#include "Camera_ctrl.h"
#include "LCD_ctrl.h"
#include "gpio.h"
#include "board.h"
#include <stdio.h>

// Globalne bufory obrazu (zadeklarowane w main.c lub tutaj)
// Używamy extern, zakładając, że są w main.c żeby linker script ich nie zgubił
extern uint16_t pic[160][120];
extern volatile uint32_t DCMI_FrameIsReady;

// Stan aplikacji
static AppState_t CurrentState = STATE_INIT;

// Zmienna do odbioru znaku UART
uint8_t rx_byte;

// extern UART_HandleTypeDef huart3;

void Menu_InitWrapper(void) {
    // 1. Inicjalizacja LCD
    if (LCDCtrl_Init() != LCD_OK) {
        CurrentState = STATE_ERROR;
        return;
    }

    // 2. Splash Screen
    LCDCtrl_ShowSplashScreen();

    // 3. Inicjalizacja Kamery
    if (CameraCtrl_Init() != CAM_OK) {
        LCD_ShowString(0, 0, 160, 16, 12, "Cam Init Error!");
        CurrentState = STATE_ERROR;
        return;
    }

    // 4. Przejście do Menu
    LCDCtrl_ShowMenu();
    CurrentState = STATE_MENU;

    // Rozpocznij nasłuch UART (Interrupt mode dla pojedynczego znaku)
    // HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}

void Process_UART_Command(uint8_t cmd) {
    char msg[30];

    switch (cmd) {
        case '1': // Zwiększ jasność
            if (AppCamSettings.brightness < 100) AppCamSettings.brightness += 10;
            CameraCtrl_SetBrightness(AppCamSettings.brightness);
            if(CurrentState == STATE_MENU) LCDCtrl_UpdateParamValue(1, "1.Jasnosc:", AppCamSettings.brightness);
            break;

        case '2': // Zmniejsz jasność
            if (AppCamSettings.brightness > 0) AppCamSettings.brightness -= 10;
            CameraCtrl_SetBrightness(AppCamSettings.brightness);
            if(CurrentState == STATE_MENU) LCDCtrl_UpdateParamValue(1, "1.Jasnosc:", AppCamSettings.brightness);
            break;

        case '3': // Toggle Night Mode
            AppCamSettings.night_mode = !AppCamSettings.night_mode;
            CameraCtrl_SetNightMode(AppCamSettings.night_mode);
            if(CurrentState == STATE_MENU) LCDCtrl_UpdateParamValue(3, "3.Nocny:", AppCamSettings.night_mode);
            break;

        case 's': // Start/Stop Stream
        case 'S':
            if (CurrentState == STATE_MENU) {
                CurrentState = STATE_STREAMING;
                CameraCtrl_StartStream((uint32_t*)pic, 160*120*2/4); // Start DMA
            } else if (CurrentState == STATE_STREAMING) {
                CurrentState = STATE_MENU;
                CameraCtrl_StopStream();
                LCDCtrl_ShowMenu();
            }
            break;
    }

    // Echo na UART dla potwierdzenia
    // snprintf(msg, 30, "CMD: %c OK\r\n", cmd);
    // HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}

void Menu_Loop(void) {

    // Obsługa przycisku fizycznego na płytce (jako alternatywa dla UART 's')
    static uint32_t last_btn_tick = 0;
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_SET) { // Zakładam, że wciśnięty = SET (sprawdź w board.h)
        if (HAL_GetTick() - last_btn_tick > 500) {
            Process_UART_Command('s'); // Symulujemy komendę 's'
            last_btn_tick = HAL_GetTick();
        }
    }

    // Maszyna stanów
    switch (CurrentState) {
        case STATE_MENU:
            // W menu nic nie robimy w pętli, czekamy na przerwania UART
            break;

        case STATE_STREAMING:
            if (DCMI_FrameIsReady) {
                DCMI_FrameIsReady = 0;

                // 1. Tutaj w przyszłości wstawisz analizę obrazu (Motion Detect)

                // 2. Wyświetlenie klatki
                LCDCtrl_DrawFrame((uint16_t*)pic);
            }
            break;

        case STATE_ERROR:
            // Mruganie diodą błędu
            HAL_Delay(100);
            break;

        default:
            break;
    }
}

// Callback UART - wywoływany, gdy przyjdzie znak
/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) { // Sprawdź czy to właściwy UART
        Process_UART_Command(rx_byte);
        // Nasłuchuj kolejnego znaku
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}
*/



