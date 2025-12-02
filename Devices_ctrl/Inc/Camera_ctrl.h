#pragma once

#include "main.h"



// Struktura błędów kamery
typedef enum {
	CAM_OK			= 0,
	CAM_ERR			= 1,
	CAM_INIT_ERR	= 2,
	CAM_I2C_ERR		= 3
} Cam_Status_t;

// Struktura ustawień kamery
typedef struct {
    uint8_t brightness; // 0-100
    uint8_t contrast;   // 0-100
    uint8_t night_mode; // 0: Off, 1: On (High Gain)
    uint8_t motion_detect_enabled;
} Camera_Settings_t;

extern Camera_Settings_t AppCamSettings;

// Funkcje do obsługi kamery
Cam_Status_t CameraCtrl_Init(void);
Cam_Status_t CameraCtrl_StartStream(uint32_t *buffer_addr, uint32_t buffer_size);
Cam_Status_t CameraCtrl_StopStream(void);

// Funkcje pomocnicze/ wrappery dla biblioteki
Cam_Status_t CameraCtrl_SetBrightness(uint8_t val);
Cam_Status_t CameraCtrl_SetContrast(uint8_t val);
Cam_Status_t CameraCtrl_SetNightMode(uint8_t enable);

