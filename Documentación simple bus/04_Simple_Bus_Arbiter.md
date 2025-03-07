# Arbiter (Árbitro)

El arbiter es un componente esencial del bus que maneja múltiples solicitudes provenientes de los maestros. Determina cuál de estas solicitudes tiene la mayor prioridad y debe ser procesada por el bus.

---

## **Funciones del arbiter**

El arbiter recibe las solicitudes de los maestros conectados al bus y selecciona la más adecuada según un conjunto de reglas predefinidas.

- **Definición de la función principal**:
  ```cpp
  simple_bus_request *arbitrate(const simple_bus_request_vec &Q);
  ```
  - `Q`: Un vector que contiene todas las solicitudes actuales enviadas al bus.
  - **Retorno**: Devuelve la solicitud seleccionada para ser procesada.

---

## **Reglas de arbitraje**
El arbiter sigue estas reglas para decidir cuál solicitud procesar:
1. Si la solicitud actual es una transacción bloqueante con el flag `lock` activado, esta siempre será seleccionada.
2. Si una solicitud previa tenía el flag `lock` activado y el mismo maestro realiza otra solicitud inmediatamente después, esta nueva solicitud será seleccionada.
3. En cualquier otro caso, la solicitud con la prioridad más alta (es decir, el número de prioridad más bajo) será seleccionada.

### **Verificación de unicidad de prioridades**
- El arbiter verifica que todas las prioridades de las solicitudes sean únicas.
- Si hay dos o más solicitudes con la misma prioridad, se genera un mensaje de error y la ejecución se detiene.

---

## **Momento en que el arbiter es llamado**
El arbiter es invocado en los siguientes casos:
1. Cuando el bus termina de procesar la solicitud actual de un maestro.
2. Cuando hay una o más nuevas solicitudes de los maestros pendientes de ser procesadas.

---

## **Nota sobre las prioridades**
- La prioridad más alta está representada por el valor numérico más bajo de `unique_priority`.

---

### **Ejemplo de comportamiento del arbiter**
Supongamos las siguientes solicitudes:

1. **Sin bloqueo ni bloqueo con lock**:
   - Solicitudes: `R[3](-)` (prioridad 3) y `R[4](-)` (prioridad 4).
   - Resultado: `R[3](-)` es seleccionada porque tiene mayor prioridad.

2. **Conflicto de prioridades**:
   - Solicitudes: `R[3](-)`, `R[3](-)`, y `R[4](-)`.
   - Resultado: Error, ya que hay múltiples solicitudes con la misma prioridad.

3. **Uso del flag lock**:
   - Ciclo 1: `R[4](+)` (lock activado).
   - Ciclo 2: `R[3](-)` y `R[4](+)`.
   - Resultado: `R[4](+)` se selecciona nuevamente en el segundo ciclo, incluso si `R[3](-)` tiene mayor prioridad.

---

El arbiter garantiza que el bus siempre procese la solicitud más adecuada, optimizando el acceso de los maestros al bus y respetando las prioridades definidas.

