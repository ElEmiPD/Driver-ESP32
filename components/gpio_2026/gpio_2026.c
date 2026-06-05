// FileName:        gpio_2026.c                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 3.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Definición de funciones de configuración
//                  control GPIO para ESP32. Construcción de estructura 
//                  gpio_pin_t y configuración hardware.                                                                      
// Autor:           Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón 
// Updated:         05/06/2026

#include "gpio_2026.h"

// Handle global para la interrupción GPIO, compartida por todos los pines configurados
static intr_handle_t gpio_isr_handle = NULL;

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

gpio_pin_t* gpio_init2026   (uint8_t pin, 
                            bool is_output, 
                            gpio_logic_t logic, 
                            pull_mode_t pull_mode, 
                            gpio_int_type_t int_type)
{

    // Verificar rango
    if(pin > 39){
        puts("Error: Pin GPIO fuera de rango (0-39)");
        return NULL;
    }

    // Verificar GPIO inexistentes
    if(GPIO_MUX_REGS[pin] == 0){
        puts("Error: GPIO no existe");
        return NULL;
    }

    // GPIO34-39 son solo entrada
    if((is_output || pull_mode != GPIO_FLOATING) && pin >= 34){
        puts("Error: GPIO34-39 son solo entrada y no tienen resistencias de pull-up/pull-down");
        return NULL;
    }

    gpio_pin_t *gpio = &gpio_table[pin];
    
    *gpio = (gpio_pin_t){
        .pin = pin,
        .is_output = is_output,
        .logic = logic,
        .pull_mode = pull_mode,
        .int_type = int_type
    };

    // Configuración hardware

    if(gpio->is_output)
        gpio_config_out(gpio);
    else
        gpio_config_in(gpio);

    return gpio;
}

// Configuración de pin como entrada GPIO

void gpio_config_in(gpio_pin_t *gpio)
{
    uint32_t reg;

    reg = GPIO_MUX_REGS[gpio->pin];

    // Deshabilitar salida
    GPIO_ENABLE &= ~(1 << gpio->pin);

    // MCU_SEL = GPIO
    HWREG32(reg) &= ~MUX_MCU_SEL_MASK;
    HWREG32(reg) |=  MUX_MCU_SEL_GPIO;

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

    // Configurar tipo de interrupción en GPIO_PINn_REG
    // INT_TYPE ocupa bits [9:7] del registro GPIO_PINn
    uint32_t pin_reg_addr = GPIO_PIN_REG_BASE + (gpio->pin * 4);
    HWREG32(pin_reg_addr) &= ~(0x7 << 7);
    HWREG32(pin_reg_addr) |= ((gpio->int_type & 0x7) << 7);

    if(gpio->int_type != INT_DESHABILITADA) {
        // Bits 13 y 15 — PRO_CPU y APP_CPU
        HWREG32(pin_reg_addr) |= (1u << 13) | (1u << 15);
    } else {
        HWREG32(pin_reg_addr) &= ~((1u << 13) | (1u << 15));
    }

    // Siempre limpiar en config, habilitar solo en IT_ATTACH
    HWREG32(pin_reg_addr) &= ~(0x1F << 13);
}

// Configuración de pin como salida GPIO

void gpio_config_out(gpio_pin_t *gpio)
{
    uint32_t reg;

    reg = GPIO_MUX_REGS[gpio->pin];

    // INPUT DISABLE
    HWREG32(reg) &= ~FUN_IE;

    // MCU_SEL = GPIO
    HWREG32(reg) &= ~MUX_MCU_SEL_MASK;
    HWREG32(reg) |=  MUX_MCU_SEL_GPIO;

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

// ------------------------------ Interrupciones --------------------------------

/**
 * @brief Manejador global de interrupciones GPIO.
 *
 * Función de servicio de interrupción (ISR) encargada de atender todas las
 * las interrupciones generadas por los pines GPIO configurados mediante
 * gpio_init2026(). Debido a que el periférico GPIO comparte un único vector
 * de interrupción, esta rutina actúa como un dispatcher que identifica los
 * pines que generaron el evento y ejecuta el callback asociado a cada uno.
 *
 * Funcionamiento:
 *      1. Lee los registros de estado de interrupción GPIO.
 *      2. Identifica los pines que generaron el evento.
 *      3. Limpia las banderas de interrupción correspondientes.
 *      4. Verifica si el pin tiene un callback registrado.
 *      5. Ejecuta el callback y le pasa el argumento almacenado en cb_arg.
 *
 * Cada pin GPIO puede registrar de manera independiente una función callback
 * mediante los campos callback y cb_arg de la estructura gpio_pin_t,
 * permitiendo desacoplar la lógica de aplicación del controlador GPIO.
 *
 * Ejemplo:
 *
 *      void button_callback(void *arg)
 *      {
 *          // Código de usuario
 *      }
 *
 *      gpio_table[0].callback = button_callback;
 *      gpio_table[0].cb_arg   = NULL;
 *
 * Cuando se produzca una interrupción en GPIO0, esta ISR ejecutará:
 *
 *      button_callback(NULL);
 *
 * @param[in] arg
 *      Argumento reservado por el sistema de interrupciones.
 *      No es utilizado por el dispatcher.
 *
 * @note
 *      Esta función es de uso interno del driver y no debe ser invocada
 *      directamente por la aplicación.
 *
 * @note
 *      Los callbacks ejecutados desde esta ISR deben ser breves y cumplir
 *      las restricciones propias del contexto de interrupción. Se recomienda
 *      evitar operaciones bloqueantes, asignación dinámica de memoria y
 *      funciones de depuración como printf().
 */
static void IRAM_ATTR gpio_dispatcher_isr(void *arg)
{
    uint32_t status  = GPIO_STATUS_REG;
    uint32_t status1 = GPIO_STATUS1_REG;

    GPIO_STATUS_W1TC_REG  = status;
    GPIO_STATUS1_W1TC_REG = status1;

    while (status) {
        int i = __builtin_ctz(status);
        if (gpio_table[i].callback)
            gpio_table[i].callback(gpio_table[i].cb_arg);
        status &= status - 1; // limpiar bit procesado
    }

    while (status1) {
        int i = __builtin_ctz(status1);
        if (gpio_table[32 + i].callback)
            gpio_table[32 + i].callback(gpio_table[32 + i].cb_arg);
        status1 &= status1 - 1;
    }
}

void gpio_isr_service_init(void)
{
    if (gpio_isr_handle != NULL) return; // ya inicializado

    esp_intr_alloc(
        ETS_GPIO_INTR_SOURCE,
        ESP_INTR_FLAG_IRAM,         
        gpio_dispatcher_isr,
        NULL,
        &gpio_isr_handle
    );
}

void gpio_enable_interrupt(gpio_pin_t *gpio, gpio_isr_callback_t cb, void *arg)
{
    gpio->callback = cb;
    gpio->cb_arg   = arg;

    uint32_t pin_reg_addr = GPIO_PIN_REG_BASE + (gpio->pin * 4);
    HWREG32(pin_reg_addr) &= ~(0x1F << 13);          // limpiar campo INT_ENA
    HWREG32(pin_reg_addr) |= (1u << 13) | (1u << 15); // PRO_CPU + APP_CPU
}

void gpio_disable_interrupt(gpio_pin_t *gpio)
{
    uint32_t pin_reg_addr = GPIO_PIN_REG_BASE + (gpio->pin * 4);
    HWREG32(pin_reg_addr) &= ~(0x1F << 13); // limpiar INT_ENA

    gpio->callback = NULL;
    gpio->cb_arg   = NULL;
}