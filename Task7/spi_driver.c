#include "spi_driver.h"

static uint8_t s_clkdiv = 0;

void spi_init(uint8_t clkdiv)
{
    s_clkdiv = clkdiv;
    SPI_REG_CTRL = SPI_CTRL_EN | SPI_CTRL_CLKDIV(clkdiv);
}

uint8_t spi_transfer_byte(uint8_t tx_byte)
{
    while (SPI_REG_STATUS & SPI_STATUS_BUSY);

    SPI_REG_TXDATA = tx_byte;

    SPI_REG_CTRL = SPI_CTRL_EN |
                   SPI_CTRL_CLKDIV(s_clkdiv) |
                   SPI_CTRL_START;

    while (!(SPI_REG_STATUS & SPI_STATUS_DONE));

    uint8_t rx_byte = (uint8_t)(SPI_REG_RXDATA & 0xFF);

    SPI_REG_STATUS = SPI_STATUS_DONE;

    return rx_byte;
}

void spi_disable(void)
{
    SPI_REG_CTRL = 0;
}
