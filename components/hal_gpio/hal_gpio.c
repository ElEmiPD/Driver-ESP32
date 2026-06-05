// FileName:        hal_gpio.c
// Dependencies:    hal_gpio.h, gpio_2026.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           Independiente de placa (portable)
// Program version: 3.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Implementación del HAL de GPIO.
//                  Todo acceso al hardware pasa por gpio_2026.
//                  Para portar a otro MCU, solo se reemplaza este .c;
//                  hal_gpio.h y las capas superiores no cambian.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#include "hal_gpio.h"

// ===========================================================================
//  Definición de la tabla interna
//  Una sola tabla para entradas y salidas; el campo is_output de gpio_pin_t
//  permite discriminar la dirección en tiempo de ejecución.
// ===========================================================================
gpio_pin_t *hal_gpio_table[HAL_GPIO_MAX_PINS] = {NULL};

// ===========================================================================
//  Helpers internos (no visibles fuera de este archivo)
//  static: visibilidad restringida a hal_gpio.c.
// ===========================================================================

/**
 * Valida el número de pin y que la tabla tenga un descriptor válido.
 * Retorna HAL_GPIO_ERR_PIN si el pin está fuera de rango.
 * Retorna HAL_GPIO_ERR_INIT si el pin no fue inicializado.
 * Retorna HAL_GPIO_OK si todo está bien.
 */
static hal_gpio_err_t validate(uint8_t pin)
{
    if (pin >= HAL_GPIO_MAX_PINS)
    {
        return HAL_GPIO_ERR_PIN;
    }

    if (hal_gpio_table[pin] == NULL)
    {
        return HAL_GPIO_ERR_INIT;
    }
    return HAL_GPIO_OK;
}


 //Valida pin inicializado Y que sea salida.
 
static hal_gpio_err_t validate_output(uint8_t pin)
{
    hal_gpio_err_t err = validate(pin);
    if (err != HAL_GPIO_OK)
    {
        return err;
    }                
    
    if (!hal_gpio_table[pin]->is_output)
    {
        return HAL_GPIO_ERR_DIR;
    }   
    return HAL_GPIO_OK;
}


 // Valida pin inicializado y que sea entrada.
 
static hal_gpio_err_t validate_input(uint8_t pin)
{
    hal_gpio_err_t err = validate(pin);
    if (err != HAL_GPIO_OK)
    {
        return err;
    }

    if (hal_gpio_table[pin]->is_output)
    {
        return HAL_GPIO_ERR_DIR;
    }   

    return HAL_GPIO_OK;
}

// ===========================================================================
//  Salidas digitales
// ===========================================================================

hal_gpio_err_t hal_gpio_output_init(uint8_t pin, gpio_logic_t logic)
{
    if (pin >= HAL_GPIO_MAX_PINS) 
    {
        return HAL_GPIO_ERR_PIN;
    }
    // gpio_init2026 valida que el pin exista en el hardware y retorna NULL
    // si es inválido (p. ej. GPIO24 o GPIO28-31 en ESP32).
    gpio_pin_t *p = gpio_init2026(pin, true, logic, GPIO_DEFAULT, INT_DESHABILITADA);

    if (p == NULL) 
    {
        return HAL_GPIO_ERR_PIN;
    }

    hal_gpio_table[pin] = p;
    return HAL_GPIO_OK;
}

hal_gpio_err_t hal_gpio_output_set(uint8_t pin, bool state)
{
    hal_gpio_err_t err = validate_output(pin);
    if (err != HAL_GPIO_OK)
    {
        return err;
    } 

    gpio_write(hal_gpio_table[pin], state);
    return HAL_GPIO_OK;
}

hal_gpio_err_t hal_gpio_output_toggle(uint8_t pin)
{
    hal_gpio_err_t err = validate_output(pin);
    if (err != HAL_GPIO_OK) 
    {
        return err;
    }

    // Leer estado físico actual y escribir el inverso lógico.
    // gpio_read ya aplica la lógica activa del pin, por lo que
    // el toggle es correcto independientemente de ACTIVE_HIGH/LOW.
    bool current = gpio_read(hal_gpio_table[pin]);
    gpio_write(hal_gpio_table[pin], !current);
    return HAL_GPIO_OK;
}

hal_gpio_err_t hal_gpio_output_get(uint8_t pin, bool *state)
{
    hal_gpio_err_t err = validate_output(pin);
    if (err != HAL_GPIO_OK) 
    {
        return err;
    }
    if (state == NULL)      
    {
        return HAL_GPIO_ERR_PIN;   // puntero nulo
    }

    *state = gpio_read(hal_gpio_table[pin]);
    return HAL_GPIO_OK;
}

// ===========================================================================
//  Entradas digitales
// ===========================================================================

hal_gpio_err_t hal_gpio_input_init(uint8_t pin, pull_mode_t pull, gpio_logic_t logic)
{
    if (pin >= HAL_GPIO_MAX_PINS) return HAL_GPIO_ERR_PIN;

    gpio_pin_t *p = gpio_init2026(pin, false, logic, pull, INT_DESHABILITADA);
    if (p == NULL) 
    {
        return HAL_GPIO_ERR_PIN;
    }

    hal_gpio_table[pin] = p;
    return HAL_GPIO_OK;
}

hal_gpio_err_t hal_gpio_input_read(uint8_t pin, bool *state)
{
    hal_gpio_err_t err = validate_input(pin);
    if (err != HAL_GPIO_OK) 
    {
        return err;
    }
    if (state == NULL)      
    {
        return HAL_GPIO_ERR_PIN;  
    }

    *state = gpio_read(hal_gpio_table[pin]);
    return HAL_GPIO_OK;
}

// ===========================================================================
//  Interrupciones
// ===========================================================================

hal_gpio_err_t hal_gpio_irq_attach(uint8_t pin, gpio_int_type_t int_type, gpio_isr_callback_t cb,void *arg)
{
    hal_gpio_err_t err = validate_input(pin);
    if (err != HAL_GPIO_OK) 
    {
        return err;
    }

    gpio_pin_t *p = hal_gpio_table[pin];

    // Guardar el tipo de interrupción y el callback en la estructura del HAL
    p->int_type = int_type;
    p->callback = cb;
    p->cb_arg   = arg;

    // Forzar al driver a escribir el 'int_type' en el registro físico del chip (GPIO_PINn_REG)
    // Esto además llamará a gpio_isr_service_init() internamente.
    gpio_config_in(p);

    // Habilitar el callback y la máscara en el driver de bajo nivel
    gpio_enable_interrupt(p, cb, arg);

    return HAL_GPIO_OK;
}

hal_gpio_err_t hal_gpio_irq_detach(uint8_t pin)
{
    hal_gpio_err_t err = validate_input(pin);
    if (err != HAL_GPIO_OK) 
    {
        return err;
    }

    gpio_disable_interrupt(hal_gpio_table[pin]);
    return HAL_GPIO_OK;
}

// ===========================================================================
//  Utilidades
// ===========================================================================

bool hal_gpio_is_init(uint8_t pin)
{
    if (pin >= HAL_GPIO_MAX_PINS) 
    {
        return false;
    }
    return (hal_gpio_table[pin] != NULL);
}

hal_gpio_err_t hal_gpio_deinit(uint8_t pin)
{
    if (pin >= HAL_GPIO_MAX_PINS) 
    {
        return HAL_GPIO_ERR_PIN;
    }

    // Si tenía interrupción activa, deshabilitarla antes de soltar el descriptor
    if (hal_gpio_table[pin] != NULL && !hal_gpio_table[pin]->is_output)
    {
        gpio_disable_interrupt(hal_gpio_table[pin]);
    }
        
    hal_gpio_table[pin] = NULL;
    return HAL_GPIO_OK;
} 