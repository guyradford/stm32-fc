//
// Created by Codex on 15/06/2026.
//

#include "imu_calibration_store.h"

#include <string.h>
#include "main.h"

#define IMU_CALIBRATION_STORE_ADDRESS FLASH_EEPROM_BASE
#define IMU_CALIBRATION_STORE_MAGIC 0x494D5543UL
#define IMU_CALIBRATION_STORE_VERSION 1U

typedef struct imu_calibration_record_tag {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint8_t data[sizeof(bno055_calibration_data_t)];
    uint32_t checksum;
} IMU_CALIBRATION_RECORD;

static uint32_t IMUCalibrationStore_UpdateChecksum(uint32_t checksum, const uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
        checksum *= 16777619UL;
    }
    return checksum;
}

static uint32_t IMUCalibrationStore_GetChecksum(uint16_t version, uint16_t length, const uint8_t *data) {
    uint32_t checksum = 2166136261UL;
    uint8_t header[4] = {
            (uint8_t) (version & 0xFFU),
            (uint8_t) ((version >> 8U) & 0xFFU),
            (uint8_t) (length & 0xFFU),
            (uint8_t) ((length >> 8U) & 0xFFU)
    };

    checksum = IMUCalibrationStore_UpdateChecksum(checksum, header, sizeof(header));
    checksum = IMUCalibrationStore_UpdateChecksum(checksum, data, length);
    return checksum;
}

static const IMU_CALIBRATION_RECORD *IMUCalibrationStore_GetRecord(void) {
    return (const IMU_CALIBRATION_RECORD *) IMU_CALIBRATION_STORE_ADDRESS;
}

static bool IMUCalibrationStore_WriteByte(uint32_t address, uint8_t value) {
    return HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address, value) == HAL_OK;
}

bool IMUCalibrationStore_Load(bno055_calibration_data_t *calibration) {
    const IMU_CALIBRATION_RECORD *record = IMUCalibrationStore_GetRecord();

    if (calibration == NULL) {
        return false;
    }

    if (record->magic != IMU_CALIBRATION_STORE_MAGIC ||
        record->version != IMU_CALIBRATION_STORE_VERSION ||
        record->length != sizeof(bno055_calibration_data_t)) {
        return false;
    }

    uint32_t checksum = IMUCalibrationStore_GetChecksum(record->version, record->length, record->data);
    if (checksum != record->checksum) {
        return false;
    }

    memcpy(calibration, record->data, sizeof(*calibration));
    return true;
}

bool IMUCalibrationStore_Save(const bno055_calibration_data_t *calibration) {
    if (calibration == NULL) {
        return false;
    }

    IMU_CALIBRATION_RECORD record = {
            .magic = IMU_CALIBRATION_STORE_MAGIC,
            .version = IMU_CALIBRATION_STORE_VERSION,
            .length = sizeof(bno055_calibration_data_t),
            .checksum = 0
    };

    memcpy(record.data, calibration, sizeof(*calibration));
    record.checksum = IMUCalibrationStore_GetChecksum(record.version, record.length, record.data);

    if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) {
        return false;
    }

    bool ok = true;
    const uint8_t *bytes = (const uint8_t *) &record;
    for (uint16_t i = 0; i < sizeof(record); i++) {
        if (!IMUCalibrationStore_WriteByte(IMU_CALIBRATION_STORE_ADDRESS + i, bytes[i])) {
            ok = false;
            break;
        }
    }

    HAL_FLASHEx_DATAEEPROM_Lock();
    return ok;
}

bool IMUCalibrationStore_Clear(void) {
    if (HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) {
        return false;
    }

    bool ok = HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_WORD, IMU_CALIBRATION_STORE_ADDRESS) == HAL_OK;
    HAL_FLASHEx_DATAEEPROM_Lock();
    return ok;
}
