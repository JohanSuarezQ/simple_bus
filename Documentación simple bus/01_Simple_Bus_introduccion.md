# Introducción: El modelo de bus

Esta es la descripción de un modelo de bus abstracto simple. El modelado se realiza a nivel de transacciones y se basa en una sincronización basada en ciclos.

### **Sincronización basada en ciclos**
- Esta técnica reduce la precisión temporal para obtener una alta velocidad de simulación.
- El objetivo es modelar la organización y el movimiento de datos en el sistema en cada ciclo de reloj, en comparación con un sistema real equivalente.
- Los eventos dentro de un ciclo (subciclo) no se consideran relevantes.

### **Modelado de transacciones**
- La comunicación entre componentes se describe como llamadas a funciones.
- Los eventos o secuencias de eventos en un grupo de señales se representan mediante un conjunto de llamadas a funciones en una interfaz de software abstracta.
- Este enfoque permite una velocidad de simulación más alta que las interfaces basadas en pines y acelera el proceso de construcción del modelo.

### **Sincronización en el diseño**
- Este diseño utiliza una forma general de sincronización en la que:
  - Los módulos conectados al bus se ejecutan en el flanco de subida del reloj.
  - El bus mismo se ejecuta en el flanco de bajada del reloj.
- Esta técnica se utiliza para lograr un alto rendimiento de simulación en modelos abstractos de buses.
- **Nota**: Esta sincronización no implica que la implementación real del diseño use un bus sensible al flanco de bajada del reloj. La implementación final podría tener un solo reloj sensible solo al flanco de subida.
- También podrían haberse usado otros esquemas de sincronización (como canales primitivos con `request_update/update`), aunque probablemente resulten en un código más complicado y más lento para este diseño.

