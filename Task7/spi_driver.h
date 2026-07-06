#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>

#define SPI_BASE_ADDR 0x00400040UL

/* Register offsets */
#define SPI_REG_CTRL    (*(volatile uint32_t *)(SPI_BASE_ADDR + 0x00))
#define SPI_REG_TXDATA  (*(volatile uint32_t *)(SPI_BASE_ADDR + 0x04))
#define SPI_REG_RXDATA  (*(volatile uint32_t *)(SPI_BASE_ADDR + 0x08))
#define SPI_REG_STATUS  (*(volatile uint32_t *)(SPI_BASE_ADDR + 0x0C))

/* CTRL register */
#define SPI_CTRL_EN             (1U << 0)
#define SPI_CTRL_START          (1U << 1)
#define SPI_CTRL_CLKDIV_SHIFT   8
#define SPI_CTRL_CLKDIV(x)      (((uint32_t)(x) & 0xFFU) << SPI_CTRL_CLKDIV_SHIFT)

/* STATUS register */
#define SPI_STATUS_BUSY         (1U << 0)
#define SPI_STATUS_DONE         (1U << 1)

/* Driver API */
void spi_init(uint8_t clkdiv);
uint8_t spi_transfer_byte(uint8_t tx_byte);
void spi_disable(void);

#endif /* SPI_DRIVER_H */
