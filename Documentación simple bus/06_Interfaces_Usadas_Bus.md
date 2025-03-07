# Interfaces Utilizadas

En esta sección se describen las interfaces utilizadas en el modelo de bus simple y su implementación en el sistema. Estas interfaces facilitan la comunicación entre los diferentes componentes del sistema: el bus, los maestros, los esclavos y el arbiter.

---

## **Clases de interfaces**

### **1. Interfaces del Bus**
Estas interfaces definen los métodos utilizados para interactuar con el bus.

- **Interfaz directa (`simple_bus_direct_if`)**:
  ```cpp
  class simple_bus_direct_if : virtual public sc_interface
  {
      // Métodos de lectura y escritura directos
      virtual bool direct_read(int *data, unsigned int address) = 0;
      virtual bool direct_write(int *data, unsigned int address) = 0;
  };
  ```
  - Permite transferencias inmediatas de datos ignorando los protocolos del bus.

- **Interfaz no bloqueante (`simple_bus_non_blocking_if`)**:
  ```cpp
  class simple_bus_non_blocking_if : virtual public sc_interface
  {
      // Métodos para solicitudes no bloqueantes
      virtual void read(unsigned int priority, int *data, unsigned int address, bool lock) = 0;
      virtual void write(unsigned int priority, int *data, unsigned int address, bool lock) = 0;
      virtual simple_bus_status get_status(unsigned int priority) = 0;
  };
  ```
  - Permite que los maestros emitan solicitudes sin bloquear el flujo de ejecución.

- **Interfaz bloqueante (`simple_bus_blocking_if`)**:
  ```cpp
  class simple_bus_blocking_if : virtual public sc_interface
  {
      // Métodos para solicitudes bloqueantes
      virtual simple_bus_status burst_read(unsigned int priority, int *data, unsigned int start_address, unsigned int length, bool lock) = 0;
      virtual simple_bus_status burst_write(unsigned int priority, int *data, unsigned int start_address, unsigned int length, bool lock) = 0;
  };
  ```
  - Proporciona funciones para transferencias de datos en modo bloqueante.

---

### **2. Interfaz del Arbiter**
La interfaz del arbiter permite al bus comunicarse con el módulo que decide cuál maestro tiene acceso al bus.

- **Definición**:
  ```cpp
  class simple_bus_arbiter_if : virtual public sc_interface
  {
      virtual simple_bus_request* arbitrate(const simple_bus_request_vec &requests) = 0;
  };
  ```
  - **Función principal**: Selecciona la solicitud más adecuada según las reglas de prioridad.

---

### **3. Interfaz del Esclavo**
Los esclavos utilizan esta interfaz para comunicarse con el bus.

- **Definición**:
  ```cpp
  class simple_bus_slave_if : public simple_bus_direct_if
  {
      virtual simple_bus_status read(int *data, unsigned int address) = 0;
      virtual simple_bus_status write(int *data, unsigned int address) = 0;
      virtual unsigned int start_address() const = 0;
      virtual unsigned int end_address() const = 0;
  };
  ```
  - Incluye métodos para la lectura/escritura y funciones para obtener el rango de direcciones manejado por el esclavo.

---

## **Implementación de las interfaces**

### **Clase `simple_bus`**
El bus es un canal jerárquico que implementa las interfaces del bus y sirve como punto de comunicación entre maestros y esclavos.

- **Definición**:
  ```cpp
  class simple_bus : public simple_bus_direct_if,
                     public simple_bus_non_blocking_if,
                     public simple_bus_blocking_if,
                     public sc_module
  {
      sc_in_clk clock;
      sc_port<simple_bus_slave_if, 0> slave_port;
      sc_port<simple_bus_arbiter_if> arbiter_port;
      ...
  };
  ```
  - Proporciona puertos para conectar esclavos y el arbiter.
  - La implementación del proceso principal utiliza sensibilidad dinámica para comunicarse con las funciones de la interfaz del bus.

### **Clase `simple_bus_arbiter`**
El arbiter implementa la interfaz `simple_bus_arbiter_if` y decide qué solicitud procesar en función de las prioridades de los maestros.

- **Definición**:
  ```cpp
  class simple_bus_arbiter : public simple_bus_arbiter_if,
                             public sc_module
  {
      ...
  };
  ```

---

