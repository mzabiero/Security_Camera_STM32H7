#include "Camera_ctrl.h"
#include "camera.h"
#include "dcmi.h"
#include "i2c.h"
#include <stdio.h>

Camera_Settings_t AppCamSettings = {
    .brightness = 50,
    .contrast = 50,
    .night_mode = 0,
    .motion_detect_enabled = 0
};

extern I2C_HandleTypeDef hi2c1;
extern DCMI_HandleTypeDef hdcmi;

Cam_Status_t CameraCtrl_Init(void) {
    // Używamy zmodyfikowanej funkcji zwracającej status
    uint8_t status = Camera_Init_Device(&hi2c1, FRAMESIZE_QQVGA);

    if (status != Camera_OK) {
        return CAM_INIT_ERR; // Teraz mamy pewność, że to błąd
    }

    // Ustawienie domyślnych parametrów przy użyciu nowych funkcji w camera.c
    CameraCtrl_SetBrightness(AppCamSettings.brightness);
    CameraCtrl_SetContrast(AppCamSettings.contrast);

    return CAM_OK;
}

Cam_Status_t CameraCtrl_StartStream(uint32_t *buffer_addr, uint32_t buffer_size) {
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)buffer_addr, buffer_size) != HAL_OK) {
        return CAM_ERR;
    }
    return CAM_OK;
}

Cam_Status_t CameraCtrl_StopStream(void) {
    if (HAL_DCMI_Stop(&hdcmi) != HAL_OK) {
        return CAM_ERR;
    }
    return CAM_OK;
}

// Wrappery mapują 0-100 na logikę sensora
Cam_Status_t CameraCtrl_SetBrightness(uint8_t val) {
    AppCamSettings.brightness = val;
    // Wywołanie funkcji z camera.c
    if (Camera_SetBrightness(val) != Camera_OK) return CAM_I2C_ERR;
    return CAM_OK;
}

Cam_Status_t CameraCtrl_SetContrast(uint8_t val) {
    AppCamSettings.contrast = val;
    if (Camera_SetContrast(val) != Camera_OK) return CAM_I2C_ERR;
    return CAM_OK;
}

Cam_Status_t CameraCtrl_SetNightMode(uint8_t enable) {
    AppCamSettings.night_mode = enable;
    if (enable) {
        // Maksymalne wzmocnienie dla nocy
        Camera_SetGainCeiling(0xFF);
        // Można też zmienić exposure, ale to wymagałoby edycji rejestrów AEC
    } else {
        // Domyślne auto-gain
        Camera_SetGainCeiling(0x00);
    }
    return CAM_OK;
}
