// FileName:        gpio_2026.c                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 2.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Definición de funciones de configuración
//                  control GPIO para ESP32. Construcción de estructura 
//                  gpio_pin_t y configuración hardware.                                                                      
// Autor:           Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón 
// Updated:         03/06/2026

#include "gpio_2026.h"
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
    HWREG32(pin_reg_addr) &= ~(0x7 << 7); // Limpiar campo
    HWREG32(pin_reg_addr) |= ((gpio->int_type & 0x7) << 7); // Asignar tipo

    if(gpio->int_type != INT_DESHABILITADA) {
        // Si se configuró un tipo de interrupción, habilitarla con callback NULL
        // para que el usuario la configure explícitamente después.
        gpio_isr_service_init(); // Asegurar que el servicio de ISR esté inicializado
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
 * @brief ISR compartida del periférico GPIO. Dispatcher de callbacks.
 *
 * Manejador de interrupción invocado por el hardware cada vez que cualquier
 * pin GPIO con interrupción habilitada genera un evento. Dado que el ESP32
 * utiliza un único vector para todas las interrupciones GPIO, esta función
 * determina en tiempo de ejecución qué pines dispararon el evento,
 * limpia sus banderas y ejecuta el callback registrado en cada uno.
 *
 * Secuencia de ejecución:
 *      1. Lee GPIO_STATUS_REG  (pines 0-31)  y GPIO_STATUS1_REG (pines 32-39).
 *      2. Limpia las banderas en GPIO_STATUS_W1TC_REG / GPIO_STATUS1_W1TC_REG
 *         antes de procesar, para no perder flancos que ocurran durante
 *         la ejecución del dispatcher.
 *      3. Recorre el bitmask e invoca gpio->callback(gpio->cb_arg) por cada
 *         pin activo que tenga un callback registrado.
 *
 * @param[in]   arg     No utilizado. Requerido por la firma de esp_intr_alloc().
 *
 * @note    Declarada con IRAM_ATTR para garantizar su ejecución desde memoria
 *          interna, requerido cuando la caché de flash puede estar deshabilitada
 *          durante la atención de la interrupción.
 * @note    Los callbacks registrados por el usuario también deben declararse
 *          con IRAM_ATTR y respetar las restricciones de contexto ISR:
 *          sin printf(), malloc(), vTaskDelay() ni llamadas bloqueantes.
 * @note    Función de uso interno. No debe ser referenciada fuera de gpio_2026.c.
 */

static void IRAM_ATTR gpio_dispatcher_isr(void *arg)
{
    // Leer qué pines dispararon
    uint32_t status    = GPIO_STATUS_REG;
    uint32_t status1   = GPIO_STATUS1_REG;

    // Limpiar las banderas ANTES de procesar (evita perder flancos)
    GPIO_STATUS_W1TC_REG  = status;
    GPIO_STATUS1_W1TC_REG = status1;

    // Recorrer pines 0-31
    for (int i = 0; i < 32; i++) {
        if ((status >> i) & 1) {
            gpio_pin_t *g = &gpio_table[i];
            if (g->callback) {
                g->callback(g->cb_arg);
            }
        }
    }

    // Recorrer pines 32-39
    for (int i = 0; i < 8; i++) {
        if ((status1 >> i) & 1) {
            gpio_pin_t *g = &gpio_table[32 + i];
            if (g->callback) {
                g->callback(g->cb_arg);
            }
        }
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

    // Habilita la máscara de interrupción para este pin
    GPIO_INT_ENA_REG |= (1 << gpio->pin);
}

void gpio_disable_interrupt(gpio_pin_t *gpio)
{
    GPIO_INT_ENA_REG &= ~(1 << gpio->pin);
    gpio->callback = NULL;
    gpio->cb_arg   = NULL;
}