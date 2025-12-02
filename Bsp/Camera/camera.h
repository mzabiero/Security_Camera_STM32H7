#ifndef CAMERA_H
#define CAMERA_H

#include "main.h"

// --- Definicje podstawowe ---
#define XCLK_TIM       0
#define XCLK_MCO       1

// Adresy I2C kamer (7-bit address przesunięty w lewo, biblioteka HAL używa 8-bit)
#define OV7670_ADDRESS 0x42
#define OV2640_ADDRESS 0x60
#define OV7725_ADDRESS 0x42
#define OV5640_ADDRESS 0x78

// Kody błędów
#define Camera_OK      0
#define camera_ERROR   1

// Makro opóźnienia
#define Camera_delay HAL_Delay

// --- Struktury danych ---

// Struktura do list rejestrów (adres, wartość)
struct regval_t {
	uint8_t reg_addr;
	uint8_t value;
};

// Formaty pikseli
typedef enum {
	PIXFORMAT_INVALID = 0,
	PIXFORMAT_RGB565,    // 2BPP/RGB565
	PIXFORMAT_JPEG,      // JPEG/COMPRESSED
	PIXFORMAT_YUV422,
	PIXFORMAT_GRAYSCALE,
	PIXFORMAT_BAYER,
} pixformat_t;

// Rozmiary klatek
typedef enum {
    FRAMESIZE_INVALID = 0,
    FRAMESIZE_QQCIF,    // 88x72
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_CIF,      // 352x288
    FRAMESIZE_QQSIF,    // 88x60
    FRAMESIZE_QSIF,     // 176x120
    FRAMESIZE_SIF,      // 352x240
    FRAMESIZE_QQQQVGA,  // 40x30
    FRAMESIZE_QQQVGA,   // 80x60
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_HQQQVGA,  // 60x40
    FRAMESIZE_HQQVGA,   // 120x80
    FRAMESIZE_HQVGA,    // 240x160
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_64X32,    // 64x32
    FRAMESIZE_64X64,    // 64x64
    FRAMESIZE_128X64,   // 128x64
    FRAMESIZE_128X128,  // 128x128
    FRAMESIZE_LCD,      // 128x160
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_WVGA,     // 720x480
    FRAMESIZE_WVGA2,    // 752x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    FRAMESIZE_720P,     // 1280x720
    FRAMESIZE_1080P,    // 1920x1080
    FRAMESIZE_960P,     // 1280x960
    FRAMESIZE_5MPP,     // 2592x1944
} framesize_t;

// Uchwyt kamery
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t addr;
	uint32_t timeout;
	uint16_t manuf_id;
	uint16_t device_id;
	framesize_t framesize;
	pixformat_t pixformat;
} Camera_HandleTypeDef;

// --- Zmienne globalne ---
extern Camera_HandleTypeDef hcamera;
extern const uint16_t dvp_cam_resolution[][2];

// --- Funkcje Niskopoziomowe (Low-Level) ---
int32_t Camera_WriteReg(Camera_HandleTypeDef *hov, uint8_t regAddr, const uint8_t *pData);
int32_t Camera_ReadReg(Camera_HandleTypeDef *hov, uint8_t regAddr, uint8_t *pData);
int32_t Camera_WriteRegb2(Camera_HandleTypeDef *hov, uint16_t reg_addr, uint8_t reg_data);
int32_t Camera_ReadRegb2(Camera_HandleTypeDef *hov, uint16_t reg_addr, uint8_t *reg_data);
int32_t Camera_WriteRegList(Camera_HandleTypeDef *hov, const struct regval_t *reg_list);
int32_t Camera_read_id(Camera_HandleTypeDef *hov);
void Camera_Reset(Camera_HandleTypeDef *hov);
void Camera_XCLK_Set(uint8_t xclktype);

// --- Funkcja Inicjalizująca (zmodyfikowana) ---
// Zwraca status błędu (0 = OK, 1 = Error)
uint8_t Camera_Init_Device(I2C_HandleTypeDef *hi2c, framesize_t framesize);

// --- Funkcje Sterujące (High-Level) ---
// Obsługują zarówno OV7725 jak i OV2640 (przez detekcję device_id)
uint8_t Camera_SetBrightness(uint8_t level); // 0-100
uint8_t Camera_SetContrast(uint8_t level);   // 0-100
uint8_t Camera_SetGainCeiling(uint8_t gain); // 0 = Auto/Day, >0 = Night Mode
uint8_t Camera_SetEffect(uint8_t effect);    // 0: Normal, 1: B&W, 2: Sepia

#endif /* CAMERA_H */
