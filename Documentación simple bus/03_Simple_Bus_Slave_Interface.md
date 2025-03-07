# Interfaces del bus

El modelo de bus define varias interfaces que permiten la comunicación entre los maestros y el bus. Estas interfaces son:

---

## **1. Interfaz Bloqueante (Blocking Interface)**

- **Descripción**: Las funciones de la interfaz bloqueante mueven datos a través del bus en modo ráfaga (burst-mode).
- **Definición de las funciones**:
  ```cpp
  simple_bus_status burst_read(unsigned int unique_priority,
                               int *data,
                               unsigned int start_address,
                               unsigned int length = 1,
                               bool lock = false);

  simple_bus_status burst_write(unsigned int unique_priority,
                                int *data,
                                unsigned int start_address,
                                unsigned int length = 1,
                                bool lock = false);
  ```
- **Detalles**:
  - Estas funciones leen o escriben un bloque de palabras de datos (32 bits).
  - Los datos están apuntados por `data` y tienen un tamaño especificado por `length`.
  - La dirección de inicio se indica con `start_address`.
  - El parámetro `unique_priority` especifica la importancia del maestro y sirve como su identificador.
  - Si `lock` está activado:
    1. Reserva el bus para el uso exclusivo del maestro para la siguiente solicitud.
    2. La transacción no puede ser interrumpida por una solicitud de mayor prioridad.
  - **Estados de retorno**:
    - `SIMPLE_BUS_OK`: Transferencia exitosa.
    - `SIMPLE_BUS_ERROR`: Error durante la transferencia (por ejemplo, dirección no válida o intento de escritura en un esclavo de solo lectura).

---

## **2. Interfaz No Bloqueante (Non-Blocking Interface)**

- **Descripción**: Permite que las solicitudes de lectura o escritura sean colocadas en cola y procesadas posteriormente.
- **Definición de las funciones**:
  ```cpp
  void read(unsigned int unique_priority,
            int *data,
            unsigned int address,
            bool lock = false);

  void write(unsigned int unique_priority,
             int *data,
             unsigned int address,
             bool lock = false);

  simple_bus_status get_status(unsigned int unique_priority);
  ```
- **Detalles**:
  - Las funciones de lectura y escritura devuelven inmediatamente, dejando la solicitud en cola.
  - El estado de la solicitud se consulta con `get_status()`.
  - **Estados de la solicitud**:
    - `SIMPLE_BUS_REQUEST`: La solicitud está en cola, esperando ser procesada.
    - `SIMPLE_BUS_WAIT`: La solicitud está siendo procesada.
    - `SIMPLE_BUS_OK`: La solicitud se completó exitosamente.
    - `SIMPLE_BUS_ERROR`: Ocurrió un error en la solicitud.
  - Si una nueva solicitud se realiza antes de que la actual sea procesada, se produce un error y la ejecución se detiene.

---

## **3. Interfaz Directa (Direct Interface)**

- **Descripción**: Permite transferencias inmediatas de datos, ignorando el protocolo del bus.
- **Definición de las funciones**:
  ```cpp
  bool direct_read(int *data,
                   unsigned int address);

  bool direct_write(int *data,
                    unsigned int address);
  ```
- **Detalles**:
  - Las transferencias ocurren de forma instantánea al invocar la función.
  - **Estados de retorno**:
    - `true`: Transferencia exitosa.
    - `false`: Transferencia fallida (por ejemplo, dirección no mapeada a un esclavo o memoria de solo lectura).

---

