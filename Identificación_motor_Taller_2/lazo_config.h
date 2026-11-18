/*
 * lazo_config.h — Modelo de los motores y diseño del lazo de RPM
 * Generado por el notebook del Taller 2 para el grupo G2, a partir de parametros_motor_G2.json
 * RPM ≈ K_m·PWM − N0, una recta por sentido. K_m = K del Lab 1, N0 = B del Lab 1.
 */
#pragma once

// Rueda izquierda (L)
#define KM_L_ADELANTE   0.35204f
#define N0_L_ADELANTE   2.41048f
#define ZM_L_ADELANTE   10
#define KM_L_ATRAS      0.34570f
#define N0_L_ATRAS      2.36980f
#define ZM_L_ATRAS      15
#define TAU_L_S         0.02739f
#define RETARDO_L_S     0.00000f

// Rueda derecha (R)
#define KM_R_ADELANTE   0.37817f
#define N0_R_ADELANTE   2.28063f
#define ZM_R_ADELANTE   15
#define KM_R_ATRAS      0.38553f
#define N0_R_ATRAS      2.59838f
#define ZM_R_ATRAS      20
#define TAU_R_S         0.02501f
#define RETARDO_R_S     0.00000f

// Diseño del lazo (valores iniciales; se cambian en vivo con comandos)
#define LAMBDA_MS        50
#define CRITERIO_K        0     // 0 = por sentido, 1 = promediada
#define PREALIMENTACION   1
#define REF_TAU_MS        0     // modelo de referencia apagado

#define TS_LAZO_US       5000
#define VENTANA_MS         10
#define R_SUAVE_RPM      1.0f
#define I_MAX_PWM       255.0f
#define REF_MAX_RPM      60.0f

// Ensayo de escalón de consigna
#define LAZO_ANTES_MS     500
#define LAZO_DURANTE_MS  1000
#define LAZO_DESPUES_MS   700
