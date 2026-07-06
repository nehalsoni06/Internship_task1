# SPI Master IP for VSDSquadron FM BasicRISCV

## Overview

This repository contains a memory-mapped **SPI (Serial Peripheral Interface) Master IP** for the VSDSquadron FM BasicRISCV SoC.

The IP enables the RISC-V processor to communicate with SPI peripherals such as Flash memory, sensors, ADCs, DACs, displays, and EEPROMs using a simple software driver and memory-mapped registers.

---

# Repository Structure

```
ip/
└── spi_master/
    ├── rtl/
    ├── software/
    ├── docs/
    └── README.md
```

---

# How to Integrate

1. Copy `rtl/spi_master.v` into your SoC project.
2. Instantiate the SPI Master in `riscv.v`.
3. Connect the address decoder to the SPI registers.
4. Expose the SPI signals (MOSI, MISO, SCLK and CS) to the FPGA top-level.
5. Add the software files:
   - `spi_driver.c`
   - `spi_driver.h`
6. Build the firmware and generate the `.hex` file.
7. Generate the FPGA bitstream and program the board.

📷 **RTL Integration Screenshot**

> *Insert Screenshot Here*

---

# Documentation

Detailed documentation is available in the **docs** folder.

| Document | Description |
|----------|-------------|
| IP_User_Guide.md | IP overview and features |
| Register_Map.md | Register descriptions |
| Integration_Guide.md | SoC integration steps |
| Example_Usage.md | Driver API and example code |
| Step_By_Step_Execution.md | Complete execution procedure |

---

# Software Files

```
software/
├── spi_driver.c
├── spi_driver.h
└── example_loopback.c
```

These files provide the driver API and a ready-to-run SPI loopback example.

---

# How to Test

### Firmware

Compile the firmware:

```bash
make clean
make example_loopback.hex
```

📷 **Firmware Compilation Screenshot**

> *Insert Screenshot Here*

---

### FPGA

1. Generate the FPGA bitstream.
2. Program the VSDSquadron FPGA.
3. Connect **MOSI** to **MISO** using a jumper wire.
4. Open the UART terminal.
5. Reset the FPGA.

Expected UART output:

```
SPI Loopback Test

Sent     : A5

Received : A5

PASS: loopback matched
```

📷 **UART Output Screenshot**

> *Insert Screenshot Here*

---

### Hardware Setup

Connect:

```
MOSI ─────────── MISO
```

SPI_MOSI (Pin 42)
        │
        └──────────────► SPI_MISO (Pin 43)

📷 **Hardware Setup Screenshot**

> *Insert Screenshot Here*

---

# Features

- Memory-mapped SPI Master
- 8-bit full-duplex transfer
- Configurable SPI clock divider
- Polling-based operation
- Software driver included
- Example application included
- Ready for VSDSquadron BasicRISCV integration

---

# Author

**Nehal Soni**

B.Tech Electronics & Communication Engineering

The LNM Institute of Information Technology (LNMIIT)

VSD FPGA Internship – Task 5
