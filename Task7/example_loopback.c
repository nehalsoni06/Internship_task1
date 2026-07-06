#include <stdint.h>
#include "spi_driver.h"

int printf(const char *fmt, ...);

#define SPI_CLKDIV 9

int main(void)
{
    printf("SPI Loopback Test\r\n");

    spi_init(SPI_CLKDIV);

    uint8_t tx = 0xA5;
    uint8_t rx = spi_transfer_byte(tx);

    printf("Sent     : %x\r\n", tx);
    printf("Received : %x\r\n", rx);

    if (rx == tx)
        printf("PASS: loopback matched\r\n");
    else
        printf("FAIL: loopback mismatch\r\n");

    spi_disable();

    while (1);

    return 0;
}
