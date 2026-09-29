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
- Three-phase current sensors
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

1. Connect the target board and ST-LINK.
2. Flash the `Debug` image from STM32CubeIDE and reset the board.
3. Connect UART4 at 576000 baud to receive telemetry if required.

After reset, the firmware initializes CANopen as node 1 at 500 kbit/s, enables
the gate driver, and starts complementary PWM in the open-loop `moment` mode.
