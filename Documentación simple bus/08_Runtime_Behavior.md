# Comportamiento en Tiempo de Ejecución

El comportamiento en tiempo de ejecución describe cómo se comportan los diferentes componentes del banco de pruebas mientras interactúan con el bus. Cada maestro tiene un comportamiento específico y emite solicitudes de bus de manera independiente.

---

## **1. Maestro Directo**
El maestro directo actúa como un monitor que lee periódicamente ubicaciones de memoria y muestra su contenido en la salida.

### **Comportamiento**:
- Lee cuatro ubicaciones de memoria adyacentes en intervalos de tiempo.
- Utiliza la interfaz directa del bus para realizar transferencias inmediatas.
- Los resultados se imprimen en la consola.

**Ejemplo de código:**
```cpp
while (true)
{
    bus_port->direct_read(&mydata[0], m_address);
    bus_port->direct_read(&mydata[1], m_address + 4);
    bus_port->direct_read(&mydata[2], m_address + 8);
    bus_port->direct_read(&mydata[3], m_address + 12);

    if (m_verbose)
        sb_fprintf(stdout, "%f %s : mem[%x:%x] = (%x, %x, %x, %x)\n",
                   sc_time_stamp(), name(), m_address, m_address + 15,
                   mydata[0], mydata[1], mydata[2], mydata[3]);

    wait(m_timeout, SC_NS);
}
```
### **Parámetros configurables:**
- **`m_address`**: Dirección base de memoria.
- **`m_timeout`**: Intervalo de tiempo entre lecturas.
- **`m_verbose`**: Controla si se imprime o no la salida.

---

## **2. Maestro No Bloqueante**
El maestro no bloqueante realiza operaciones de lectura y escritura de palabras individuales, verificando el estado de las solicitudes hasta completarlas.

### **Comportamiento**:
1. Lee una palabra de la memoria utilizando la interfaz no bloqueante.
2. Realiza una operación aritmética sobre los datos leídos.
3. Escribe la palabra modificada de vuelta en la misma dirección de memoria.

**Ejemplo de código:**
```cpp
wait(); // Para el próximo flanco de reloj
while (true)
{
    bus_port->read(m_unique_priority, &mydata, addr, m_lock);
    while ((bus_port->get_status(m_unique_priority) != SIMPLE_BUS_OK) &&
           (bus_port->get_status(m_unique_priority) != SIMPLE_BUS_ERROR))
        wait();

    if (bus_port->get_status(m_unique_priority) == SIMPLE_BUS_ERROR)
        sb_fprintf(stdout, "%f %s : ERROR no se puede leer desde %x\n",
                   sc_time_stamp(), name(), addr);

    mydata += cnt;
    cnt++;

    bus_port->write(m_unique_priority, &mydata, addr, m_lock);
    while ((bus_port->get_status(m_unique_priority) != SIMPLE_BUS_OK) &&
           (bus_port->get_status(m_unique_priority) != SIMPLE_BUS_ERROR))
        wait();

    if (bus_port->get_status(m_unique_priority) == SIMPLE_BUS_ERROR)
        sb_fprintf(stdout, "%f %s : ERROR no se puede escribir en %x\n",
                   sc_time_stamp(), name(), addr);

    wait(m_timeout, SC_NS);
    addr += 4; // Siguiente palabra
    if (addr > (m_start_address + 0x80)) {
        addr = m_start_address;
        cnt = 0;
    }
}
```
### **Parámetros configurables:**
- **`addr`**: Dirección base de la memoria.
- **`m_timeout`**: Intervalo de tiempo entre iteraciones.
- **`m_lock`**: Indica si las solicitudes son bloqueadas por prioridad.

---

## **3. Maestro Bloqueante**
El maestro bloqueante realiza operaciones de lectura y escritura por bloques, lo que permite manejar conjuntos de datos en cada transacción.

### **Comportamiento**:
1. Lee un bloque de palabras desde una dirección base de memoria.
2. Realiza operaciones aritméticas sobre el bloque de datos.
3. Escribe el bloque de datos modificado de vuelta en la memoria.
4. Espera un tiempo definido antes de la siguiente iteración.

**Ejemplo de código:**
```cpp
while (true)
{
    wait(); // Para el próximo flanco de reloj
    status = bus_port->burst_read(m_unique_priority, mydata, m_address, mylength, m_lock);
    if (status == SIMPLE_BUS_ERROR)
        sb_fprintf(stdout, "%f %s : ERROR en lectura bloqueante en %x\n",
                   sc_time_stamp(), name(), m_address);

    for (i = 0; i < mylength; ++i)
    {
        mydata[i] += i;
        wait();
    }

    status = bus_port->burst_write(m_unique_priority, mydata, m_address, mylength, m_lock);
    if (status == SIMPLE_BUS_ERROR)
        sb_fprintf(stdout, "%f %s : ERROR en escritura bloqueante en %x\n",
                   sc_time_stamp(), name(), m_address);

    wait(m_timeout, SC_NS);
}
```
### **Parámetros configurables:**
- **`m_address`**: Dirección base del bloque de datos.
- **`m_timeout`**: Tiempo de espera entre iteraciones.
- **`m_lock`**: Indica si las transacciones deben ser bloqueadas.

---

## **Resumen del Comportamiento**
- Cada maestro puede emitir solicitudes en ciclos diferentes o simultáneos.
- Las solicitudes se procesan según las prioridades definidas por el arbiter.
- Los maestros tienen comportamientos configurables mediante parámetros definidos en su construcción.

Con este enfoque, el modelo logra simular eficientemente un entorno de bus simple, considerando tanto bloqueos como prioridades en tiempo de ejecución.

