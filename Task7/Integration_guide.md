# SPI Master IP — Integration Guide (VSDSquadron FM)

This IP is already integrated into the BasicRISCV SoC's `riscv.v` (top
module `SOC`). This guide documents exactly how it's wired there, so you
can either reuse the existing integration as-is or replicate the pattern
if instantiating a second instance / porting to another SoC.

## 1. Required RTL Files

```
rtl/spi_master.v
```

Single file, no sub-modules, no external dependencies.

## 2. Where It's Instantiated

Inside `SOC`, alongside the other peripherals (`Processor CPU`,
`Memory RAM`, `corescore_emitter_uart UART`):

```verilog
wire spi_write_en = isIO & mem_wstrb & mem_wordaddr[IO_SPI_bit];
wire spi_read_en  = isIO & mem_rstrb & mem_wordaddr[IO_SPI_bit];

spi_master SPI (
    .clk(clk),
    .rst_n(resetn),
    .write_data(mem_wdata),
    .write_en(spi_write_en),
    .read_en(spi_read_en),
    .addr(mem_addr[3:0]),
    .read_data(spi_rdata),
    .miso(SPI_MISO),
    .sclk(SPI_SCLK),
    .mosi(SPI_MOSI),
    .cs_n(SPI_CS_N)
);
```

`spi_rdata` is then muxed into the shared `IO_rdata` bus alongside the
other peripherals' read data (`gpio_rdata`, etc.), selected by
`mem_wordaddr[IO_SPI_bit]`.

## 3. Address Decoding

- `isIO` = `mem_addr[22]` selects the I/O page vs RAM.
- Inside the I/O page, one-hot bit `IO_SPI_bit = 4` selects this
  peripheral: `mem_wordaddr[IO_SPI_bit]`, i.e. `mem_addr[31:2]` bit 4.
- Effective byte address: `(1 << 22) | (1 << (4 + 2))` = `0x400040`.
- Only `mem_addr[3:0]` is routed into the module as `addr` — this gives
  the 4 register slots (`0x0/0x4/0x8/0xC`) described in
  `Register_Map.md`.

If you're integrating a **second** SPI instance (e.g. for a second
slave device), you must pick a new, currently-unused one-hot bit (the
existing ones in `riscv.v` are `IO_LEDS_bit=0`, `IO_UART_DAT_bit=1`,
`IO_UART_CNTL_bit=2`, `IO_GPIO_bit=3`, `IO_SPI_bit=4`) — e.g. add
`IO_SPI2_bit = 5`, wire its own `write_en`/`read_en`, and add a branch
to the `IO_rdata` mux, following the exact same pattern shown above for
`spi_write_en`/`spi_read_en`.

## 4. Signals Exposed to Top-Level

| Signal | Direction | Notes |
|---|---|---|
| `clk` | in | SoC's internal `clk` (from `Clockworks CW`) |
| `rst_n` | in | SoC's internal `resetn` |
| `write_data[31:0]` | in | `mem_wdata` |
| `write_en` | in | `isIO & mem_wstrb & mem_wordaddr[IO_SPI_bit]` |
| `read_en` | in | `isIO & mem_rstrb & mem_wordaddr[IO_SPI_bit]` |
| `addr[3:0]` | in | `mem_addr[3:0]` |
| `read_data[31:0]` | out | Combinational, feeds the `IO_rdata` mux |
| `miso` | in | Physical SPI MISO pin |
| `sclk` | out | Physical SPI clock pin |
| `mosi` | out | Physical SPI MOSI pin |
| `cs_n` | out | Physical SPI chip-select pin, active low, hardware-toggled automatically |

## 5. Clock Assumptions

- Single clock domain — `clk`/`resetn` come from the SoC's own
  `Clockworks CW` block.
- SPI clock rate is derived from the system clock and the software-set
  `CLKDIV` field: `f_sclk = f_clk / (2*(CLKDIV+1))`. There is no
  separate SPI-domain clock input.

## 6. Board-Level / Pin Connections (VSDSquadron FM)

`spi_master` already exposes its four SPI signals as **top-level ports
on the `SOC` module itself**:

```verilog
module SOC (
    input       CLK,
    input       RESET,
    output reg [4:0] LEDS,
    input       RXD,
    output      TXD,

    output      SPI_SCLK,
    output      SPI_MOSI,
    input       SPI_MISO,
    output      SPI_CS_N
);
```

These must be constrained to physical FPGA pins in your board's `.pcf`
file (matched to whichever header/pins your SPI slave device is wired
to on the VSDSquadron FM board). Example `.pcf` entries (pin numbers
are board-specific — confirm against your board's pinout documentation
before flashing):

```
set_io SPI_SCLK  <pin_number>
set_io SPI_MOSI  <pin_number>
set_io SPI_MISO  <pin_number>
set_io SPI_CS_N  <pin_number>
```

No additional constraint entries are needed for the internal bus
signals (`addr`, `write_data`, etc.) since those stay inside the SoC
fabric and never reach a physical pin.
