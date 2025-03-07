# Ejecución del Ejemplo Simple Bus

El ejemplo del **Simple Bus** describe el flujo de ejecución y las acciones realizadas durante la simulación. Esta sección detalla cómo se lleva a cabo el procesamiento de las solicitudes de bus en función de sus prioridades y estados.

---

## **1. Monitoreo del Comportamiento en Ejecución**

El comportamiento en tiempo de ejecución se puede analizar inspeccionando:

- **Colección de solicitudes pendientes en el arbiter**.
- **Identificación de la solicitud seleccionada** por el arbiter en cada ciclo.

El arbiter se invoca en cada ciclo si hay solicitudes pendientes. Las solicitudes se procesan una por una, dividiendo las solicitudes en partes cuando se trata de transacciones de ráfaga.

### **Ejemplo de Prioridades y Estados de Solicitudes**

Se utilizan las siguientes convenciones:
- **R[p](-)**: Solicitud con prioridad `p`, sin bloqueo.
- **R[p](+)**: Solicitud con prioridad `p`, con bloqueo.

#### Escenarios:
1. **Unica solicitud:**
    ```
    R[3](-)
    ```
    - Resultado: La única solicitud es seleccionada.

2. **Múltiples solicitudes con prioridades diferentes:**
    ```
    R[3](-) R[4](-)
    ```
    - Resultado: `R[3](-)` es seleccionada debido a su mayor prioridad.

3. **Múltiples solicitudes con la misma prioridad:**
    ```
    R[3](-) R[3](-) R[4](-)
    ```
    - Resultado: Error. Dos solicitudes con la misma prioridad. La simulación termina con un mensaje de error.

4. **Solicitud con bloqueo:**
    ```
    Ciclo 1: R[3](+)
    Ciclo 2: R[3](+)
    ```
    - Resultado: La solicitud con bloqueo en el ciclo 1 mantiene su acceso en el ciclo 2.

---

## **2. Bloqueo y Priorización**

El bloqueo afecta las decisiones del arbiter cuando:

1. **Bloqueo no seguido por otra solicitud:**
    ```
    Ciclo 1: R[3](+)
    Ciclo 2: R[4](+)
    ```
    - Resultado: La solicitud bloqueada en el ciclo 1 no se mantiene en el ciclo 2. `R[4](+)` es seleccionada.

2. **Bloqueo con solicitud del mismo maestro:**
    ```
    Ciclo 1: R[4](+)
    Ciclo 2: R[4](+), R[3](-)
    ```
    - Resultado: `R[4](+)` mantiene el acceso debido al bloqueo, incluso cuando `R[3](-)` tiene mayor prioridad.

3. **Conflicto entre solicitudes bloqueadas:**
    ```
    Ciclo 1: R[4](+)
    Ciclo 2: R[3](+), R[4](+)
    ```
    - Resultado: `R[4](+)` es seleccionada nuevamente en el ciclo 2. `R[3](+)` debe esperar a pesar de su mayor prioridad.

---

## **3. Procesamiento de Solicitudes de Ráfaga**

Las solicitudes de ráfaga son descompuestas en partes individuales que se procesan por separado. Cada parte se considera una solicitud independiente y se selecciona en función de las prioridades.

### Ejemplo:
- **Solicitud de ráfaga:**
    ```
    R[4](+)
    ```
    - Durante varios ciclos, las partes individuales de la solicitud se procesan una a una, respetando el bloqueo.

- **Interrupción por prioridad:**
    ```
    Ciclo 1: R[4](-) parte 1
    Ciclo 2: R[3](-)
    Ciclo 3: R[4](-) parte 2
    ```
    - Resultado: `R[3](-)` interrumpe `R[4](-)` en el ciclo 2 debido a su mayor prioridad. La parte 2 de `R[4](-)` se procesa en el ciclo 3.

---

## **4. Resumen del Comportamiento del Arbiter**

El arbiter sigue estas reglas:
- **Prioridad más alta** (número más bajo) tiene preferencia.
- **Bloqueo** garantiza acceso continuo mientras el maestro siga emitiendo solicitudes.
- **Errores** ocurren si:
  - Dos o más solicitudes tienen la misma prioridad.
  - Un maestro emite una solicitud mientras otra está en progreso.

Con este comportamiento, el modelo de bus gestiona eficientemente las solicitudes de acceso, garantizando un control adecuado de prioridades y bloqueos.

