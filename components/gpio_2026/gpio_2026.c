// FileName:        gpio_2026.c                                                                                            
// Dependencies:    None                                                                                              
// Processor:       ESP32                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 1.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Definición de funciones de configuración
//                  control GPIO para ESP32. Construcción de estructura 
//                  gpio_pin_t y configuración hardware.                                                                      
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         31/05/2026

#include "gpio_2026.h"

// IOMUX REGISTERS (GPIO0-39)
// Tabla de direcciones de los registros de configuración de pines
const uint32_t GPIO_MUX_REGS[] = {
    0x3FF49044, 0x3FF49088, 0x3FF49040, 0x3FF49084,     // 0-3
    0x3FF49048, 0x3FF4906C, 0x3FF49060, 0x3FF49064,     // 4-7
    0x3FF49068, 0x3FF49054, 0x3FF49058, 0x3FF4905C,     // 8-11
    0x3FF49034, 0x3FF49038, 0x3FF49030, 0x3FF4903C,     // 12-15
    0x3FF4904C, 0x3FF49050, 0x3FF49070, 0x3FF49074,     // 16-19
    0x3FF49078, 0x3FF4907C, 0x3FF49080, 0x3FF4908C,     // 20-23
    0x00000000, 0x3FF49024, 0x3FF49028, 0x3FF4902C,     // 24-27
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 28-31
    0x3FF4901C, 0x3FF49020, 0x3FF49014, 0x3FF49018,     // 32-35
    0x3FF49004, 0x3FF49008, 0x3FF4900C, 0x3FF49010      // 36-39
};

gpio_pin_t gpio_table[40];


// Construcción de estructura gpio_pin_t y configuración hardware

gpio_pin_t gpio_init2026(uint8_t pin, bool is_output, gpio_logic_t logic, pull_mode_t pull_mode)
{
    gpio_pin_t gpio = {
        .pin = pin,
        .is_output = is_output,
        .logic = logic,
        .pull_mode = pull_mode
    };

    gpio_table[pin] = gpio;

    // Configuración hardware

    if(gpio.is_output)
        gpio_config_out(&gpio);
    else
        gpio_config_in(&gpio);

    return gpio;
}

// Configuración de pin como entrada GPIO

void gpio_config_in(gpio_pin_t *gpio)
{
    uint32_t reg;

    reg = GPIO_MUX_REGS[gpio->pin];

    // Deshabilitar salida
    GPIO_ENABLE &= ~(1 << gpio->pin);

    // INPUT ENABLE
    HWREG32(reg) |= FUN_IE; 
    
    // Configurar resistencia de pull-up o pull-down
    switch(gpio->pull_mode)
    {
        case GPIO_PULLUP:
            HWREG32(reg) |= PULL_WPU;
            HWREG32(reg) &= ~PULL_WPD;
            break;

        case GPIO_PULLDOWN:
            HWREG32(reg) |= PULL_WPD;
            HWREG32(reg) &= ~PULL_WPU;
            break;

        default:
            HWREG32(reg) &= ~PULL_WPU;
            HWREG32(reg) &= ~PULL_WPD;
            break;
    }
}

// Configuración de pin como salida GPIO

void gpio_config_out(gpio_pin_t *gpio)
{
    uint32_t reg;

    reg = GPIO_MUX_REGS[gpio->pin];

    // INPUT DISABLE
    HWREG32(reg) &= ~FUN_IE;

    // MCU_SEL = GPIO
    HWREG32(reg) &= ~(0x7 << 12); // MCU_SEL bits [14:12] = 000 para GPIO
    HWREG32(reg) |=  (0x2 << 12); // 

    // Habilitar salida
    GPIO_ENABLE |= (1 << gpio->pin);

    // Estado inicial: OFF lógico
    gpio_write(gpio, false);
}

// gpio_read(gpio)

bool gpio_read(gpio_pin_t *gpio)
{
    bool state;

    state = (GPIO_IN & (1 << gpio->pin)) ? true : false;

    if(gpio->logic == GPIO_ACTIVE_LOW)
        state = !state;

    return state;
}

// gpio_write(gpio, value) 

void gpio_write(gpio_pin_t *gpio, bool value)
{
    if(gpio->logic == GPIO_ACTIVE_LOW)
        value = !value;
    
    if(value)
        GPIO_OUT_W1TS = (1 << gpio->pin);
    else
        GPIO_OUT_W1TC = (1 << gpio->pin);
}