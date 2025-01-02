
#include "bshal_delay.h"
#include "bshal_gpio.h"
#include "bshal_i2cm.h"

#include "i2c.h"
#include "ina22x.h"

#include <stdint.h>

static bshal_i2cm_instance_t *mp_i2c;

void SystemClock_Config(void) {
#ifdef STM32F0
    extern void ClockSetup_HSE8_SYS48();
    ClockSetup_HSE8_SYS48();
#endif

#ifdef STM32F1
    extern void ClockSetup_HSE8_SYS72(void);
    ClockSetup_HSE8_SYS72();
#endif

#ifdef RV32F103
    extern void ClockSetup_HSE8_SYS72(void);
    ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F3
    extern void ClockSetup_HSE8_SYS72(void);
    ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F4
    extern void SystemClock_HSE25_SYS84(void);
    SystemClock_HSE25_SYS84();
#endif

#ifdef STM32L0
    extern void SystemClock_HSI16_SYS32(void);
    SystemClock_HSI16_SYS32();
#endif

#ifdef STM32L1
    extern void SystemClock_HSE8_SYS48(void);
    SystemClock_HSE8_SYS48();
#endif
}

void SysTick_Handler(void) {}

extern uint32_t SystemCoreClock;

int main(void) {
    SystemClock_Config();
    extern void SystemCoreClockUpdate(void);
    SystemCoreClockUpdate();
    mp_i2c = i2c_init();
    // 0x40, 0x41,  ina220
    // 0x42, ina3221
    // 0x45	ina226
    ina22x_t ina22x = {.p_i2c = mp_i2c, .addr = 0x45};
    int status = ina22x_init(&ina22x);
    while (true)
        ;
}
