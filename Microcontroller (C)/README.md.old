Generic PID
===========

An implementation of a PID controller for a generic microcontroller in C.

## Features

This implementation is designed for use in embedded systems and as such has a constant time and memory complexity, and does not use any of the following:

* Dynamic memory allocation (i.e. _malloc()_)
* Standard library functions
* Floating point numbers/arithmetic
* Division operations

This implementation is generic and can be used on most microcontrollers.

## Fixed Point Arithmetic

As floating point types are not used in this implementation, a fixed point number representation is used instead. The `fix_t.h` and `fix_t.c` files implement a flexible fixed point number type, `fix_t`, along with addition, subtraction and multiplication operations. The data type of the fixed point number as well as the number of bits available for the fractional part can be customised by changing the `WORD_SIZE` and `Q_POINT` definitions in `fix_t.h`, along with the `CONVERSION_FACTOR` constant.

The tests for the `fix_t` type are designed for a 32-bit word size and 16-bit fractional part.

#### Definitions

Four custom data types are defined. Their names and purposes are summarised below:

* `word_t`: An unsigned type which is used for modifying the bits of a fixed point number.
* `fix_t`: A signed type which represents the fixed point number and is used for operations which require interpretation of positive and negative.
* `dword_t`: An unsigned type which is double the length of `word_t` and is used for holding the result of a multiplication.
* `dint_t`: A signed type which is double the length of `fix_t` and is used for type conversion in multiplication.



A macro, `FIX_POINT()` is also defined which allows the conversion of integer and decimal numbers to their fixed-point representation, with **rounding** occurring instead of **truncating**. This macro relies on the `CONVERSION_FACTOR` constant being set to `2^Q_POINT` and being a floating point type (by including a '.0' at the end of the integer).



Addition, subtraction and multiplication functions are implemented which indicate if an overflow has occurred. These functions are `fixAdd()`, `fixSubtract()` and `fixMultiply()` respectively.

Multiplication is performed by using a double-length data type and then truncating the result.

## PID Controller & Algorithm

The PID controller is contained in a structure, `struct pidController`, which contains the controller parameters, input and output registers and other internal variables. All values in the controller are fixed point and the aforementioned fixed point operations are used. 

This structure is generic and can be used with a wide variety of microcontrollers as the inputs and outputs of the controller are pointers which can be set to specific memory location such as the input for a DAC.

Error checking is performed after each operation as the fixed point number representation has a greatly reduced range compared to standard integer types and the risk of overflow is much higher. When overflow occurs, the control algorithm exits early, allowing the main program to handle the error, which currently resets the controller.

There are parameters for controller output limits which will cause the final output to be saturated if it goes above (or below) these bounds.

The following image shows a block diagram of a basic continuous PID controller:

![PID](images/PIDBlockDiagram.png)

The implemented PID controller has the same basic structure but is modified to utilise set point weighting and derivative filtering for improved performance.

## Main Control Loop

The main program which runs on the microcontroller is responsible not only for calculating the control output of the PID controller, but also handling errors, reading inputs from sensors and setting analogue voltage outputs.

A timer will run an iteration of the control algorithm periodically, which involved updating the setpoint (if necessary) and obtaining feedback from the controlled system before running the PID calculation.

The setpoint will likely be fed in from an external process such as a higher level controller, and the feedback signal will be read through an analogue-to-digital converter (ADC) on the microcontroller (if it has one).

Once the control output is calculated, a digital-to-analogue converter must be run to convert it into a voltage which is used to control the system.

These three processes all have respective functions in the main program allowing them to be customised to the choice of microcontroller.

The timer will trigger an interrupt which sets actions flags for the main control loop. This is to make the interrupt service routine as short as possible and also allows for other actions to be added in the future by adding more action flags. Currently only two flags exist, a reset flag which resets the controller in the event of an overflow or other error, and a controller iteration flag which runs another iteration of the PID controller.

The following image shows a flow diagram of the main program:

![Main program flow diagram](images/FlowDiagram.png)
