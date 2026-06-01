#include "timer_2026.h"


void timer_init(void)
{
    /*Deshabilitar el timer antes de configurarlo para evitar comportamientos erráticos
      Usamos un AND bit a bit con el complemento de la máscara para apagar solo ese bit*/
    TIMG0_T0CONFIG_REG &= ~TIMER_EN_BIT;

    // Configurar el prescaler y establecer el modo de conteo ascendente
    // Desplazamos el valor 80 a su posición correcta en el registro de configuración
    TIMG0_T0CONFIG_REG = (TIMER_PRESCALER << TIMER_DIVIDER_SHIFT) 
                       | TIMER_INCREASE_BIT;

    // Preparar el reinicio del contador a 0
    TIMG0_T0LOADLO_REG = 0; // Se escribe 0 en la parte baja (32 bits)
    TIMG0_T0LOADHI_REG = 0; // Se escribe 0 en la parte alta (32 bits)
    
    // Aplicar un valor para disparar la carga
    // Esto obliga al hardware a tomar los ceros de los registros LOAD y aplicarlos al contador real
    TIMG0_T0LOAD_REG   = 1;

    // Habilitar el timer nuevamente para que comience a contar
    TIMG0_T0CONFIG_REG |= TIMER_EN_BIT;
}

uint64_t timer_get_us(void)
{
    // Enviar el trigger de actualización
    // Esto obliga al hardware a congelar y copiar el valor actual del contador 
    // hacia los registros de lectura, evitando que cambien mientras los leemos
    TIMG0_T0UPDATE_REG = 1;
    
    // Leer las dos mitades de 32 bits de los registros
    uint64_t lo = TIMG0_T0LO_REG;
    uint64_t hi = TIMG0_T0HI_REG;
    
    // Combinar ambas mitades en una sola variable de 64 bits.
    // Se desplaza la parte alta 32 bits a la izquierda y se hace un OR con la parte baja
    return (hi << 32) | lo;
}

void timer_delay_us(uint32_t us)
{
    // Guardamos la marca de tiempo de inicio.
    uint64_t inicio = timer_get_us();
    
    // El procesador se queda atascado en este while
    // evaluando constantemente hasta que la diferencia de tiempo alcance el valor solicitado
    while ((timer_get_us() - inicio) < us);
}

void timer_delay_ms(uint32_t ms)
{
    // Reutilizamos la función de microsegundos, simplemente multiplicando por 1000
    timer_delay_us(ms * 1000);
}