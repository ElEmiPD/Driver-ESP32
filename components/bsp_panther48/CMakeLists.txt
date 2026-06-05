idf_component_register(SRCS "bsp_panther48.c"
                    INCLUDE_DIRS "include"
                    REQUIRES hal_gpio
                    REQUIRES hal_timer
                    REQUIRES wdt_2026
                    REQUIRES hal_pwm)
