# STM32 FOC Motor Controller

Experimental STM32 firmware for field-oriented control of a three-phase
PMSM/BLDC motor.

## Features

- Field-oriented motor control (FOC)
- Clarke and Park transformations
- Space-vector PWM generation
- Three-phase current measurement
- Absolute encoder support
- CANopen communication
- UART telemetry

## Control architecture

The controller implements the main stages of a conventional FOC loop:

1. Phase currents are sampled and represented as a three-phase vector.
2. Clarke and Park transforms convert the measured currents into the rotating
   `d`/`q` reference frame using the rotor electrical angle.
3. PI regulators calculate the requested `d`- and `q`-axis voltages from the
   current errors.
4. Inverse transforms convert the voltage request back to the stationary frame.
5. The space-vector PWM block generates three duty cycles for the inverter.

The application also contains experimental torque, velocity, position,
calibration, idle, and current-control modes. Controller gains and limits are
defined in the source code and must be tuned for the motor, inverter, current
sensors, encoder, and control-loop period used by the target hardware.

CANopen is used for external commands and status exchange, while UART telemetry
can be used to inspect measurements and controller state during development.

## Sensors and connected devices

### Phase-current sensing

Three analog current channels measure the motor phase currents. The STM32
operational amplifiers condition the signals and the ADCs sample them in
synchronization with the PWM timer. These measurements are the feedback values
for the current-control loop.

### Absolute encoders

The firmware provides interfaces for two iC-MU150 absolute encoders: one for the
motor shaft and one for the gearbox/output shaft. Angular position is read over
separate SPI buses. The encoder angle is converted into the electrical angle
required by the Park transforms. An I2C path is also present for reading and
programming encoder EEPROM data.

### Power stage

The inverter is driven by three complementary PWM pairs. GPIO signals control
the gate-driver enable input and monitor its fault output. Hardware dead time and
fault protection must be verified for the actual inverter before enabling PWM.

### Communication interfaces

- **CAN/CANopen** — commands, configuration, and status exchange with a higher-
  level controller.
- **UART with DMA** — binary telemetry without blocking the control loop.
- **Auxiliary ADC input** — an additional analog channel reserved for board-level
  measurements such as temperature or supply monitoring.

## Runtime flow

1. Initialize the clock, GPIO, ADC, operational amplifiers, timers, encoders,
   communication interfaces, and power-stage control signals.
2. Receive operating commands and setpoints through CANopen or the local
   application state.
3. Sample phase currents and read the rotor position.
4. Calculate the electrical angle and execute the FOC current loop.
5. Convert the requested voltage vector into three PWM duty cycles.
6. Update the inverter outputs and publish diagnostic telemetry.
7. Monitor the gate-driver fault signal and move the power stage to a safe state
   when an error is detected.

## Project structure

- `Src/`, `Inc/` — application code and hardware initialization
- `foc/` — motor-control algorithms
- `parser_1010/` — telemetry protocol
- `CanOpen_config/` — CANopen integration
- `CanOpen_stack/` — CANopenNode sources
- `Drivers/` — STM32 HAL and CMSIS
- `stm32_g4veh_test.ioc` — STM32CubeMX configuration

## Building

### STM32CubeIDE

1. Install STM32CubeIDE with STM32G4 support.
2. Clone this repository.
3. In STM32CubeIDE, select **File → Import → Existing Projects into Workspace**.
4. Select the repository directory and import the project.
5. Build the `Debug` configuration.
6. Connect an ST-LINK programmer and flash the firmware.

### Command line

With the ARM GNU Toolchain and `make` available in `PATH`:

```sh
make -C Debug all
```

## Code formatting

The repository contains a `.clang-format` configuration. Run `clang-format`
on application source files before committing changes. Vendor and generated code
should not be reformatted.

## Safety

Remove mechanical load before initial testing. Verify current limits, PWM
polarity, fault handling, and emergency shutdown on a protected test bench before
connecting the final motor and power stage.

The current firmware is experimental and has not been qualified for unattended
or safety-critical operation.

## License

No project-wide license has been specified. Third-party components retain their
respective licenses.
