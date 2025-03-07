# Banco de Pruebas (Testbench)

El banco de pruebas es una parte fundamental del modelo de bus, ya que permite verificar su funcionalidad mediante simulaciones. En este sistema, el testbench incluye un bus con un arbiter, tres maestros y dos esclavos que representan memorias de acceso aleatorio (RAM).

---

## **Estructura del Banco de Pruebas**

El banco de pruebas está definido como un módulo jerárquico llamado `simple_bus_test`. Este módulo contiene:

1. **Canal de reloj**:
   - El canal de reloj (`sc_clock C1`) sincroniza las operaciones del bus, los maestros y los esclavos.

2. **Instancias principales**:
   - **Bus** (`simple_bus`): El canal de comunicación principal.
   - **Arbiter** (`simple_bus_arbiter`): Gestiona las prioridades de acceso al bus.
   - **Maestros**: Tres maestros, cada uno utilizando una interfaz específica del bus:
     - Maestro bloqueante (`simple_bus_master_blocking`).
     - Maestro no bloqueante (`simple_bus_master_non_blocking`).
     - Maestro directo (`simple_bus_master_direct`).
   - **Esclavos**: Dos esclavos que simulan memorias:
     - Memoria rápida (`simple_bus_fast_mem`): No tiene estados de espera.
     - Memoria lenta (`simple_bus_slow_mem`): Contiene estados de espera configurables.

---

## **Esclavos**

### **1. Memoria Rápida (Fast Memory)**
- Modelo sin estados de espera.
- No requiere puerto de reloj.
- Responde inmediatamente a las solicitudes del bus y actualiza el estado.

**Definición:**
```cpp
class simple_bus_fast_mem : public simple_bus_slave_if,
                            public sc_module {
public:
    // Constructor
    simple_bus_fast_mem(sc_module_name name, unsigned int start_address, unsigned int end_address);
    ...
};
```
- **Direcciones**:
  - `start_address`: Dirección inicial de la memoria.
  - `end_address`: Dirección final de la memoria.

### **2. Memoria Lenta (Slow Memory)**
- Modelo con estados de espera configurables.
- Contiene un puerto de reloj (`sc_in_clk clock`).

**Definición:**
```cpp
class simple_bus_slow_mem : public simple_bus_slave_if,
                            public sc_module {
public:
    // Constructor
    simple_bus_slow_mem(sc_module_name name, unsigned int start_address,
                        unsigned int end_address, unsigned int nr_wait_states);
    ...
};
```
- **Configuración:**
  - `nr_wait_states`: Número de ciclos de espera antes de completar una solicitud.

---

## **Maestros**

### **1. Maestro No Bloqueante**
- Utiliza la interfaz `simple_bus_non_blocking_if`.
- Realiza operaciones de lectura, modifica los datos y los escribe de vuelta en la memoria.
- Proceso principal (`main_action`) implementado como un hilo.

**Definición:**
```cpp
SC_MODULE(simple_bus_master_non_blocking) {
    sc_in_clk clock;
    sc_port<simple_bus_non_blocking_if> bus_port;
    ...
};
```

### **2. Maestro Bloqueante**
- Utiliza la interfaz `simple_bus_blocking_if`.
- Realiza lecturas y escrituras en bloques.
- Tiene prioridad más baja que el maestro no bloqueante para permitir interrupciones durante transacciones largas.

**Definición:**
```cpp
SC_MODULE(simple_bus_master_blocking) {
    sc_in_clk clock;
    sc_port<simple_bus_blocking_if> bus_port;
    ...
};
```

### **3. Maestro Directo**
- Utiliza la interfaz `simple_bus_direct_if`.
- Lee ubicaciones de memoria específicas en intervalos de tiempo y las imprime.

**Definición:**
```cpp
SC_MODULE(simple_bus_master_direct) {
    sc_in_clk clock;
    sc_port<simple_bus_direct_if> bus_port;
    ...
};
```

---

## **Esquema del Banco de Pruebas**
El banco de pruebas contiene los siguientes componentes conectados entre sí:

```text
      +---------+   +---------+   +---------+
      | Maestro |   | Maestro |   | Maestro |
  +-->| Bloq.   |-->| No Bloq.|-->| Directo |
  |   +---------+   +---------+   +---------+
  |        |             |             |
  |        +-------------+-------------+
  |                      |
  |      /-------------------------------\
  +->[*]/             Bus                \         +---------+
      \---------------------------------/[*]----->| Arbiter |
  |                     | |                         +---------+
  |                 ____/ \____
clock              /           \
  |                |           |
  |            +---+---+   +---+---+
  |            | Esclavo|   | Esclavo|
  +----------->| Rápido |   | Lento  |
               +-------+   +-------+

Leyenda:
- [*]: Puerto
- (|): Interfaz
```

---

