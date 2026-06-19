/*
 * Copyright 2016-2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    alamak_fw_power.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
#include "app.h"

/* TODO: insert other definitions and declarations here. */
#define LPI2C_MASTER_SLAVE_ADDR_7BIT 0x42U
#define LPI2C_DATA_LENGTH            33U

//uint8_t g_master_txBuff[LPI2C_DATA_LENGTH];
//uint8_t g_master_rxBuff[LPI2C_DATA_LENGTH];
//uint8_t deviceAddress = 0x01U;
//size_t txCount        = 0xFFU;

//volatile bool g_ExternalIT = false;

//status_t I2C_Send(void) {
//	status_t reVal        = kStatus_Fail;
//
//	/* Send master blocking data to slave */
//	if (kStatus_Success == LPI2C_MasterStart(LPI2C0_PERIPHERAL, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Write))
//	{
//		/* Check master tx FIFO empty or not */
//		LPI2C_MasterGetFifoCounts(LPI2C0_PERIPHERAL, NULL, &txCount);
//		while (txCount)
//		{
//			LPI2C_MasterGetFifoCounts(LPI2C0_PERIPHERAL, NULL, &txCount);
//		}
//		/* Check communicate with slave successful or not */
//		if (LPI2C_MasterGetStatusFlags(LPI2C0_PERIPHERAL) & kLPI2C_MasterNackDetectFlag)
//		{
//			return kStatus_LPI2C_Nak;
//		}
//
//		/* subAddress = 0x01, data = g_master_txBuff - write to slave.
//		  start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
//		reVal = LPI2C_MasterSend(LPI2C0_PERIPHERAL, &deviceAddress, 1);
//		if (reVal != kStatus_Success)
//		{
//			if (reVal == kStatus_LPI2C_Nak)
//			{
//				LPI2C_MasterStop(LPI2C0_PERIPHERAL);
//			}
//			return -1;
//		}
//
//		reVal = LPI2C_MasterSend(LPI2C0_PERIPHERAL, g_master_txBuff, LPI2C_DATA_LENGTH);
//		if (reVal != kStatus_Success)
//		{
//			if (reVal == kStatus_LPI2C_Nak)
//			{
//				LPI2C_MasterStop(LPI2C0_PERIPHERAL);
//			}
//			return -1;
//		}
//
//		reVal = LPI2C_MasterStop(LPI2C0_PERIPHERAL);
//		if (reVal != kStatus_Success)
//		{
//			return -1;
//		}
//	}
//
//	return kStatus_Success;
//}

//void GPIO1_IRQHandler(void) {
//	__IO uint32_t tmp;
//
//	/* Suppression de l'IT */
//	tmp=GPIO_GpioGetInterruptFlags(BOARD_INITPINS_ONOFF_INT_GPIO);
//	tmp = tmp | BOARD_INITPINS_ONOFF_INT_GPIO_PIN; // on ne conserve que la ligne d'IT comme interruption à effacer
//	GPIO_GpioClearInterruptFlags(BOARD_INITPINS_ONOFF_INT_GPIO, tmp);
//	/*tmp = GPIO1->ISFR[0];
//	GPIO1->ISFR[0] = 0;
//	GPIO1->ISFR[0] = tmp;
//	GPIO1->ISFR[0] = 0;*/
//
//	g_ExternalIT = true;
//
//	SDK_ISR_EXIT_BARRIER;
//}
/*
 * @brief   Application entry point.
 */
int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif

	APP_Init();

	APP_Run();
//	CTIMER_StartTimer(CTIMER0_PERIPHERAL);
//	CTIMER_StartTimer(CTIMER1_PERIPHERAL);
//	PRINTF("Hello World\r\n");
//
//	/* Force the counter to be placed into memory. */
//	//volatile static int i = 0 ;
//	uint8_t pwmMot=0;
//	uint16_t pwmServo=3277;
//	uint32_t counter0, counter1;
//	lpadc_conv_result_t adc_result;
//
////	/* Set up i2c master to send data to slave*/
////	/* First data in txBuff is data length of the transmiting data. */
////	g_master_txBuff[0] = LPI2C_DATA_LENGTH - 1U;
////	for (uint32_t i = 1U; i < LPI2C_DATA_LENGTH; i++)
////	{
////		g_master_txBuff[i] = i - 1;
////	}
////
////	PRINTF("Master will send data :");
////	for (uint32_t i = 0U; i < LPI2C_DATA_LENGTH - 1U; i++)
////	{
////		if (i % 8 == 0)
////		{
////			PRINTF("\r\n");
////		}
////		PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
////	}
////	PRINTF("\r\n\r\n");
//
//	/* Enter an infinite loop, just incrementing a counter. */
//	while(1) {
//
//		PRINTF("PWM 0A = %u%%\r\n", pwmMot);
//		PRINTF("PWM 0B = %u%%\r\n", (100-pwmMot));
//		PRINTF("PWM 1A = %u\r\n", pwmServo);
//		/* Update duty cycles for all 2 motors PWM signals */
//		PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, pwmMot);
//		PWM_UpdatePwmDutycycle(FLEXPWM0_PERIPHERAL, kPWM_Module_0, kPWM_PwmB, kPWM_SignedCenterAligned, (100-pwmMot));
//
//		/* Update duty cycles for all servo PWM signals, high accuracy needed */
//		PWM_UpdatePwmDutycycleHighAccuracy(FLEXPWM0_PERIPHERAL, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, pwmServo);
//
//		/* Set the load okay bit for all submodules to load registers from their buffer */
//		PWM_SetPwmLdok(FLEXPWM0_PERIPHERAL, kPWM_Control_Module_0 | kPWM_Control_Module_1, true);
//
//		pwmMot+=10;
//
//		if (pwmMot>100) {
//			pwmMot=0;
//		}
//
//		pwmServo+=(3277/10);
//		if (pwmServo>6554) {
//			pwmServo=3277;
//		}
//
//		// Counter
//		counter0=CTIMER_GetTimerCountValue(CTIMER0_PERIPHERAL);
//		counter1=CTIMER_GetTimerCountValue(CTIMER1_PERIPHERAL);
//		PRINTF("Counter 0 = %lu\r\n", counter0);
//		PRINTF("Counter 1 = %lu\r\n", counter1);
//		PRINTF("--------------------\r\n");
//
//		// ADC
//		LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);
//		LPADC_GetConvResult(ADC0_PERIPHERAL, &adc_result);
//		PRINTF("ADC = %lu\r\n", adc_result.convValue);
//		PRINTF("--------------------\r\n");
//
////		PRINTF("Envoi de donnee I2C: ");
////		if (I2C_Send() != kStatus_Success) {
////			PRINTF("Echec\r\n");
////		} else {
////			PRINTF("Succes\r\n");
////		}
//
//		PRINTF("--------------------\r\n");
//		PRINTF("ON/OFF Int: ");
//		if (g_ExternalIT == true) {
//			PRINTF("Raised !\r\n");
//		} else {
//			PRINTF("No IT\r\n");
//		}
//
//		/* Activation du shutdown */
//		if (g_ExternalIT == true) {
//			GPIO_PinWrite(BOARD_INITPINS_ONOFF_KILL_GPIO, BOARD_INITPINS_ONOFF_KILL_PIN, 0U);
//
//			while (1); //Attente de l'arret
//		}
//
//		g_ExternalIT = false;
//		PRINTF("\r\n");
//
//		/** Attente de 1 seconde*/
//		/* wait around 1 second. */
//		SDK_DelayAtLeastUs(1000000 ,SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
//	}

	return 0 ;
}
