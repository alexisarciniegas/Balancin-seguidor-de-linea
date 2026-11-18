# Taller 2: Control de Velocidad y Diseño en Estado — Grupo G2

## 1. Cuatro ganancias de estado en unidades del firmware

Para la geometría configurada del balancín ($m_w = 0,052\text{ kg}$, $r = 0,0335\text{ m}$, $M_b = 0,611\text{ kg}$, $l = 0,031\text{ m}$, $I_b = 1,066 \times 10^{-3}\text{ kg}\cdot\text{m}^2$) y el juego de polos elegido $[-6,0; -6,5; -1,5; -2,0]\text{ rad/s}$, las ganancias obtenidas en el SI y convertidas a las unidades del firmware (usando el factor de conversión $\frac{60}{2\pi r} \approx 285,07\text{ RPM/(m/s)}$) son:

| Ganancia | Valor en SI | Valor en unidades del firmware | Unidad en el firmware |
| :--- | :---: | :---: | :--- |
| **$k_\theta$** (ángulo) | $25,99\text{ (m/s}^2\text{)/rad}$ | **$129,30$** | $\text{(RPM/s)/}^\circ$ |
| **$k_\omega$** (vel. angular) | $4,52\text{ (m/s}^2\text{)/(rad/s)}$ | **$22,49$** | $\text{(RPM/s)/(}^\circ\text{/s)}$ |
| **$k_v$** (avance) | $2,52\text{ s}^{-1}$ | **$718,90$** | $\text{(RPM/s)/(m/s)}$ |
| **$k_x$** (posición) | $2,16\text{ s}^{-2}$ | **$616,50$** | $\text{(RPM/s)/m}$ |

### Bloque listo para el firmware (`balancin_config.h`)

```cpp
// Ganancias de realimentación de estados - Grupo G2
// Polos de diseño: [-6.0, -6.5, -1.5, -2.0] rad/s
#define K_THETA  129.30f   // (RPM/s) / deg
#define K_OMEGA   22.49f   // (RPM/s) / (deg/s)
#define K_V      718.90f   // (RPM/s) / (m/s)
#define K_X      616.50f   // (RPM/s) / m
```

---

## 2. Resumen comparativo de los ensayos del lazo de RPM (`lazo_vel`)

| Ensayo | Rueda L: $t_{\text{subida}}$ | Rueda L: Sobrepico | Rueda R: $t_{\text{subida}}$ | Rueda R: Sobrepico | Error est. (L / R) |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Simulación Parte A ($10 \to 30\text{ RPM}$)** | $20\text{ ms}$ | $25\text{ \%}$ | $20\text{ ms}$ | $23\text{ \%}$ | $0,00 / 0,00\text{ RPM}$ |
| **B1. Base ($10 \to 30\text{ RPM}$, $\lambda=46\text{ ms}$)** | $15\text{ ms}$ | $32\text{ \%}$ | $15\text{ ms}$ | $32\text{ \%}$ | $+0,02 / +0,02\text{ RPM}$ |
| **B2. Escalón grande ($10 \to 50\text{ RPM}$)** | $25\text{ ms}$ | $27\text{ \%}$ | $20\text{ ms}$ | $29\text{ \%}$ | $-0,04 / -0,01\text{ RPM}$ |
| **B3. Escalón atrás ($-10 \to -30\text{ RPM}$)** | $15\text{ ms}$ | $32\text{ \%}$ | $15\text{ ms}$ | $32\text{ \%}$ | $+0,02 / +0,01\text{ RPM}$ |
| **B4. Cruce por cero ($10 \to -10\text{ RPM}$)** | $10\text{ ms}$ | $23\text{ \%}$ | $10\text{ ms}$ | $35\text{ \%}$ | $+0,02 / +0,01\text{ RPM}$ |
| **B5a. Promediado (`k p`, $10 \to -10\text{ RPM}$)** | $10\text{ ms}$ | $23\text{ \%}$ | $10\text{ ms}$ | $37\text{ \%}$ | $+0,01 / +0,00\text{ RPM}$ |
| **B5b. Promediado (`k p`, $-10 \to -30\text{ RPM}$)** | $20\text{ ms}$ | $32\text{ \%}$ | $15\text{ ms}$ | $31\text{ \%}$ | $-0,02 / +0,01\text{ RPM}$ |
| **B6. $\lambda = 30\text{ ms}$ ($10 \to 30\text{ RPM}$)** | $15\text{ ms}$ | $30\text{ \%}$ | $10\text{ ms}$ | $37\text{ \%}$ | $+0,02 / +0,01\text{ RPM}$ |
| **B7. Modelo ref. (`ref_tau = 26 ms`)** | $35\text{ ms}$ | $10\text{ \%}$ | $25\text{ ms}$ | $10\text{ \%}$ | $+0,01 / +0,03\text{ RPM}$ |
| **B8. Sin prealimentación (`ff = 0`)** | $95\text{ ms}$ | $5\text{ \%}$ | $90\text{ ms}$ | $5\text{ \%}$ | $-0,23 / -0,18\text{ RPM}$ |

---

## 3. Respuestas a las preguntas de análisis

**1. Polo inestable ($\tau$ vs. $\alpha$) y efecto en el periodo de muestreo:**
Con el par ($\tau$) como entrada el polo inestable da $12,24\text{ rad/s}$ ($\tau_{\text{caída}} \approx 81,7\text{ ms}$), y con la aceleración ($\alpha$) baja a $10,60\text{ rad/s}$ ($\tau_{\text{caída}} \approx 94,3\text{ ms}$). Sale menor con $\alpha$ porque el lazo interno de RPM obliga a la rueda a seguir la velocidad pedida, quitando el efecto de reacción libre sobre el eje. Como el robot cae más despacio, el muestreo de $5\text{ ms}$ va más holgado y alcanza a tomar unas 19 muestras antes de que se cumpla una constante de tiempo de caída.

**2. Mínimo de $k_\theta$ frente a la $k_\theta$ diseñada:**
Con nuestra geometría ($r = 0,0335\text{ m}$), el mínimo de $k_\theta$ para vencer la gravedad es $g = 9,81\text{ (m/s}^2\text{)/rad}$, que pasado a unidades del código da $48,8\text{ (RPM/s)/}^\circ$. La ganancia que diseñamos fue de $25,99\text{ (m/s}^2\text{)/rad}$, equivalente a $129,3\text{ (RPM/s)/}^\circ$. Es $2,65\text{ veces}$ el valor mínimo, lo cual tiene sentido para que las ruedas tengan suficiente fuerza de reacción al enderezar el balancín.

**3. Por qué $k_v$ y $k_x$ tienen signo positivo:**
Nuestras ganancias dieron positivas: $k_v = 2,52\text{ s}^{-1}$ ($718,9\text{ (RPM/s)/(m/s)}$) y $k_x = 2,16\text{ s}^{-2}$ ($616,5\text{ (RPM/s)/m}$). Salen positivas porque, para frenar cuando el robot va hacia adelante, las ruedas tienen que acelerar primero hacia ese mismo lado más rápido que el chasis; así se adelantan al centro de masa, inclinan el cuerpo hacia atrás y el mismo peso frena el avance.

**4. Comparación medida vs. modelo (Parte A) y aporte de B8, B7 y B6:**
En el escalón base (B1) medimos $15\text{ ms}$ de subida, $32\text{ \%}$ de sobrepico y error de $0,02\text{ RPM}$, muy cerca del modelo ($20\text{ ms}$ de subida, $23\text{–}25\text{ \%}$ de sobrepico y error $0$). El sobrepico salió un poco más alto en la práctica porque probamos con los $K_p$ del archivo de ejemplo, pero nuestros motores a $10,32\text{ V}$ tienen más ganancia ($K_m \approx 0,35\text{–}0,38$) y empujan más fuerte. Al quitar la prealimentación (B8) la subida se volvió muy lenta ($90\text{–}95\text{ ms}$), demostrando que ella es la que da la rapidez inicial; el modelo de referencia (B7) frenó el empujón extra del PI y bajó el sobrepico a solo $10\text{ \%}$ ($25\text{–}35\text{ ms}$ de subida); y bajar $\lambda$ a $30\text{ ms}$ (B6) aceleró la rueda R a $10\text{ ms}$, pero subió el sobrepico a $37\text{ \%}$ y metió más ruido en el PWM.

**5. Criterio promediado hacia atrás y cruzando por cero (B5):**
Cruzando por cero ($10 \to -10\text{ RPM}$) tuvimos $10\text{ ms}$ de subida con sobrepico de $23\text{ \%}$ (L) y $37\text{ \%}$ (R), y hacia atrás ($-10 \to -30\text{ RPM}$) la rueda L subió de $15\text{ ms}$ a $20\text{ ms}$. Casi no hubo diferencia frente al criterio por sentido porque nuestros motores son bastante simétricos (varían menos de $2\text{ \%}$ entre adelante y atrás), aunque en el sentido más duro la prealimentación promedio calcula un poco menos de PWM y el integrador tarda un momento más en compensarlo.

**6. Elección del juego de polos (tabla B.5) y retardo total estimado:**
Escogemos los polos $[-6,0; -6,5; -1,5; -2,0]\text{ rad/s}$ porque corrigen rápido sin disparar las ganancias ni meterle ruido del giroscopio a los motores. Estimamos que el robot tiene un retardo total de unos $15\text{ a }20\text{ ms}$, sumando la lectura de la IMU y el filtro ($4\text{ a }7\text{ ms}$), medio ciclo de control ($2,5\text{ ms}$) y lo que tarda la rueda en responder con el lazo de RPM ($\sim 10\text{ a }12\text{ ms}$).

**7. Saturación de RPM en simulación (B.4) e inclinación máxima recuperable:**
En la simulación la consigna llega a unas $35\text{–}42\text{ RPM}$, así que no satura ni el límite del programa ($60\text{ RPM}$) ni el máximo físico de nuestros motores ($\sim 87\text{–}94\text{ RPM}$ con PWM 255). Con este diseño, el robot aguanta unos $7^\circ\text{ a }8^\circ$ de inclinación inicial antes de topar las $60\text{ RPM}$ de software, y unos $10^\circ\text{ a }11^\circ$ antes de saturar los motores al máximo.