/*
 * app_functions.h
 *
 *  Created on: 19 juin 2026
 *      Author: dimercur
 */

#ifndef APP_FUNCTIONS_H_
#define APP_FUNCTIONS_H_

typedef enum {
    APP_ADC_CHANNEL_VBAT = 0,
    APP_ADC_CHANNEL_MAX
} APP_ADCChannels_t;

typedef enum {
    APP_ODOMETER_1 = 1,
    APP_ODOMETER_2 = 2,
    APP_ODOMETER_MAX
} APP_OdometerIndex_t;

typedef enum {
    APP_MOTOR_1 = 1,
    APP_MOTOR_2 = 2,
    APP_MOTOR_MAX
} APP_MotorIndex_t;

uint16_t APP_GetVBat(void);

void APP_SetVBat(uint16_t vbat);

uint16_t APP_ConvertADCToVoltage(uint16_t adcValue);

uint16_t APP_GetRawADCValue(APP_ADCChannels_t channel);

void APP_SetOdometerToI2C(APP_OdometerIndex_t odometerIndex, uint32_t value);

uint32_t APP_GetRawOdometerValue(APP_OdometerIndex_t odometerIndex);

void APP_ResetRawOdometer(APP_OdometerIndex_t odometerIndex);

uint8_t APP_GetMotorFromI2C(APP_MotorIndex_t motorIndex);

int8_t APP_GetDirectionFromI2C(void);

void APP_SetMotorsAndDirection(uint8_t dutyLeft, uint8_t dutyRight, int8_t direction);

void APP_SetMotorPWM(APP_MotorIndex_t motorIndex, uint8_t pwmValue);

void APP_SetDirectionPWM(int8_t direction);

#endif /* APP_FUNCTIONS_H_ */
