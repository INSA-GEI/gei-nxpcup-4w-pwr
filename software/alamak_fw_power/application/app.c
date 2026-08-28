/*
 * app.c
 *
 *  Created on: 17 juin 2026
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

#include "configuration.h"

volatile APP_SystemData_t APP_SystemData = {
		.state = APP_STATE_STARTUP,
		.evtI2CDataChanged = false,
		.evtTimer1ms = false,
		.evtTimer10ms = false,
		.evtTimer100ms = false,
		.evtTimer1s = false,
		.evtShutdown = false,
		.odometer_1 = 0,
		.odometer_2 = 0,
		.motorcontrolEnabled = false
};

uint16_t APP_VbatFiltered = 0;
uint16_t APP_VbatFilteringArray[APP_VBAT_FILTERING_MESURES] = {0};
uint8_t APP_VbatFilteringIndex = 0;
uint32_t localFiltering=0;

void APP_1msTimerEventCallback(void);
void APP_10msTimerEventCallback(void);
void APP_100msTimerEventCallback(void);
void APP_1sTimerEventCallback(void);
void APP_I2CDataChangedEventCallback(void);
void APP_ShutdownEventCallback(void);

/**
 * @brief Initializes the application.
 * This function is called once at the beginning of the program to set up the necessary peripherals and
 * configurations for the application.
 */
void APP_Init(void)
{
	int i=0;
	uint32_t vbat=0;

	// Check battery level and perform a shutdown if it's too low
#if !DEBUG_NO_VBAT_MESURE_AT_STARTUP
	// les premieres mesures ADC sont foireuses, donc, campagne de lecture de 16 mesures
	// pour remettre l'ADC sur pied
	for (i=0; i<10; i++) {
		vbat += APP_GetRawADCValue(APP_ADC_CHANNEL_VBAT);
		SDK_DelayAtLeastUs(100000, CLOCK_GetCoreSysClkFreq());
	}

	vbat = APP_ConvertADCToVoltage(vbat/10);

	if (vbat < 610) { // VBAT < 6.10 V: Too low
		PRINTF("Battery too low (%u mV), shutting down...\r\n", vbat);
#if !DEBUG_NO_SHUTDOWN
		GPIO_PinWrite(BOARD_INITPINS_ONOFF_KILL_GPIO, BOARD_INITPINS_ONOFF_KILL_PIN, 0U);
#endif // !DEBUG_NO_SHUTDOWN

		while (1); // Attente de l'arret
	}
#endif // !DEBUG_NO_VBAT_MESURE_AT_STARTUP

	// Initialize I2C slave interface
	I2C_Slave_Init();

	// Start the odometer timers
	CTIMER_StartTimer(CTIMER0_PERIPHERAL);
	CTIMER_StartTimer(CTIMER1_PERIPHERAL);
}

/**
 * @brief Main function
 * Runs the main application loop.
 *
 */
void APP_Run(void)
{
	PRINTF("Rock'n'Roll, baby\r\n");
	PRINTF("\r\n");

	/*
	 * Main application loop
	 *
	 * This loop continuously checks for events and calls the corresponding callback functions.
	 * The events include:
	 * - 1-millisecond timer event
	 * - 10-millisecond timer event
	 * - 100-millisecond timer event
	 * - 1-second timer event
	 * - I2C data changed event
	 * - Shutdown event
	 */
	while (1)
	{
		// Handle 1-millisecond periodic task
		if (APP_SystemData.evtTimer1ms)
		{
			APP_SystemData.evtTimer1ms = false;
			APP_1msTimerEventCallback();
		}

		// Handle 10-millisecond periodic task
		if (APP_SystemData.evtTimer10ms)
		{
			APP_SystemData.evtTimer10ms = false;
			APP_10msTimerEventCallback();
		}

		// Handle 100-millisecond periodic task
		if (APP_SystemData.evtTimer100ms)
		{
			APP_SystemData.evtTimer100ms = false;
			APP_100msTimerEventCallback();
		}

		// Handle 1-second periodic task
		if (APP_SystemData.evtTimer1s)
		{
			APP_SystemData.evtTimer1s = false;
			APP_1sTimerEventCallback();
		}

		// Handle I2C data changed event
		if (APP_SystemData.evtI2CDataChanged)
		{
			APP_SystemData.evtI2CDataChanged = false;
			APP_I2CDataChangedEventCallback();
		}

		// Handle shutdown event
		if (APP_SystemData.evtShutdown)
		{
			APP_SystemData.evtShutdown=false;
			uint8_t CR_reg = I2C_Slave_GetReg(REG_CR);
			CR_reg = CR_reg | APP_CR_SHUTDOWN_REQUEST;
			APP_ShutdownEventCallback();
		}
	}
}

/**
 * @brief Callback function for I2C data changed event.
 * This function is called when the I2C data is changed.
 */
void APP_I2CDataChangedEventCallback(void)
{
	uint8_t CR_reg = I2C_Slave_GetReg(REG_CR);

	APP_SystemData.pwmMot1 = APP_GetMotorFromI2C(APP_MOTOR_1);
	APP_SystemData.pwmMot2 = APP_GetMotorFromI2C(APP_MOTOR_2);
	APP_SystemData.pwmServo = APP_GetDirectionFromI2C();

	/**************************************  
	 * Process Control Register (CR) bits *
	 **************************************/
	if (CR_reg & APP_CR_MOTOR_CONTROL_ENABLE) {
		APP_SystemData.motorcontrolEnabled = true;
	} else {
		APP_SystemData.motorcontrolEnabled = false;
	}

	if (CR_reg & APP_CR_SHUTDOWN_REQUEST) {
		APP_SystemData.evtShutdown = true;
		APP_SystemData.state = APP_STATE_IMX_SHUTDOWN;
	}

	if (CR_reg & APP_CR_RESET_ODOMETER) {
		APP_ResetRawOdometer(APP_ODOMETER_1);
		APP_ResetRawOdometer(APP_ODOMETER_2);

		/* Clear the reset bit */
		I2C_Slave_SetReg(REG_CR, CR_reg & ~APP_CR_RESET_ODOMETER);
	}

	if (CR_reg & APP_CR_CAMERA_LED) {
		GPIO_PinWrite(BOARD_INITPINS_CMD_CAMERA_LED_GPIO, BOARD_INITPINS_CMD_CAMERA_LED_PIN, 1U);
	} else {
		GPIO_PinWrite(BOARD_INITPINS_CMD_CAMERA_LED_GPIO, BOARD_INITPINS_CMD_CAMERA_LED_PIN, 0U);
	}

	/* Update PWM values */
	/*APP_SetMotorPWM(APP_MOTOR_1, APP_SystemData.pwmMot1);
	APP_SetMotorPWM(APP_MOTOR_2, APP_SystemData.pwmMot2);
	APP_SetDirectionPWM(APP_SystemData.pwmServo);*/

	APP_SetMotorsAndDirection(APP_SystemData.pwmMot1, APP_SystemData.pwmMot2, APP_SystemData.pwmServo);
}

/**
 * @brief Callback function for shutdown event.
 * This function is called when the shutdown event is received (ON/OFF button pressed) 
 * or when a critical battery level is detected.
 */
void APP_ShutdownEventCallback(void)
{
	PRINTF("Shutdown initiated !\r\n");
	PRINTF("--------------------\r\n");

	// TODO: A revoir, attendre que l'IMX8 s'arrete de son coté avant de couper l'alimentation
	/* Activation du shutdown */
#if !DEBUG_NO_SHUTDOWN
	if (APP_SystemData.evtShutdown == true)
	{
		GPIO_PinWrite(BOARD_INITPINS_ONOFF_KILL_GPIO, BOARD_INITPINS_ONOFF_KILL_PIN, 0U);
		while (1); // Attente de l'arret
	}
#else
	PRINTF("DEBUG_NO_SHUTDOWN is defined, shutdown is disabled for debugging purposes.\r\n");
#endif // !DEBUG_NO_SHUTDOWN
}

/**
 * @brief Callback function for 1-millisecond timer event.
 * This function is called every 1 millisecond to handle periodic tasks.
 */
void APP_1msTimerEventCallback(void)
{
	// get odometer values from CTIMER counters
	APP_SystemData.odometer_1 = CTIMER_GetTimerCountValue(CTIMER0_PERIPHERAL);
	APP_SystemData.odometer_2 = CTIMER_GetTimerCountValue(CTIMER1_PERIPHERAL);

	// Update odometer values to I2C registers
	APP_SetOdometerToI2C(APP_ODOMETER_1, APP_SystemData.odometer_1);
	APP_SetOdometerToI2C(APP_ODOMETER_2, APP_SystemData.odometer_2);
}

/**
 * @brief Callback function for 10-millisecond timer event.
 * This function is called every 10 milliseconds to handle periodic tasks.
 */
void APP_10msTimerEventCallback(void)
{
	APP_VbatFilteringArray[APP_VbatFilteringIndex] = APP_GetRawADCValue(APP_ADC_CHANNEL_VBAT);
	APP_VbatFilteringIndex++;

	if (APP_VbatFilteringIndex >= APP_VBAT_FILTERING_MESURES) {
		APP_VbatFilteringIndex = 0;

		for (uint8_t i = 0; i < APP_VBAT_FILTERING_MESURES; i++) {
			localFiltering += APP_VbatFilteringArray[i];
		}

		localFiltering /= APP_VBAT_FILTERING_MESURES;
		APP_VbatFiltered = APP_ConvertADCToVoltage(localFiltering);
		APP_SetVBat(APP_VbatFiltered);

		if (APP_VbatFiltered < APP_VBAT_CRITICAL_THRESHOLD) {
			PRINTF("Battery critical level reached (%u mV), shutting down requested...\r\n", APP_VbatFiltered);
			// TODO: voir si on peut faire un shutdown soft de l'IMX8 avant de couper l'alimentation
			APP_SystemData.state = APP_STATE_CRITICAL_LOW_BATTERY;
			APP_SystemData.evtShutdown = true;
		} else if (APP_VbatFiltered < APP_VBAT_LOW_THRESHOLD) {
			PRINTF("Battery low level reached (%u mV)\r\n", APP_VbatFiltered);

			APP_SystemData.state = APP_STATE_LOW_BATTERY;
		}
	}
}

/**
 * @brief Callback function for 100-millisecond timer event.
 * This function is called every 100 milliseconds to handle periodic tasks.
 */
void APP_100msTimerEventCallback(void)
{
	static uint8_t ledCounter = 0;

	// Led management
	switch (APP_SystemData.state) {
	case APP_STATE_RUNNING: // Led ON
		GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 1U);
		break;
	case APP_STATE_LOW_BATTERY: // Blink slowly
		if (ledCounter < 5) {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 1U);
		} else {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 0U);
		}
		break;
	case APP_STATE_CRITICAL_LOW_BATTERY: // blink fast
		if ((ledCounter/2 % 2) == 0) {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 1U);
		} else {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 0U);
		}
		break;
	case APP_STATE_IMX_SHUTDOWN:
		// Flash the LED
		if (ledCounter < 2) {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 1U);
		} else {
			GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 0U);
		}
	default: // Led OFF
		GPIO_PinWrite(BOARD_INITPINS_ALERT_LED_GPIO, BOARD_INITPINS_ALERT_LED_PIN, 0U);
		break;
	}

	ledCounter++;
	if (ledCounter >= 10) {
		ledCounter = 0;
	}
}

/**
 * @brief Callback function for 1-second timer event.
 * This function is called every 1 second to handle periodic tasks.
 */
void APP_1sTimerEventCallback(void)
{
	PRINTF("PWM 0A = %u%%\r\n", APP_SystemData.pwmMot1);
	PRINTF("PWM 0B = %u%%\r\n", APP_SystemData.pwmMot2);
	PRINTF("PWM 1A = %i%%\r\n", APP_SystemData.pwmServo);
	PRINTF("--------------------\r\n");

	PRINTF("Counter 0 = %lu\r\n", APP_SystemData.odometer_1);
	PRINTF("Counter 1 = %lu\r\n", APP_SystemData.odometer_2);
	PRINTF("--------------------\r\n");

	PRINTF("ADC = %lu\r\n", APP_GetVBat());
	PRINTF("--------------------\r\n");
	PRINTF("\r\n");
}

/**
 * @brief GPIO1 interrupt handler.
 * This function is called when an interrupt is received on GPIO1.
 * Raised when user presses the ON/OFF button. It sets the g_ExternalIT flag to true.
 */
void GPIO1_IRQHandler(void)
{
	__IO uint32_t tmp;

	/* Suppression de l'IT */
	tmp = GPIO_GpioGetInterruptFlags(BOARD_INITPINS_ONOFF_INT_GPIO);
	tmp = tmp | BOARD_INITPINS_ONOFF_INT_GPIO_PIN; // on ne conserve que la ligne d'IT comme interruption à effacer
	GPIO_GpioClearInterruptFlags(BOARD_INITPINS_ONOFF_INT_GPIO, tmp);
	/*tmp = GPIO1->ISFR[0];
	GPIO1->ISFR[0] = 0;
	GPIO1->ISFR[0] = tmp;
	GPIO1->ISFR[0] = 0;*/

	APP_SystemData.evtShutdown = true;

	SDK_ISR_EXIT_BARRIER;
}

/**
 * @brief SysTick interrupt handler.
 * This function is called every 1 millisecond.
 *
 * Used for periodic events generation for the application.
 */
void SysTick_Handler(void)
{
#define SYSTICK_PERIOD_1MS 1U
#define SYSTICK_PERIOD_10MS 10U
#define SYSTICK_PERIOD_100MS 100U
#define SYSTICK_PERIOD_1S 1000U

	static uint32_t counter_1ms = 0;
	static uint32_t counter_10ms = 0;
	static uint32_t counter_100ms = 0;
	static uint32_t counter_1s = 0;

	counter_1ms++;
	counter_10ms++;
	counter_100ms++;
	counter_1s++;

	if (counter_1s >= SYSTICK_PERIOD_1S)
	{
		counter_1s = 0;
		APP_SystemData.evtTimer1s = true;
	}

	if (counter_100ms >= SYSTICK_PERIOD_100MS)
	{
		counter_100ms = 0;
		APP_SystemData.evtTimer100ms = true;
	}

	if (counter_10ms >= SYSTICK_PERIOD_10MS)
	{
		counter_10ms = 0;
		APP_SystemData.evtTimer10ms = true;
	}

	if (counter_1ms >= SYSTICK_PERIOD_1MS)
	{
		counter_1ms = 0;
		APP_SystemData.evtTimer1ms = true;
	}

	/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
	 Store immediate overlapping exception return operation might vector to incorrect interrupt. */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
	__DSB();
#endif
}
