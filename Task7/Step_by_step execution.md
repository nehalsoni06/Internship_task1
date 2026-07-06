# SPI Master IP — Step-by-Step Execution Guide

This is a practical run-through for building, flashing, and validating
the SPI Master IP on the VSDSquadron FM board. Follow the steps in order.
Each step has a placeholder for a screenshot — replace `[SCREENSHOT: ...]`
with your actual capture when you submit this.

---

## Step 1 — Confirm the RTL is present and unmodified

Open `riscv.v` and confirm the existing `spi_master` instantiation is
still there (it should already be present, per `Integration_Guide.md`).

```
Open: /home/vsduser/vsdfpga_labs/basicRISCV/RTL/riscv.v
Search for: spi_master SPI (
```


---

## Step 2 — Copy the driver files into your firmware project

Copy these three files into your firmware source folder:

```
software/spi_driver.h
software/spi_driver.c
software/example_loopback.c
```


---

## Step 3 — Wire the loopback jumper

Physically connect a single jumper wire from the `SPI_MOSI` pin to the
`SPI_MISO` pin on the VSDSquadron FM board (check your `.pcf` file for
which physical pin numbers these map to).

This lets you test the IP with zero external hardware — whatever byte
you send should come straight back.


---

## Step 4 — Build the firmware

Compile your firmware project as usual (replace with your actual build
command/toolchain):

```
make clean
make
```

---

## Step 5 — Flash to the board

Flash the compiled firmware to the VSDSquadron FM board:

```
make flash
```

(or whichever flashing command your lab setup uses)


---

## Step 6 — Open a UART terminal

Open a serial terminal (e.g. `screen`, `minicom`, or your IDE's built-in
terminal) at the board's baud rate (115200, per the existing UART
instantiation in `riscv.v`).

```
screen /dev/ttyUSB0 115200
```


---

## Step 7 — Reset the board and observe output

Press the reset button (or power-cycle) and watch the UART terminal.

**Expected output:**
```
SPI Loopback Test - jumper MOSI to MISO
Sent:     A5
Received: A5
PASS: loopback matched
```


---

## Step 8 — If it says FAIL instead

| Symptom | Check |
|---|---|
| `FAIL: loopback mismatch` | Jumper wire not making contact, or wrong pins |
| No output at all | Wrong baud rate, or UART pins not connected |
| Board doesn't reset / hangs | Re-check `SPI_BASE_ADDR` in `spi_driver.h` matches `IO_SPI_bit` in `riscv.v` |


---

## Step 9 — (Optional) Try the full example with a real SPI slave

Once loopback passes, wire up a real SPI slave device (e.g. SPI flash)
per `Integration_Guide.md`'s pin connections, and swap in
`software/example_main.c` instead of `example_loopback.c` to see a real
device-to-device transfer.


---
