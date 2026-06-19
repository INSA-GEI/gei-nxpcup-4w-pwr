/*
 * i2c_slave.c
 * Description : Implémentation du driver I2C esclave sur LPI2C0.
 */

#include "i2c_slave_gemini.h"
#include <string.h>

#include "app.h"

#define I2C_SLAVE_ADDR         0x42U
#define LPI2C_CLOCK_FREQ       24000000U
#define LPI2C_SLAVE_BASE       LPI2C0
#define RX_BUFFER_SIZE         32U

/* Variables globales et privées */
//volatile bool gI2CChanged = false;

static uint8_t i2c_regs[REG_MAX_IDX + 1];
static lpi2c_slave_handle_t g_s_handle;

/* Variables d'état pour la machine I2C */
static uint8_t current_reg_index = 0;
static uint8_t rx_buf[RX_BUFFER_SIZE];
static uint8_t tx_byte;
static bool is_receiving = false;

/* Fonction utilitaire : vérifie si un registre est inscriptible par le maître (RW) */
static bool is_rw_register(uint8_t index)
{
    return (index >= REG_MOT1 && index <= REG_CR);
}

/* * Callback appelé sous interruption par le driver fsl_lpi2c
 */
static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *param)
{
    switch (xfer->event)
    {
        /* Le maître a adressé l'esclave */
        case kLPI2C_SlaveAddressMatchEvent:
            is_receiving = false;
            xfer->data = NULL;
            xfer->dataSize = 0;
            break;

        /* Le maître veut écrire des données */
        case kLPI2C_SlaveReceiveEvent:
            is_receiving = true;
            xfer->data = rx_buf;
            xfer->dataSize = RX_BUFFER_SIZE;
            break;

        /* Le maître veut lire des données */
        case kLPI2C_SlaveTransmitEvent:
            is_receiving = false;

            /* Prépare l'octet à transmettre et incrémente l'index avec bouclage */
            tx_byte = i2c_regs[current_reg_index];
            xfer->data = &tx_byte;
            xfer->dataSize = 1; /* Forcer le traitement octet par octet */

            current_reg_index++;
            if (current_reg_index > REG_MAX_IDX) {
                current_reg_index = 0;
            }
            break;

        /* Fin de transaction (Stop) ou condition de Restart (Repeated Start) */
        case kLPI2C_SlaveCompletionEvent:
        case kLPI2C_SlaveRepeatedStartEvent:
            if (is_receiving && (xfer->transferredCount > 0))
            {
                /* Le premier octet reçu est toujours l'index du registre visé */
                current_reg_index = rx_buf[0];
                if (current_reg_index > REG_MAX_IDX) {
                    current_reg_index = 0;
                }

                /* Traitement des octets de données suivants (s'il y en a) */
                for (size_t i = 1; i < xfer->transferredCount; i++)
                {
                    uint8_t val = rx_buf[i];

                    /* Seuls les registres RW sont modifiables par le maître */
                    if (is_rw_register(current_reg_index))
                    {
                        if (i2c_regs[current_reg_index] != val)
                        {
                            i2c_regs[current_reg_index] = val;
                            APP_SystemData.evtI2CDataChanged = true;
                        }
                    }

                    /* Incrémentation auto avec bouclage au-delà du registre CR (0x12) */
                    current_reg_index++;
                    if (current_reg_index > REG_MAX_IDX) {
                        current_reg_index = 0;
                    }
                }
            }
            is_receiving = false;
            xfer->data = NULL;
            xfer->dataSize = 0;
            break;

        default:
            break;
    }
}

void I2C_Slave_Init(void)
{
    lpi2c_slave_config_t slaveConfig;

    /* Initialisation de la mémoire des registres */
    memset(i2c_regs, 0, sizeof(i2c_regs));
    i2c_regs[REG_ID] = 0x10; /* L'ID carte est toujours 0x10 */

    /* Configuration par défaut du SDK */
    LPI2C_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.address0 = I2C_SLAVE_ADDR;

    /* Vitesse fixée physiquement par le maître (100kHz), le filtre de glitch
       s'appuie sur la fréquence source définie par LPI2C_CLOCK_FREQ */
    LPI2C_SlaveInit(LPI2C_SLAVE_BASE, &slaveConfig, LPI2C_CLOCK_FREQ);

    /* Création du handle avec notre callback */
    LPI2C_SlaveTransferCreateHandle(LPI2C_SLAVE_BASE, &g_s_handle, lpi2c_slave_callback, NULL);

    /* Initialisation du flag de réception de données I2C */
    APP_SystemData.evtI2CDataChanged = false;

    /* Lancement de l'écoute non-bloquante avec les masques d'interruption */
    LPI2C_SlaveTransferNonBlocking(LPI2C_SLAVE_BASE, &g_s_handle,
                                   kLPI2C_SlaveCompletionEvent |
                                   kLPI2C_SlaveAddressMatchEvent |
                                   kLPI2C_SlaveRepeatedStartEvent);
}

uint8_t I2C_Slave_GetReg(i2c_reg_index_t index)
{
    if (index <= REG_MAX_IDX) {
        return i2c_regs[index];
    }
    return 0;
}

void I2C_Slave_SetReg(i2c_reg_index_t index, uint8_t value)
{
    /* L'ID de carte ne doit jamais être écrasé, même par le MCU */
    if (index > REG_ID && index <= REG_MAX_IDX) {
        i2c_regs[index] = value;
    }
}
