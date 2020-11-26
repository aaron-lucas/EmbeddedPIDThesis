#include "common.h"
#include "driverlib/fpu.h"

void setSystemClock(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
}

void enableFPU(void) {
    FPUEnable();
    FPULazyStackingEnable();
}

enum Status enablePeripheral(uint32_t peripheral) {
    // Ensure the peripheral is valid for this device
    if (!SysCtlPeripheralPresent(peripheral))
        return STATUS_FAILURE;

    // Enable peripheral if currently disabled
    if (!SysCtlPeripheralReady(peripheral))
        SysCtlPeripheralEnable(peripheral);

    // Peripherals take 5 clock cycles to become ready after being enabled
    while (!SysCtlPeripheralReady(peripheral));

    return STATUS_SUCCESS;
}

#ifdef DEBUG

void __error__(char *pcFilename, uint32_t ui32Line) {

}

#endif
