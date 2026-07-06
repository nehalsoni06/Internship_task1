# SPI Master IP — Register Map

**SoC integration:** memory-mapped into the BasicRISCV SoC's I/O page via
`IO_SPI_bit = 4` (see `Integration_Guide.md`). Effective base address:
`0x00400040`.

**Access width:** 32-bit, word-aligned. Register select uses `addr[3:2]`
(`sel`), giving 4 register slots at offsets `0x0/0x4/0x8/0xC`.

## Register Summary

| Offset | sel | Register | R/W | Reset Value | Description |
|--------|-----|----------|-----|--------------|--------------|
| 0x0 | 0 | CTRL | R/W | 0x0000_0000 | Enable, clock divider, transfer-start strobe |
| 0x4 | 1 | TXDATA | W only (reads as 0) | 0x0000_0000 | Byte to transmit |
| 0x8 | 2 | RXDATA | R only (writes ignored) | 0x0000_0000 | Last received byte |
| 0xC | 3 | STATUS | R + W1C(DONE) | 0x0000_0000 | Busy / Done flags |

All registers reset to `0x0000_0000` on `rst_n` deassertion — confirmed
directly from the RTL's reset block: `{en,busy,done,half} <= 0;
{clkdiv,tx,rx,cnt,tx_shift,rx_shift,bits} <= 0;`.

---

### 0x0 (sel=0) — CTRL

**Write behavior** (`{clkdiv, en} <= {write_data[15:8], write_data[0]}`,
every write to this offset unconditionally updates both fields):

| Bits | Name | Reset | Description |
|------|------|-------|-------------|
| 0 | EN | 0 | 1 = SPI block enabled. Written every time CTRL is written. |
| 1 | START | 0 (not stored) | Not stored — used only combinationally as a transfer-trigger strobe (see below). Reads back as 0. |
| 7:2 | — | 0 | Not used by the write path |
| 15:8 | CLKDIV | 0x00 | 8-bit SPI clock divider. Written every time CTRL is written. |
| 31:16 | — | 0 | Not used by the write path |

**Read behavior** (`{16'h0, clkdiv, 6'b0, 1'b0, en}`):

| Bits | Reset | Description |
|------|-------|-------------|
| 0 | 0 | EN (current value) |
| 7:1 | 0 | Reads as 0 (START is not stored, so it always reads back 0) |
| 15:8 | 0x00 | CLKDIV (current value) |
| 31:16 | 0 | Reads as 0 |

**Start condition (critical detail):** a transfer only actually begins
when, in the *same write* to CTRL, all of the following are true:
- `write_data[1]` (START) = 1
- Either the block was already enabled (`en` was 1 from a previous
  write) **or** this same write also sets `write_data[0]` (EN) = 1
- The FSM is in `IDLE` and not `busy`

If any condition is false, the write still updates EN/CLKDIV as normal,
but no transfer starts.

### 0x4 (sel=1) — TXDATA

| Bits | R/W | Reset | Description |
|------|-----|-------|-------------|
| 7:0 | W | 0x00 | Byte to transmit on the next start. Latched into the shift register when the transfer begins. |
| 31:8 | — | 0 | Ignored on write |

Reading this offset always returns `0x00000000` — TXDATA is not
readable back.

### 0x8 (sel=2) — RXDATA

| Bits | R/W | Reset | Description |
|------|-----|-------|-------------|
| 7:0 | R | 0x00 | Last byte received, valid once `STATUS.DONE` is set |
| 31:8 | — | 0 | Reads as 0 |

Writes to this offset have no effect — there is no write case for
`sel==2` in the RTL, so any write is silently dropped.

### 0xC (sel=3) — STATUS

**Write behavior** (only bit 1 is decoded):

| Bits | R/W | Reset | Description |
|------|-----|-------|-------------|
| 1 | W1C | 0 | Writing 1 clears `DONE`. Writing 0 has no effect. |
| all other bits | — | 0 | Not decoded on write |

**Read behavior** (`{29'b0, 1'b0, done, busy}`):

| Bits | Reset | Description |
|------|-------|-------------|
| 0 | 0 | BUSY — 1 while a transfer is in progress |
| 1 | 0 | DONE — 1 once a transfer completes; write 1 to clear |
| 2 | 0 | Reads as 0 |
| 31:3 | 0 | Reads as 0 |

---

## Timing / Behavioral Notes

- **SPI clock rate:** `f_sclk = f_clk / (2 * (CLKDIV + 1))`, where
  `f_clk` is the SoC's system clock. Each SCLK half-period takes
  `CLKDIV + 1` system clock cycles.
- **SPI mode:** Mode 0 (CPOL=0, CPHA=0) — `MISO` is sampled on the
  rising edge of `SCLK`; `MOSI`/shift register update on the falling
  edge. Not configurable.
- **Bit order:** MSB-first, 8 bits per transaction, full-duplex (`MOSI`
  drives out while `MISO` is simultaneously captured).
- **Chip-select:** `cs_n` is driven low automatically only while the FSM
  is in the `XFER` state, and high otherwise (`IDLE`, `DONE_ST`, and
  reset). There is no software-controlled CS register.
- **Read data is combinational** — `read_data` reflects the selected
  register the same cycle `read_en` is asserted, with no pipeline delay.
