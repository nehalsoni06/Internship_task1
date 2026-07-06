# SPI Master IP — Example Usage

## 1. Software Programming Model

Typical sequence to perform an SPI transfer:

1. **Initialize** — write `CTRL` with `EN=1` and the desired `CLKDIV`
   (no `START` bit set yet).
2. **Load TX data** — write the byte to send into `TXDATA`.
3. **Trigger the transfer** — write `CTRL` again with `EN=1`, the same
   `CLKDIV`, and `START=1` in the same write. (`EN` and `CLKDIV` must be
   re-supplied because every CTRL write overwrites both fields — see
   `Register_Map.md`.)
4. **Poll `STATUS.BUSY`** until it clears, or poll `STATUS.DONE` until
   it sets.
5. **Read `RXDATA`** for the byte received during the transfer.
6. **Clear `STATUS.DONE`** by writing 1 to it, before starting the next
   transfer.

Repeat steps 2–6 for each additional byte — this IP does not support
multi-byte burst transfers in a single trigger.

## 2. Files Provided

```
software/spi_driver.h      - register map + driver API
software/spi_driver.c      - driver implementation
software/example_main.c    - ready-to-run example application
```

## 3. Example Application Walkthrough

`example_main.c` initializes the SPI block, then sends a 4-byte sequence
(illustrative — e.g. an SPI flash "read JEDEC ID" style command),
printing each transmitted/received byte pair over UART:

```c
#define SPI_CLKDIV   4   /* f_sclk = f_clk / (2*(4+1)) = f_clk / 10 */

spi_init(SPI_CLKDIV);

uint8_t test_bytes[4] = {0x9F, 0x00, 0x00, 0x00};
for (int i = 0; i < 4; i++) {
    uint8_t rx = spi_transfer_byte(test_bytes[i]);
    /* print tx/rx pair over UART */
}

spi_disable();
```

**Adjust `SPI_CLKDIV` for your system clock and target slave's maximum
SPI frequency:** `f_sclk = f_clk / (2*(CLKDIV+1))`. Increase `CLKDIV` to
slow the SPI clock down if your slave device can't keep up.

Replace `uart_print()`/`uart_print_hex_byte()` with your actual UART
driver calls (this SoC already has `corescore_emitter_uart` wired in
`riscv.v` — reuse that path).

## 4. Building and Running

1. `rtl/spi_master.v` is already integrated in `riscv.v` — no RTL changes
   needed unless you're adding a second instance (see
   `Integration_Guide.md`).
2. Add `software/spi_driver.c`, `spi_driver.h`, and `example_main.c` to
   your firmware build.
3. Wire a real SPI slave device to the board's `SPI_SCLK`/`SPI_MOSI`/
   `SPI_MISO`/`SPI_CS_N` pins per your `.pcf` constraints.
4. Flash and run on the VSDSquadron FM board.

## 5. Validation & Expected Output

**Expected behavior once running (with an SPI flash or similar slave
connected):**

```
SPI Master Example: sending test bytes
TX/RX: 9F / <manufacturer ID byte>
TX/RX: 00 / <device ID byte 1>
TX/RX: 00 / <device ID byte 2>
TX/RX: 00 / <device ID byte 3>
SPI transfer sequence complete
```

The exact RX bytes depend entirely on what slave device is attached and
what command byte was sent — the example above assumes an SPI flash
JEDEC ID read for illustration.

**Common failure symptoms and likely causes:**

| Symptom | Likely Cause |
|---|---|
| RX byte is always `0x00` or `0xFF` | No slave connected, or `SPI_MISO`/`SPI_MOSI`/`SPI_SCLK`/`SPI_CS_N` `.pcf` pins don't match the actual wiring |
| Firmware hangs waiting for `DONE` | `START` write didn't actually trigger — check `EN` was already 1 or set in the *same* CTRL write as `START` (see step 3 above) |
| `SPI_CLKDIV` seems to reset to 0 unexpectedly | A later CTRL write (e.g. the transfer-trigger write) omitted the `CLKDIV` field — every CTRL write must re-supply it |
| Garbage/incorrect RX data | Slave device doesn't support SPI Mode 0, or `SPI_CLKDIV` is too fast for the slave's timing |
| Firmware hangs waiting for `BUSY` to clear on the *first* transfer | `spi_init()` wasn't called first, or `EN` was never set |
