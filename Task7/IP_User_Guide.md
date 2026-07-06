# SPI Master IP — User Guide

## 1. IP Overview

The **SPI Master IP** is a single-slave, single-byte SPI controller
integrated into the VSDSquadron FM BasicRISCV SoC. It performs one
full-duplex 8-bit SPI transaction per software-triggered "start", with
hardware-managed chip-select and clock generation.

**Typical use cases:**
- Talking to SPI Flash, ADCs, DACs, sensors, or other single-slave SPI
  peripherals from bare-metal RISC-V firmware
- Bit-banged-free SPI transfers where the CPU just loads a byte, starts
  the transfer, and polls for completion

**When to use it:** whenever firmware needs a hardware-timed SPI
transaction to a single external device without hand-rolling GPIO
bit-banging.

## 2. Feature Summary

| Feature | Supported |
|---|---|
| Transfer size | 8 bits (1 byte) per transaction |
| Full-duplex (simultaneous TX/RX) | Yes |
| Bit order | MSB-first |
| SPI mode | Mode 0 only (CPOL=0, CPHA=0 — sample on rising edge, shift on falling edge) |
| Clock divider | 8-bit, software-programmable |
| Chip-select | Automatic, hardware-driven, single slave only |
| Status polling | Yes (BUSY + DONE flags) |
| Interrupt output | **No** — polling only |
| Multi-byte / burst transfers | **No** — one byte per start, software must re-trigger for each byte |
| Multiple slaves / manual CS control | **No** — single `cs_n` output, automatically toggled per transfer |

**Limitations (stated up front, not buried):**
- Only 8-bit transfers — for multi-byte protocols, firmware issues one
  start per byte and manages any inter-byte CS behavior itself (CS is
  deasserted between every single-byte transfer since it's automatic).
- Only SPI Mode 0 is implemented — CPOL/CPHA are not configurable.
- Only one chip-select line (`cs_n`) — supporting more than one SPI
  slave requires external address decoding/muxing of `cs_n` plus shared
  `sclk`/`mosi`/`miso`, or a separate IP instance.
- No interrupt — firmware must poll the `STATUS` register.
- `TXDATA` (offset `0x4`) is write-only — reading it returns 0.
- `RXDATA` (offset `0x8`) is read-only — writes to it are silently
  ignored (not decoded in the write path).

## 3. Block Diagram

```
        SOC Bus (mem_addr / mem_wdata / mem_wstrb / mem_rstrb)
                       |
                       v
       isIO & mem_wordaddr[IO_SPI_bit]   (one-hot select, riscv.v)
                       |
                       v
        +----------------------------+
        |     Register Decode        |   sel = addr[3:2]
        |  CTRL / TXDATA / RXDATA /   |
        |         STATUS              |
        +----------------------------+
                       |
                       v
        +----------------------------+
        |   Clock Divider (clkdiv)   |
        +----------------------------+
                       |
                       v
        +----------------------------+
        |  Shift Register FSM        |
        |  IDLE -> XFER -> DONE_ST    |
        +----------------------------+
                 |           |
                 v           v
             sclk/mosi     miso (sampled)
                 |
                 v
              cs_n (auto, low during XFER only)
```

See `Register_Map.md` for the full register/bit-field reference,
`Integration_Guide.md` for how this is wired into `riscv.v`, and
`Example_Usage.md` for a working transfer routine.
