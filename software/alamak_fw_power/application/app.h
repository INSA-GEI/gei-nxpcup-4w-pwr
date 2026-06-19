/*
 * app.h
 *
 *  Created on: 17 juin 2026
 *      Author: dimercur
 */

#ifndef APP_H_
#define APP_H_

typedef enum {
    APP_STATE_IMX_BOOT = 0,
    APP_STATE_RUNNING = 1,
    APP_STATE_IMX_SHUTDOWN = 2,
    APP_STATE_ERROR = 3,
    APP_STATE_LOW_BATTERY = 4,
    APP_STATE_CRITICAL_LOW_BATTERY = 5,
    APP_STATE_MAX
} APP_States_t;

#define APP_VBAT_CRITICAL_THRESHOLD 9100 // mV
#define APP_VBAT_LOW_THRESHOLD 9500 // mV

typedef struct {
    APP_States_t state;
    bool evtI2CDataChanged;
    bool evtTimer1ms;
    bool evtTimer10ms;
    bool evtTimer100ms;
    bool evtTimer1s;
    bool evtShutdown;
    uint32_t odometer_1;
    uint32_t odometer_2;
    uint8_t pwmMot1; // valeur entre 0 et 100 pour le moteur 1
    uint8_t pwmMot2; // valeur entre 0 et 100 pour le moteur 2   
    int8_t pwmServo; // valeur entre -100 et 100 pour le servo
    bool shutdownRequest; // false = pas de demande, true = demande de shutdown
    bool motorcontrolEnabled; // false = asservissement moteurs désactivés, true = asservissement moteurs activés
} APP_SystemData_t;

/* Control Register (CR) bits */
#define APP_CR_MOTOR_CONTROL_ENABLE 0x01
#define APP_CR_SHUTDOWN_REQUEST 0x02
#define APP_CR_RESET_ODOMETER 0x04
#define APP_CR_CAMERA_LED 0x08

extern volatile APP_SystemData_t APP_SystemData;

void APP_Init(void);

void APP_Run(void);

#endif /* APP_H_ */
