/*
 * configuration.h
 *
 *  Created on: 24 juin 2026
 *      Author: dimercur
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/* Version */
#define FW_VERSION				1U

/* Configuration du driver I2C esclave */
#define I2C_SLAVE_ADDR         0x42U
#define LPI2C_CLOCK_FREQ       24000000U
#define LPI2C_SLAVE_BASE       LPI2C0
#define RX_BUFFER_SIZE         32U

/* Mesures de filtrage de la tension de batterie */
#define APP_VBAT_FILTERING_MESURES 		50

/* Constantes liées à l'arret */
#define DEBUG_NO_SHUTDOWN 				1U
#define DEBUG_NO_VBAT_MESURE_AT_STARTUP 0U
#define SHUTDOWN_DELAY_MS 				5000U // Délai avant l'arrêt de l'alimentation après un shutdown (en millisecondes)

#endif /* CONFIGURATION_H_ */
