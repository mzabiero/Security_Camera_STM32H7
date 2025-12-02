#include "camera.h"
#include "tim.h"
// Zakładam, że te pliki istnieją w Twoim projekcie:
#include "ov7670.h"
#include "ov2640.h"
#include "ov7725.h"
#include "ov5640.h"

Camera_HandleTypeDef hcamera;

// Resolution table (bez zmian)
const uint16_t dvp_cam_resolution[][2] = {
	{0, 0},
	{88, 72}, {176, 144}, {352, 288}, {88, 60}, {176, 120}, {352, 240},
	{40, 30}, {80, 60}, {160, 120}, {320, 240}, {640, 480}, {60, 40},
	{120, 80}, {240, 160}, {480, 320}, {64, 32}, {64, 64}, {128, 64},
	{128, 128}, {128, 160}, {128, 160}, {720, 480}, {752, 480}, {800, 600},
	{1024, 768}, {1280, 1024}, {1600, 1200}, {1280, 720}, {1920, 1080},
	{1280, 960}, {2592, 1944},
};

// --- Funkcje I2C (bez zmian) ---

int32_t Camera_WriteReg(Camera_HandleTypeDef *hov, uint8_t regAddr, const uint8_t *pData)
{
	uint8_t tt[2];
	tt[0] = regAddr;
	tt[1] = pData[0];
	if (HAL_I2C_Master_Transmit(hov->hi2c, hov->addr, tt, 2, hov->timeout) == HAL_OK) return Camera_OK;
	else return camera_ERROR;
}

int32_t Camera_ReadReg(Camera_HandleTypeDef *hov, uint8_t regAddr, uint8_t *pData)
{
	HAL_I2C_Master_Transmit(hov->hi2c, hov->addr + 1, &regAddr, 1, hov->timeout);
	if (HAL_I2C_Master_Receive(hov->hi2c, hov->addr + 1, pData, 1, hov->timeout) == HAL_OK) return Camera_OK;
	else return camera_ERROR;
}

int32_t Camera_WriteRegb2(Camera_HandleTypeDef *hov, uint16_t reg_addr, uint8_t reg_data)
{
	if (HAL_I2C_Mem_Write(hov->hi2c, hov->addr + 1, reg_addr, I2C_MEMADD_SIZE_16BIT, &reg_data, 1, hov->timeout) == HAL_OK) return Camera_OK;
	else return camera_ERROR;
}

int32_t Camera_ReadRegb2(Camera_HandleTypeDef *hov, uint16_t reg_addr, uint8_t *reg_data)
{
	if (HAL_I2C_Mem_Read(hov->hi2c, hov->addr + 1, reg_addr, I2C_MEMADD_SIZE_16BIT, reg_data, 1, hov->timeout) == HAL_OK) return Camera_OK;
	else return camera_ERROR;
}

int32_t Camera_WriteRegList(Camera_HandleTypeDef *hov, const struct regval_t *reg_list)
{
	const struct regval_t *pReg = reg_list;
	while (pReg->reg_addr != 0xFF && pReg->value != 0xFF)
	{
		if (Camera_WriteReg(hov, pReg->reg_addr, &(pReg->value)) != Camera_OK) return camera_ERROR;
		pReg++;
	}
	return Camera_OK;
}

int32_t Camera_read_id(Camera_HandleTypeDef *hov)
{
	uint8_t temp[2];
	temp[0] = 0x01;
	// Logika identyfikacji bez zmian, ale zwraca status
	if (hov->addr != OV5640_ADDRESS)
	{
		Camera_WriteReg(hov, 0xFF, temp);
		Camera_ReadReg(hov, 0x1C, &temp[0]);
		Camera_ReadReg(hov, 0x1D, &temp[1]);
		hov->manuf_id = ((uint16_t)temp[0] << 8) | temp[1];
		Camera_ReadReg(hov, 0x0A, &temp[0]);
		Camera_ReadReg(hov, 0x0B, &temp[1]);
	}
	else
	{
		Camera_ReadRegb2(&hcamera, 0x300A, &temp[0]);
		Camera_ReadRegb2(&hcamera, 0x300B, &temp[1]);
		hov->manuf_id = 0;
	}
	hov->device_id = ((uint16_t)temp[0] << 8) | temp[1];

	// Sprawdzenie czy cokolwiek odczytano
	if (hov->device_id == 0 || hov->device_id == 0xFFFF) return camera_ERROR;
	return Camera_OK;
}

void Camera_Reset(Camera_HandleTypeDef *hov)
{
	uint8_t temp = 0x01;
	Camera_WriteReg(hov, 0xFF, &temp);
	temp = 0x80;
	Camera_WriteReg(hov, 0x12, &temp);
	HAL_Delay(100);
}


uint8_t Camera_Init_Device(I2C_HandleTypeDef *hi2c, framesize_t framesize)
{
	hcamera.hi2c = hi2c;
	hcamera.addr = OV7670_ADDRESS;
	hcamera.timeout = 100;

    // Próba odczytu ID
	if (Camera_read_id(&hcamera) != Camera_OK) return camera_ERROR;

    // OV7670
	if (hcamera.manuf_id == 0x7fa2 && hcamera.device_id == 0x7673)
	{
		OV7670_Config();
        return Camera_OK;
	}
	else
	{
        // Sprawdź czy to OV2640 (ID: 0x2641 lub 0x2642)
		hcamera.addr = OV2640_ADDRESS;
		Camera_read_id(&hcamera);
		if (hcamera.manuf_id == 0x7fa2 && ((hcamera.device_id - 0x2641) <= 2))
		{
			ov2640_init(framesize);
            return Camera_OK;
		}
		else
		{
            // OV7725
			hcamera.addr = OV7725_ADDRESS;
			Camera_read_id(&hcamera);
			if (hcamera.manuf_id == 0x7fa2 && ((hcamera.device_id - 0x7721) <= 2))
			{
				ov7725_init(framesize);
                return Camera_OK;
			}
			else
			{
                // OV5640
				hcamera.addr = OV5640_ADDRESS;
				Camera_read_id(&hcamera);
				if (hcamera.device_id == 0x5640)
				{
					ov5640_init(framesize);
                    return Camera_OK;
				}
				else
				{
					hcamera.addr = 0;
					hcamera.device_id = 0;
                    return camera_ERROR; // Nie znaleziono żadnej znanej kamery
				}
			}
		}
	}
    return camera_ERROR;
}

// --- POMOCNICZE FUNKCJE WEWNĘTRZNE ---

// OV2640 wymaga przełączania banków (DSP vs Sensor)
// 0xFF to rejestr wyboru banku. 0x01 = DSP, 0x00 = Sensor.
static void OV2640_SetBank(uint8_t bank) {
    Camera_WriteReg(&hcamera, 0xFF, &bank);
}


// --- UNIWERSALNE FUNKCJE STERUJĄCE ---

// Jasność:
// OV7725: Rejestr BRIGHT (0x55)
// OV2640: Rejestr 0x55 w Banku 0 lub specjalne ustawienia w DSP.
//         Dla uproszczenia w OV2640 używamy podejścia standardowego (offset jasności w Banku 1)
uint8_t Camera_SetBrightness(uint8_t level) {
    uint8_t val;

    // Mapowanie 0-100 na zakres rejestru (zazwyczaj 0-255 lub ze znakiem)
    val = (level * 255) / 100;

    if (hcamera.device_id == 0x7721) { // OV7725
        return Camera_WriteReg(&hcamera, 0x55, &val);
    }
    else if ((hcamera.device_id == 0x2641) || (hcamera.device_id == 0x2642)) { // OV2640
        OV2640_SetBank(1); // Bank DSP
        // W OV2640 jasność często kontroluje się kilkoma rejestrami,
        // ale 0x55 w Banku 1 to często "Brightness Offset".
        // Niektóre drivery używają tu mapowania:
        // 0=0x40, 1=0x30, 2=0x20, 3=0x10, 4=0x00 (default), 5=0x10... z bitami znaku.
        // Poniżej wersja uproszczona:
        return Camera_WriteReg(&hcamera, 0x55, &val);
    }

    return Camera_OK;
}

// Kontrast:
// OV7725: Rejestr CNST (0x56)
// OV2640: Rejestr 0x56 w Banku 1
uint8_t Camera_SetContrast(uint8_t level) {
    uint8_t val = (level * 255) / 100;

    if (hcamera.device_id == 0x7721) { // OV7725
        return Camera_WriteReg(&hcamera, 0x56, &val);
    }
    else if ((hcamera.device_id == 0x2641) || (hcamera.device_id == 0x2642)) { // OV2640
        OV2640_SetBank(1); // Bank DSP
        return Camera_WriteReg(&hcamera, 0x56, &val);
    }
    return Camera_OK;
}

// Tryb Nocny (Gain Ceiling / Exposure)
uint8_t Camera_SetGainCeiling(uint8_t gain) {
    // gain: 0 = Normal/Auto, 1 (lub >0) = High Gain / Night Mode

    if (hcamera.device_id == 0x7721) { // OV7725
        uint8_t reg_val = (gain > 0) ? 0xFF : 0x00; // Max gain vs Auto
        return Camera_WriteReg(&hcamera, 0x00, &reg_val);
    }
    else if ((hcamera.device_id == 0x2641) || (hcamera.device_id == 0x2642)) { // OV2640
        // OV2640 Gain jest w Banku 0 (Sensor)
        // Rejestr 0x00 to Gain Control
        OV2640_SetBank(0); // Bank Sensor

        // Jeśli tryb nocny (gain > 0), wymuszamy wysokie wzmocnienie
        // W przeciwnym razie zdajemy się na AGC (Auto Gain)
        // Uwaga: To prosta implementacja. Pełna wymagałaby edycji rejestrów AGC/AEC.
        if (gain > 0) {
             // Włączamy podbicie
             uint8_t val = 0xFF; // Max gain?
             Camera_WriteReg(&hcamera, 0x00, &val);
        } else {
             // Przywracamy Auto (to wymagałoby resetu kamery lub wiedzy o poprzednim stanie AGC)
             // W tym prostym przypadku, po prostu nic nie robimy lub wpisujemy default 0x00
             uint8_t val = 0x00;
             Camera_WriteReg(&hcamera, 0x00, &val);
        }
    }
    return Camera_OK;
}

// Efekt:
// 0: Normal
// 1: B&W
// 2: Sepia (opcjonalnie)
uint8_t Camera_SetEffect(uint8_t effect) {
    if (hcamera.device_id == 0x7721) { // OV7725
        uint8_t val = 0;
        switch(effect) {
            case 0: val = 0x00; break; // Normal
            case 1: val = 0x20; break; // B&W (Bit 5 w rej. 0xA6) - sprawdzic dokumentacje
            case 2: val = 0x40; break; // Sepia? (zależne od sensora)
        }
        // Zakładam SDE register 0xA6 dla OV7725
        return Camera_WriteReg(&hcamera, 0xA6, &val);
    }
    else if ((hcamera.device_id == 0x2641) || (hcamera.device_id == 0x2642)) { // OV2640
        OV2640_SetBank(1); // Bank DSP

        // OV2640 Effects Control: 0x7C (Enable SDE), 0x7D (Mode)
        // Sekwencje zaczerpnięte z typowych driverów
        uint8_t r7c = 0x00;
        uint8_t r7d = 0x00;

        switch(effect) {
            case 0: // Normal
                r7c = 0x00; r7d = 0x00;
                break;
            case 1: // B&W
                r7c = 0x20; r7d = 0x18;
                break;
            case 2: // Sepia
                r7c = 0x20; r7d = 0x40;
                break;
            default:
                return Camera_OK;
        }

        Camera_WriteReg(&hcamera, 0x7C, &r7c);
        Camera_WriteReg(&hcamera, 0x7D, &r7d);
    }
    return Camera_OK;
}
