# STM32 FOC Motor Controller

Experimental firmware for a three-phase PMSM/BLDC inverter based on the
STM32G474VE. It contains Clarke/Park transforms, PI controllers, SVPWM,
three-phase current acquisition, CANopen communication, encoder drivers, and
UART telemetry.

The current startup mode is an open-loop rotating voltage vector. Encoder
feedback and closed-loop current control are not active.

## Requirements

- STM32CubeIDE with STM32G4 support and the ARM GNU Toolchain
- ST-LINK programmer/debugger
- STM32G474VE target board and compatible three-phase inverter
- Current sensors and a current-limited bench supply
- Optional: iC-MU150 encoder, CAN interface, and 576000-baud UART connection

STM32 HAL/CMSIS code is under `Drivers/`. CANopenNode is under
`CanOpen_stack/`. The CubeMX configuration is `stm32_g4veh_test.ioc`.

## Build

1. Clone the repository.
2. Open STM32CubeIDE and select **File → Import → Existing Projects into
   Workspace**.
3. Select the repository directory.
4. Build the `Debug` configuration.

After STM32CubeIDE has generated `Debug/`, the same configuration can be rebuilt
from a terminal with:

```sh
make -C Debug all
```

## Flash and run

1. Disconnect the motor or remove the mechanical load.
2. Power the logic side and connect ST-LINK.
3. Flash the `Debug` image from STM32CubeIDE.
4. Verify PWM polarity, dead time, current scaling, and the gate-driver enable
   signal with the power stage current-limited.
5. Connect UART4 at 576000 baud for telemetry if required.

The firmware enables the gate driver and starts complementary PWM during
startup. `DRIVER_FAULT` is configured as an input but does not currently trigger
a software shutdown. Do not use this firmware on an energized power stage until
fault handling, current limits, encoder feedback, and emergency shutdown have
been validated for the actual hardware.
