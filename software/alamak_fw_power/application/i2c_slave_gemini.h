/*
 * i2c_slave.h
 * Description : Déclarations pour le driver I2C esclave (LPI2C0) du MCXA153.
 */

#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

#include <stdint.h>
#include <stdbool.h>
#include "fsl_lpi2c.h"

/* Flag global mis à true lorsqu'un registre RW est modifié par le maître I2C */
extern volatile bool gI2CChanged;

/* Définition des index de registres */
typedef enum {
    REG_ID       = 0x00, /* RO : Identifiant (0x10) */
    REG_SR       = 0x01, /* RO : Indicateurs PWR */
    REG_VBAT_1R  = 0x02, /* RO : VBAT LSB */
    REG_VBAT_2R  = 0x03, /* RO : VBAT MSB */
    REG_ODO1_1R  = 0x04, /* RO : Odo 1 LSB */
    REG_ODO1_2R  = 0x05, /* RO : Odo 1 */
    REG_ODO1_3R  = 0x06, /* RO : Odo 1 */
    REG_ODO1_4R  = 0x07, /* RO : Odo 1 MSB */
    REG_ODO2_1R  = 0x08, /* RO : Odo 2 LSB */
    REG_ODO2_2R  = 0x09, /* RO : Odo 2 */
    REG_ODO2_3R  = 0x0A, /* RO : Odo 2 */
    REG_ODO2_4R  = 0x0B, /* RO : Odo 2 MSB */
    REG_MOT1     = 0x0C, /* RW : Moteur 1 (0 - 100) */
    REG_MOT2     = 0x0D, /* RW : Moteur 2 (0 - 100) */
    REG_DIR      = 0x0E, /* RW : Direction (-100 - 100) */
    REG_CR       = 0x0F, /* RW : Control Register */
    REG_MAX_IDX  = 0x0F  /* Index maximum avant retour à 0 */
} i2c_reg_index_t;

/**
 * @brief Initialise le périphérique LPI2C0 en mode esclave.
 */
void I2C_Slave_Init(void);

/**
 * @brief Getter privé au MCU (application) pour lire la valeur d'un registre.
 * @param index L'index du registre.
 * @return La valeur contenue dans le registre.
 */
uint8_t I2C_Slave_GetReg(i2c_reg_index_t index);

/**
 * @brief Setter privé au MCU (application) pour mettre à jour la valeur d'un registre.
 * @note Cette fonction est utilisée par le MCU pour mettre à jour les valeurs RO (ex: capteurs).
 * @param index L'index du registre.
 * @param value La nouvelle valeur.
 */
void I2C_Slave_SetReg(i2c_reg_index_t index, uint8_t value);

#endif /* I2C_SLAVE_H_ */
