# Resultados del Laboratorio 1 · Identificación del motor DC

**Control Inteligente II (168418) · Ingeniería Mecatrónica · Universidad de Pamplona**
* **Grupo:** G2
* **Batería medida en los ensayos:** $10,50\text{ V}$

---

## 1. Tabla resumen de los motores

Parámetros obtenidos a partir de los ensayos de escalera y escalón en el archivo `parametros_motor_G2.json`:

| Parámetro | Unidades | Rueda Izquierda (L) | Rueda Derecha (R) |
|---|---|---|---|
| **Ganancia estática ($K$) adelante** | $\text{RPM/PWM}$ | $0,357$ | $0,381$ |
| **Corte estático ($B$) adelante** | $\text{RPM}$ | $2,51$ | $2,09$ |
| **Zona muerta adelante** | $\text{PWM}$ | $10$ | $15$ |
| **Ganancia estática ($K$) atrás** | $\text{RPM/PWM}$ | $0,344$ | — *(sin ensayo)* |
| **Corte estático ($B$) atrás** | $\text{RPM}$ | $2,16$ | — *(sin ensayo)* |
| **Zona muerta atrás** | $\text{PWM}$ | $14$ | — *(sin ensayo)* |
| **Ganancia estática ($K$) promedio** | $\text{RPM/PWM}$ | $0,351$ | $0,381$ |
| **Corte estático ($B$) promedio** | $\text{RPM}$ | $2,33$ | $2,09$ |
| **Zona muerta promedio** | $\text{PWM}$ | $12$ | $15$ |
| **Constante de tiempo ($\tau$)** | $\text{ms}$ | $28,2$ | $23,7$ |
| **Retardo puro ($L$)** | $\text{ms}$ | $0,0$ | $0,0$ |
| **Constante de tiempo de freno ($\tau_{freno}$)** | $\text{ms}$ | $16,8$ | $14,7$ |
| **Retardo de freno ($L_{freno}$)** | $\text{ms}$ | $2,9$ | $3,3$ |
| **$K$ incremental girando (adelante)** | $\text{RPM/PWM}$ | $0,375$ ($+4,9\text{ \%}$) | $0,384$ ($+0,7\text{ \%}$) |
| **$K$ incremental girando (atrás)** | $\text{RPM/PWM}$ | $0,372$ ($+8,2\text{ \%}$) | $0,389$ |
| **Cuentas por vuelta** | $\text{cuentas}$ | $3290,9$ | $3290,9$ |
| **RPM máxima estimada ($\text{PWM } 255$)** | $\text{RPM}$ | $88,6$ | $95,1$ |

---

## 2. Preguntas de análisis

**1. En un escalón desde parado ($0 \rightarrow 120$), la $K$ incremental sale menor que en un escalón con la rueda girando ($60 \rightarrow 120$). ¿Por qué? ¿Cuál de las dos describe mejor el motor para el lazo de RPM?**

* **Respuesta:** Desde parado sale menor ($0,344\text{ RPM/PWM}$ en L y $0,363\text{ RPM/PWM}$ en R) que con la rueda ya girando ($0,374\text{ RPM/PWM}$ en L y $0,381\text{ RPM/PWM}$ en R) porque al arrancar parte del PWM se gasta en vencer la fricción inicial (zona muerta). La que mejor describe al motor es la de la **rueda girando**, ya que el control de RPM casi siempre trabaja con el robot en movimiento.

**2. ¿Por qué la $\tau$ al llevar el PWM a 0 sale menor que al acelerar? ¿Qué pasaría si en lugar de PWM 0 se pusiera el driver en *standby* (`STBY` en bajo)?**

* **Respuesta:** Al poner PWM 0, $\tau$ baja bastante (de $28,2\text{ ms}$ a $16,8\text{ ms}$ en L, y de $23,7\text{ ms}$ a $14,7\text{ ms}$ en R) porque el driver pone en corto los cables del motor y su propia energía lo frena de golpe (freno en corto). Si se usara *standby*, el motor quedaría desconectado (rueda libre) y tardaría mucho más en detenerse solo por el roce mecánico.

**3. Comparen la zona muerta hacia adelante y hacia atrás, y la de las dos ruedas. Cuando el balancín está casi quieto, las ruedas cambian de sentido todo el tiempo. ¿Qué efecto tiene la zona muerta en ese caso?**

* **Respuesta:** La rueda L arranca con $\text{PWM } 10$ hacia adelante y $14$ hacia atrás; la rueda R arranca con $15$ hacia adelante (faltó el dato hacia atrás por un ensayo repetido en positivo). Como el balancín quieto cambia de sentido todo el tiempo pasando por cero, la zona muerta hace que los motores no respondan a órdenes pequeñas y el robot se quede tambaleando (oscilando).

**4. Con su $\tau$ y la resolución de su encoder, ¿qué limita más la rapidez del lazo de RPM, el motor o la medición de la RPM? Calculen cuántas RPM vale una cuenta en una ventana de $10\text{ ms}$.**

* **Respuesta:** Con $3290,9\text{ cuentas por vuelta}$, $1\text{ cuenta}$ en $10\text{ ms}$ ($0,01\text{ s}$) equivale a $1,82\text{ RPM}$:

$$\Delta RPM = \frac{60}{3290,9 \times 0,01} = 1,82\text{ RPM}$$

Lo que más limita la rapidez del lazo es **el motor**, porque tarda entre $23,7\text{ ms}$ y $28,2\text{ ms}$ en reaccionar, mucho más que los $10\text{ ms}$ que toma medir la velocidad.

**5. Si la batería baja de $8,4\text{ V}$ a $7,4\text{ V}$, ¿cuánto cambia $K$ según la sección 4.2? ¿Qué parte del lazo de RPM tiene que corregir eso?**

* **Respuesta:** Si baja de $8,4\text{ V}$ a $7,4\text{ V}$, la ganancia $K$ cae un $12\text{ \%}$ (aplicado a nuestros motores bajaría de $0,357$ a $\approx 0,315\text{ RPM/PWM}$ en L y de $0,381$ a $\approx 0,336\text{ RPM/PWM}$ en R). Esa pérdida de fuerza por descarga de la batería la corrige automáticamente el **integrador (la parte I)** del controlador PI.

**6. Con la RPM máxima de su motor (PWM 255), calculen la velocidad máxima del robot en $\text{m/s}$. El lazo de equilibrio necesita reserva de velocidad para recuperarse cuando el robot se inclina. ¿Qué velocidad de avance les parece razonable para la pista, y por qué?**

* **Respuesta:** A máximo PWM ($255$), el motor alcanza $88,6\text{ RPM}$ en L y $95,1\text{ RPM}$ en R. Con ruedas de $6,5\text{ cm}$ ($0,065\text{ m}$) de diámetro y tomando la rueda más lenta (L), la velocidad máxima es de $0,30\text{ m/s}$:

$$v_{max} = \frac{88,6}{60} \times \pi \times 0,065\text{ m} = 0,30\text{ m/s}$$

Para la pista es razonable avanzar a $0,10\text{ o }0,15\text{ m/s}$ (la mitad o menos de su capacidad), porque así se deja suficiente potencia libre de reserva para que el robot pueda acelerar de emergencia y recuperar el equilibrio si se inclina sin saturar el PWM.
