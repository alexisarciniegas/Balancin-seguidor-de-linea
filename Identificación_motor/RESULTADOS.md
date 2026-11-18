# Resultados Laboratorio 1 · Identificación del motor DC

**Curso:** Control Inteligente II (168418) · Ingeniería Mecatrónica  
**Grupo:** G2  
**Fecha:** 17 de noviembre de 2026  
**Voltaje de batería en los ensayos:** $10,32\text{ V}$

---

## 1. Montaje experimental

El robot se ubicó sobre el soporte con ambas ruedas libres en el aire para los ensayos de escalera y escalón, verificando previamente la dirección de giro y el conteo de los encoders (motores JGA25-370 con encoders magnéticos).

---

## 2. Tabla resumen de parámetros por rueda

A continuación se presentan los parámetros estáticos y dinámicos obtenidos a partir del archivo `G2.log` y el ajuste por mínimos cuadrados en el notebook (`parametros_motor_G2.json`):

| Parámetro | Símbolo / Unidad | Rueda Izquierda (L) | Rueda Derecha (R) |
|---|---|---|---|
| **Cuentas por vuelta** | $\text{cuentas/vuelta}$ | $3290,0$ | $3290,0$ |
| **Ganancia estática (adelante)** | $K_{adelante}\text{ [RPM/PWM]}$ | $0,3520$ | $0,3782$ |
| **Corte estático (adelante)** | $B_{adelante}\text{ [RPM]}$ | $2,4105$ | $2,2806$ |
| **Zona muerta (adelante)** | $\text{PWM}_{min, ad}\text{ [PWM]}$ | $10$ | $15$ |
| **Ganancia estática (atrás)** | $K_{atras}\text{ [RPM/PWM]}$ | $0,3457$ | $0,3855$ |
| **Corte estático (atrás)** | $B_{atras}\text{ [RPM]}$ | $2,3698$ | $2,5984$ |
| **Zona muerta (atrás)** | $\text{PWM}_{min, at}\text{ [PWM]}$ | $15$ | $20$ |
| **Ganancia promedio** | $K_{prom}\text{ [RPM/PWM]}$ | $0,3489$ | $0,3819$ |
| **Corte promedio** | $B_{prom}\text{ [RPM]}$ | $2,3901$ | $2,4395$ |
| **Zona muerta promedio** | $\text{PWM}_{min, prom}\text{ [PWM]}$ | $12$ | $18$ |
| **Ganancia incremental girando (adelante)** | $K_{inc, ad}\text{ [RPM/PWM]}$ | $0,3608$ | $0,3773$ |
| **Ganancia incremental girando (atrás)** | $K_{inc, at}\text{ [RPM/PWM]}$ | $0,3647$ | $0,3865$ |
| **Constante de tiempo mecánica** | $\tau\text{ [ms]}$ | $27,39$ | $25,01$ |
| **Retardo puro** | $L\text{ [ms]}$ | $0,00$ | $0,00$ |
| **Constante de tiempo de freno (PWM 0)** | $\tau_{freno}\text{ [ms]}$ | $17,28$ | $14,03$ |
| **Retardo en freno** | $L_{freno}\text{ [ms]}$ | $2,83$ | $3,50$ |

---

## 3. Preguntas de análisis

1. **En un escalón desde parado (0 → 120), la K incremental sale menor que en un escalón con la rueda girando (60 → 120). ¿Por qué? ¿Cuál de las dos describe mejor el motor para el lazo de RPM?**
   Desde parado, $K$ da $0,341\text{ RPM/PWM}$ (rueda L) y $0,359\text{ RPM/PWM}$ (rueda R), pero con la rueda ya girando sube a $0,361\text{ RPM/PWM}$ (L) y $0,375\text{ RPM/PWM}$ (R). Sale menor desde cero porque parte del PWM se gasta en vencer la fricción seca y la zona muerta (el desplazamiento $B \approx 2,4\text{ RPM}$). Para el lazo de RPM sirve más la $K$ con la rueda girando, porque el motor casi siempre trabaja en movimiento y ese valor coincide con la pendiente real de la recta ($0,352$ en L y $0,378$ en R).

2. **¿Por qué la $\tau$ al llevar el PWM a 0 sale menor que al acelerar? ¿Qué pasaría si en lugar de PWM 0 se pusiera el driver en *standby* (STBY en bajo)?**
   Al acelerar, $\tau$ es $27,4\text{ ms}$ (L) y $25,0\text{ ms}$ (R), mientras que al frenar a PWM 0 baja a $17,3\text{ ms}$ (L) y $14,0\text{ ms}$ (R). Sale menor porque en PWM 0 el driver TB6612 pone en corto las terminales del motor y la fuerza contraelectromotriz hace un frenado eléctrico. Si se pusiera `STBY` en bajo, el puente H quedaría abierto y la rueda frenaría solo por fricción mecánica, tardando mucho más en detenerse ($\tau$ más grande).

3. **Comparen la zona muerta hacia adelante y hacia atrás, y la de las dos ruedas. Cuando el balancín está casi quieto, las ruedas cambian de sentido todo el tiempo. ¿Qué efecto tiene la zona muerta en ese caso?**
   La rueda L arranca con PWM 10 hacia adelante y 15 hacia atrás; la rueda R necesita más esfuerzo, arrancando en 15 adelante y 20 atrás. Como ambas piden 5 puntos más de PWM en reversa y la R es más dura que la L, la respuesta es asimétrica. Cuando el balancín intenta equilibrarse en el sitio e invierte el giro todo el tiempo, el motor no responde dentro de ese rango de PWM ($-15$ a $+10$ en L y $-20$ a $+15$ en R), metiendo un retardo que hace que el robot oscile o cabecee sin quedarse quieto del todo.

4. **Con su $\tau$ y la resolución de su encoder, ¿qué limita más la rapidez del lazo de RPM, el motor o la medición de la RPM? Calculen cuántas RPM vale una cuenta en una ventana de 10 ms.**
   Con $3290\text{ cuentas/vuelta}$, una cuenta en una ventana de $10\text{ ms}$ ($0,01\text{ s}$) equivale a:
   $$\text{RPM} = \frac{60}{3290 \times 0,01\text{ s}} = 1,82\text{ RPM}$$
   Lo que más limita la rapidez del lazo es el motor, porque su constante mecánica $\tau$ ($25,0$ a $27,4\text{ ms}$) es casi tres veces más lenta que los $10\text{ ms}$ que toma medir con buena resolución.

5. **Si la batería baja de 8,4 V a 7,4 V, ¿cuánto cambia K según la sección 4.2? ¿Qué parte del lazo de RPM tiene que corregir eso?**
   Según la relación $V \approx \frac{\text{PWM}}{255}V_{bat}$, si la batería cae de $8,4\text{ V}$ a $7,4\text{ V}$, el voltaje y la ganancia $K$ bajan un $11,9\text{ \%}$ ($\frac{8,4 - 7,4}{8,4}$). En nuestros motores (medidos a $10,32\text{ V}$ con $K_L = 0,349$ y $K_R = 0,382\text{ RPM/PWM}$), pasar de $8,4\text{ V}$ a $7,4\text{ V}$ haría que $K_L$ baje de $0,284$ a $0,250\text{ RPM/PWM}$ y $K_R$ de $0,311$ a $0,274\text{ RPM/PWM}$. Esa caída de ganancia la corrige la parte integral (el integrador) del controlador de RPM, que sube el PWM automáticamente hasta alcanzar las RPM pedidas.

6. **Con la RPM máxima de su motor (PWM 255), calculen la velocidad máxima del robot en m/s. El lazo de equilibrio necesita reserva de velocidad para recuperarse cuando el robot se inclina. ¿Qué velocidad de avance les parece razonable para la pista, y por qué?**
   Con la recta $RPM = K \cdot 255 - B$, la velocidad máxima hacia adelante es $87,4\text{ RPM}$ en L y $94,2\text{ RPM}$ en R (promedio de $90,8\text{ RPM}$). Usando ruedas de $65\text{ mm}$ de diámetro ($r = 0,0325\text{ m}$), la velocidad lineal máxima es:
   $$v_{max} = \frac{90,8}{60} \times 2\pi(0,0325\text{ m}) \approx 0,31\text{ m/s}\quad (0,30\text{ m/s en L y } 0,32\text{ m/s en R})$$
   Una velocidad de avance razonable para la pista es de $0,10$ a $0,15\text{ m/s}$ (unas $30$ a $45\text{ RPM}$). Así solo usamos cerca de la mitad del PWM y dejamos más del $50\text{ \%}$ de reserva de velocidad para que el lazo de ángulo pueda acelerar fuerte y recuperar el equilibrio si el robot se inclina.