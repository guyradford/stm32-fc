#include "serial_link.h"

#include "hmi.h"
#include "telemetry.h"
#include "uart_interface.h"

static uint8_t serial_link_mode = SERIAL_LINK_MODE_HMI;

void SerialLink_Init(bool setupMode) {
    serial_link_mode = SERIAL_LINK_MODE_HMI;
    HMI_Init(setupMode);
    Telemetry_Init();
}

void SerialLink_EnterTelemetryMode(uint32_t now) {
    serial_link_mode = SERIAL_LINK_MODE_TELEMETRY;
    Telemetry_Start(now);
}

void SerialLink_EnterHmiMode(void) {
    Telemetry_Stop();
    serial_link_mode = SERIAL_LINK_MODE_HMI;
    HMI_ShowMenu();
}

uint8_t SerialLink_GetMode(void) {
    return serial_link_mode;
}

void SerialLink_OnTick(uint32_t now) {
    uint8_t character;

    while ((character = UARTInterface_GetNextFromRecvBuffer()) != 0) {
        if (serial_link_mode == SERIAL_LINK_MODE_TELEMETRY) {
            if (Telemetry_OnInput(character, now) == TELEMETRY_INPUT_EXIT) {
                SerialLink_EnterHmiMode();
            }
        } else {
            HMI_OnInput(character);
        }
    }

    if (serial_link_mode == SERIAL_LINK_MODE_TELEMETRY) {
        Telemetry_OnTick(now);
    } else {
        HMI_OnTick(now);
        if (HMI_ConsumeTelemetryRequest()) {
            SerialLink_EnterTelemetryMode(now);
        }
    }
}
