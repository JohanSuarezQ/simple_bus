# Sincronización y temporización

En este modelo de bus, la temporización juega un papel importante para garantizar la correcta sincronización entre los maestros, el bus y los esclavos. A continuación se describe cómo funciona la temporización en diferentes tipos de solicitudes.

---

## **1. Solicitudes Bloqueantes (Blocking Requests)**

### **Proceso**
1. **Emisión de solicitud**:
   - En el flanco de subida del reloj, un maestro emite una solicitud bloqueante al bus.
   - El estado de la solicitud se establece en `SIMPLE_BUS_REQUEST`.
2. **Procesamiento de la solicitud**:
   - En el siguiente flanco de bajada del reloj, el bus maneja las solicitudes pendientes.
   - El arbiter selecciona la solicitud más adecuada, cuyo estado cambia a `SIMPLE_BUS_WAIT`.
3. **Transferencia de datos**:
   - El bus realiza la transferencia de datos con el esclavo correspondiente.
   - Si la transferencia se realiza con éxito, el estado se actualiza a `SIMPLE_BUS_OK`.
   - En caso de error, el estado se actualiza a `SIMPLE_BUS_ERROR`.
4. **Notificación al maestro**:
   - El maestro es notificado de la finalización de la solicitud en el siguiente flanco de subida del reloj.

### **Ejemplo de flujo**
```cpp
simple_bus_status burst_read(...) {
  // Registro de la solicitud
  ...
  wait(request->transfer_done); // Espera evento de finalización
  wait(clock->posedge_event()); // Espera el siguiente flanco de subida
  return get_status(priority);  // Retorna el estado de la solicitud
}
```

---

## **2. Solicitudes No Bloqueantes (Non-Blocking Requests)**

### **Proceso**
1. **Emisión de solicitud**:
   - En el flanco de subida del reloj, un maestro emite una solicitud no bloqueante al bus.
   - El estado de la solicitud se establece en `SIMPLE_BUS_REQUEST` y la función retorna inmediatamente.
2. **Procesamiento de la solicitud**:
   - En el siguiente flanco de bajada del reloj, el bus maneja las solicitudes pendientes.
   - El arbiter selecciona la solicitud más adecuada, cuyo estado cambia a `SIMPLE_BUS_WAIT` o `SIMPLE_BUS_ERROR`.
3. **Estado de la solicitud**:
   - El maestro consulta el estado de la solicitud usando `get_status()` hasta que el estado sea `SIMPLE_BUS_OK` o `SIMPLE_BUS_ERROR`.

### **Ejemplo de flujo**
```cpp
bus_port->read(unique_priority, &mydata, addr, lock);
while ((bus_port->get_status(unique_priority) != SIMPLE_BUS_OK) &&
       (bus_port->get_status(unique_priority) != SIMPLE_BUS_ERROR)) {
    wait();
}
```

---

## **3. Solicitudes Directas (Direct Requests)**

### **Proceso**
1. **Emisión de solicitud**:
   - El maestro realiza una solicitud directa al bus, omitiendo el protocolo normal del bus.
2. **Transferencia inmediata**:
   - El bus selecciona el esclavo correspondiente en función de la dirección proporcionada y realiza la transferencia de datos.
   - La solicitud se procesa de inmediato sin respetar estados de espera.
3. **Estado de la transferencia**:
   - Retorna `true` si la transferencia fue exitosa o `false` en caso de error (por ejemplo, dirección no válida).

### **Ejemplo de flujo**
```cpp
bool success = bus_port->direct_read(&mydata, addr);
if (!success) {
    // Manejo del error
}
```

---

### **Comparación de tipos de solicitudes**
| Tipo de Solicitud     | Procesamiento         | Retorno         |
|-----------------------|-----------------------|-----------------|
| **Bloqueante**        | Espera finalización  | Estado final    |
| **No Bloqueante**     | Procesa en segundo plano | Consulta de estado |
| **Directa**           | Inmediato            | `true` o `false` |

---

