Bandicoot Embedded
===================

This repo contains all the code which runs on the "Bandicoot" rover, from the Off-World Robotics team at UNSW.

Bandicoot is still a work in progress and hence this repo may change significantly over time as various systems are developed.

This code is designed to run on TI's [TM4C123G Launchpad](https://www.ti.com/tool/EK-TM4C123GXL) which contains an ARM Cortex-M4 TM4C123GH6PM microcontroller.

All code is currently written in C.

## Project Structure

The current project contains the following modules:

* Proportional-Integral-Derivative (PID) controller
* Pulse-width modulation (PWM) interface
* Quadrature encoder interface (QEI)
* Fixed-point math library (currently unused)

These modules are located in the `src/` directory along with TI's Tivaware software library source code.

In addition to these modules, test programs have been written to verify their operation. All code used for testing purposes is located in the `test/` directory.

## Building and Running

### Dependencies

This project requires the following:

* GNU Make
* OpenOCD
* [GNU ARM Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

GNU Make and OpenOCD can be installed via a package manager if they are not installed already. The GNU ARM embedded toolchain must be built from source and may require the binaries to be symlinked to `/usr/local/bin`. The **Troubleshooting** section below may help with installing the ARM embedded toolchain.

### Build & Run Processes

1. Install all above dependencies
2. Download/clone this git repo:

    `git clone https://github.com/Offworld-Robotics/bandicoot_embedded`

3. Run `make` to build the project
4. Make sure the `PWR SELECT` switch next to the USB port on the board is set to `DEBUG`
5. Start an OpenOCD process in the background to connect to the TM4C123G board:

    `openocd -f /usr/local/share/openocd/scripts/board/ek-tm4c123gxl.cfg &`

6. Load the program ELF file onto the board:

    `arm-none-eabi-gdb bin/[program].elf`

7. If successful, the program will be loaded onto the board and be stopped at `main`. Type `c` then enter to continue the operation of the program.

## Module Descriptions

### PID Controller

A configurable, generic Proportional-Integral-Derivative controller which can be used to control external hardware such as a DC motor. Basic parameters such as Kp, Ki and Kd can be set, as well as setpoint weights (b, c) and a derivative filter coefficient (N). 

This controller implementation avoids division operations while running by pre-calculating equation coefficients.  For ease of use, the basic parameters can be set in the `src/ControllerParameters.h` file and, which will automatically calculate the correct equation coefficients. Then a controller can be instantiated using the code from `test/simulateMotor.c`

The controller takes two inputs, the setpoint reference and feedback value, and provides one output, the control signal. These inputs and outputs are memory locations so that the controller can read and write from registers or memory already in use by the main program.

### PWM Interface

A control interface for the two PWM modules on the TM4C123GH6PM microcontroller. Allows GPIO pins to be configured for PWM outputs with a variable frequency and duty cycle.

This interface abstracts away much of the hardware control which is required to use the PWM interface such as enabling peripherals and configuring GPIO pins, and allows PWM signals to be sent using a few function calls. 

All possible PWM output pins are referenced by pin number and this is all that is needed to activate a PWM pin, as opposed to dealing with multiple system peripherals and pin configurations, etc.

Please refer to the source code for detailed interface documentation.

### Quadrature Encoder Interface (QEI)

A control interface for the two QEI modules on the TM4C123GH6PM microcontroller. Allows GPIO pins to be used as inputs from a quadrature encoder to measure position and velocity.

Similarly to the PWM interface, this module abstracts much of the hardware control away, in favour of using a few, much simpler function calls to configure and use quadrature encoders.

Please refer to the source code for detailed interface documentation.

## Troubleshooting

### Installing ARM Embedded Toolchain (Ubuntu)

Steps slightly modified from [this](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa) StackExchange post.

1. Download the latest [source code](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
2. Extract into an installation folder, e.g. `/usr/local/opt/`
```
    mkdir -p /usr/local/opt/arm-none-eabi/
    tar xvf gcc-arm-none-eabi-xxx.tar.bz2 
    mv gcc-arm-none-eabi-xxx/* /usr/local/opt/arm-none-eabi
    rm -r gcc-arm-none-eabi-xxx
```

3. Create symbolic links for all toolchain binaries into a folder on your `PATH`, e.g. `/usr/local/bin/`
```
    for bin in /usr/local/opt/arm-none-eabi/bin/*; do sudo ln -s /usr/local/bin/$(basename $bin); done
```

4. Install and symlink external dependencies
```
    sudo apt install libncurses-dev
    sudo ln -s /usr/lib/x86_64-linux-gnu/libncurses.so.6 /usr/lib/x86_64-linux-gnu/libncurses.so.5
    sudo ln -s /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
```
### Running GDB With Correct Settings

The `.gdbinit` file in this project will set up GDB when it is run and make flashing and debugging more streamlined.

This file may need permission to be loaded which can be done by adding `add-auto-load-safe-path [path/to/project]/.gdbinit` to your `~/.gdbinit` file. This can be done with the command

`echo "add-auto-load-safe-path [path/to/project]/.gdbinit" >> ~/.gdbinit`
