# Toolchain — working setup

Confirmed working: blink on ARIES V3, LED on GPIO_22.

## What's installed
- Toolchain: ~/vega-tools, triplet `riscv64-vega-elf`, GCC 10.1.0
  (note: captain's notes say riscv32-vega-elf / GCC 13.2.0 — different, both work)
- SDK: Taurus at ~/vega-sdk
- Flasher: ~/vega-xmodem
- Serial: /dev/ttyUSB0, user in `dialout` group

## Build a project
    cmake -B build -G Ninja
    cmake --build build

## Flash
    ~/vega-xmodem/xmodem.bat /dev/ttyUSB0 build/<name>.elf
- J12 jumper must be OPEN (UART boot)
- Press RESET if it says "Waiting for Reset"
- Close minicom first — only one program can hold the port

## Notes
- isl fix from captain's doc NOT needed on Ubuntu 22.04
- Ubuntu needs: sudo usermod -a -G dialout $USER
