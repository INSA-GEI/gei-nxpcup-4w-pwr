/*
 * app_functions.c
 *
 *  Created on: 19 juin 2026
 *      Author: dimercur
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#include "app.h"
#include "app_functions.h"
#include "i2c_slave_gemini.h"

typedef struct
{
    uint16_t adc;
    uint16_t voltage;
} ADCVoltagePoint_t;

static const ADCVoltagePoint_t conversionTable[] =
{
    {    0,    0 },
    {18000,  330 },
    {35000,  500 },
    {50000,  700 },
    {65535, 1200 }
};

#define NB_POINTS (sizeof(conversionTable) / sizeof(conversionTable[0]))

uint16_t APP_GetVBat(void) {
    uint16_t vbat = 0;
    vbat = (uint16_t)I2C_Slave_GetReg(REG_VBAT_1R);
    vbat |= ((uint16_t)I2C_Slave_GetReg(REG_VBAT_2R) << 8);

    return vbat;
}

void APP_SetVBat(uint16_t vbat) {
    I2C_Slave_SetReg(REG_VBAT_1R, (uint8_t)(vbat & 0xFF));
    I2C_Slave_SetReg(REG_VBAT_2R, (uint8_t)((vbat >> 8) & 0xFF));
}

uint16_t APP_ConvertADCToVoltage(uint16_t adcValue)
{
    uint32_t i;

    /* Saturation basse */
    if (adcValue <= conversionTable[0].adc)
    {
        return conversionTable[0].voltage;
    }

    /* Recherche du segment */
    for (i = 1; i < NB_POINTS; i++)
    {
        if (adcValue <= conversionTable[i].adc)
        {
            uint32_t adc1 = conversionTable[i - 1].adc;
            uint32_t adc2 = conversionTable[i].adc;
            uint32_t v1   = conversionTable[i - 1].voltage;
            uint32_t v2   = conversionTable[i].voltage;

            /* Interpolation linéaire */
            return (uint16_t)(v1 +
                ((uint32_t)(adcValue - adc1) * (v2 - v1)) / (adc2 - adc1));
        }
    }

    return conversionTable[NB_POINTS - 1].voltage;
}

uint16_t APP_GetRawADCValue(APP_ADCChannels_t channel) {
    lpadc_conv_result_t adc_result;
    if (channel >= APP_ADC_CHANNEL_MAX) {
        return 0; // Invalid channel
    }

    if (channel == APP_ADC_CHANNEL_VBAT) {
        LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);
        LPADC_GetConvResult(ADC0_PERIPHERAL, &adc_result);
    }
    
    return adc_result.convValue;
}

void APP_SetOdometerToI2C(APP_OdometerIndex_t odometerIndex, uint32_t value) {
    if (odometerIndex == APP_ODOMETER_1) {
        I2C_Slave_SetReg(REG_ODO1_1R, (uint8_t)(value & 0xFF));
        I2C_Slave_SetReg(REG_ODO1_2R, (uint8_t)((value >> 8) & 0xFF));
        I2C_Slave_SetReg(REG_ODO1_3R, (uint8_t)((value >> 16) & 0xFF));
        I2C_Slave_SetReg(REG_ODO1_4R, (uint8_t)((value >> 24) & 0xFF));
    } else if (odometerIndex == APP_ODOMETER_2) {
        I2C_Slave_SetReg(REG_ODO2_1R, (uint8_t)(value & 0xFF));
        I2C_Slave_SetReg(REG_ODO2_2R, (uint8_t)((value >> 8) & 0xFF));
        I2C_Slave_SetReg(REG_ODO2_3R, (uint8_t)((value >> 16) & 0xFF));
        I2C_Slave_SetReg(REG_ODO2_4R, (uint8_t)((value >> 24) & 0xFF));
    }
}

uint32_t APP_GetRawOdometerValue(APP_OdometerIndex_t odometerIndex) {
    uint32_t value = 0;

    if (odometerIndex == APP_ODOMETER_1) {
        value=CTIMER_GetTimerCountValue(CTIMER0_PERIPHERAL);
    } else if (odometerIndex == APP_ODOMETER_2) {
        value=CTIMER_GetTimerCountValue(CTIMER0_PERIPHERAL);
    }

    return value;
}

void APP_ResetRawOdometer(APP_OdometerIndex_t odometerIndex) {
    if (odometerIndex == APP_ODOMETER_1) {
    	CTIMER_Reset(CTIMER0_PERIPHERAL);
    } else if (odometerIndex == APP_ODOMETER_2) {
    	CTIMER_Reset(CTIMER1_PERIPHERAL);
    }
}

uint8_t APP_GetMotorFromI2C(APP_MotorIndex_t motorIndex) {
    uint8_t pwmValue = 0;

    if (motorIndex == APP_MOTOR_1) {
        pwmValue = (uint8_t)I2C_Slave_GetReg(REG_MOT1);
    } else if (motorIndex == APP_MOTOR_2) {
        pwmValue = (uint8_t)I2C_Slave_GetReg(REG_MOT2);
    }

    return pwmValue;
}

int8_t APP_GetDirectionFromI2C(void) {
    int8_t direction = 0;

    direction = (int8_t)I2C_Slave_GetReg(REG_DIR);
    return direction;
}

void APP_SetMotorPWM(APP_MotorIndex_t motorIndex, uint8_t pwmValue) {
    uint8_t pwmMot = pwmValue;
    if (pwmMot > 100) {
        pwmMot = 100; // Limit to 100%
    }

    if (motorIndex == APP_MOTOR_1) {
        PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, pwmMot);
    } else if (motorIndex == APP_MOTOR_2) {
        PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, kPWM_Module_0, kPWM_PwmB, kPWM_SignedCenterAligned, pwmMot);
    }

    /* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, kPWM_Control_Module_0 | kPWM_Control_Module_1, true);
}

void APP_SetDirectionPWM(int8_t direction) {
#define SERVO_PWM_MIN 3277  // Corresponds to -100%
#define SERVO_PWM_MAX 6554  // Corresponds to +100%

    int8_t pwmServo = direction;
    uint16_t pwmAccurateValue = 0;

    if (pwmServo < -100) {
        pwmServo = -100; // Limit to -100%
    } else if (pwmServo > 100) {
        pwmServo = 100; // Limit to 100%
    }

    // Map the direction value (-100 to 100) to the accurate PWM range (3277 to 6554)
    pwmAccurateValue = (uint16_t)(((pwmServo + 100) * (SERVO_PWM_MAX - SERVO_PWM_MIN)) / 200 + SERVO_PWM_MIN);  
    
    /* Update duty cycles for all servo PWM signals, high accuracy needed */
	PWM_UpdatePwmDutycycleHighAccuracy(FLEXPWM0_PERIPHERAL, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, pwmAccurateValue);

    /* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, kPWM_Control_Module_0 | kPWM_Control_Module_1, true);
}
