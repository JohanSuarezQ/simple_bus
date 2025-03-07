# Archivos del Simple Bus

Esta sección lista y describe los archivos incluidos en el ejemplo **Simple Bus**. Estos archivos son esenciales para entender y trabajar con el modelo proporcionado.

---

## **Archivos de Documentación**

### **`README`**
- Contiene la descripción general del ejemplo Simple Bus.
- Proporciona instrucciones para la compilación y ejecución.

### **`SLIDES.pdf`**
- Presentación en formato PDF con información visual sobre el modelo Simple Bus.

### **`ChangeLog`**
- Registro de cambios realizados en el código fuente del ejemplo.

### **`LEGAL`**
- Información sobre derechos de autor y licencias del ejemplo Simple Bus.

---

## **Archivos de Construcción**

### **`Makefile.gcc`**
- Script de construcción para compiladores GCC.

### **`Makefile.hp`**
- Script de construcción para plataformas HP-UX.

### **`Makefile.linux`**
- Script de construcción para sistemas Linux.

### **`Makefile.sun`**
- Script de construcción para plataformas Solaris.

### **`Makefile.srcs`**
- Define las dependencias y fuentes requeridas para construir el proyecto.

---

## **Archivos del Modelo Simple Bus**

### **Archivos Principales**
- **`simple_bus.cpp`**: Implementación del bus.
- **`simple_bus.h`**: Declaración de la clase principal del bus.

### **Arbiter (Arbitraje)**
- **`simple_bus_arbiter.cpp`**: Implementación del arbiter.
- **`simple_bus_arbiter.h`**: Declaración de la clase arbiter.
- **`simple_bus_arbiter_if.h`**: Interfaz del arbiter.

### **Interfaces del Bus**
- **`simple_bus_blocking_if.h`**: Interfaz para solicitudes bloqueantes.
- **`simple_bus_direct_if.h`**: Interfaz para solicitudes directas.
- **`simple_bus_non_blocking_if.h`**: Interfaz para solicitudes no bloqueantes.

### **Componentes de Memoria**
- **`simple_bus_fast_mem.h`**: Modelo de memoria rápida (sin estados de espera).
- **`simple_bus_slow_mem.h`**: Modelo de memoria lenta (con estados de espera).

### **Maestros del Bus**
- **`simple_bus_master_blocking.cpp`**: Implementación del maestro bloqueante.
- **`simple_bus_master_blocking.h`**: Declaración del maestro bloqueante.
- **`simple_bus_master_direct.cpp`**: Implementación del maestro directo.
- **`simple_bus_master_direct.h`**: Declaración del maestro directo.
- **`simple_bus_master_non_blocking.cpp`**: Implementación del maestro no bloqueante.
- **`simple_bus_master_non_blocking.h`**: Declaración del maestro no bloqueante.

### **Otras Interfaces y Herramientas**
- **`simple_bus_slave_if.h`**: Interfaz para los esclavos del bus.
- **`simple_bus_request.h`**: Definición de solicitudes de bus.
- **`simple_bus_types.cpp`**: Tipos utilizados en el modelo.
- **`simple_bus_types.h`**: Declaración de tipos del modelo.
- **`simple_bus_tools.cpp`**: Herramientas adicionales para el modelo.

---

## **Archivos de Prueba y Ejecución**

### **`simple_bus_main.cpp`**
- Punto de entrada principal para ejecutar el ejemplo.

### **`simple_bus_test.h`**
- Banco de pruebas (testbench) para el modelo Simple Bus.

### **`simple_bus.golden`**
- Archivo de referencia para verificar la salida esperada del ejemplo.

---

## **Resumen**

Estos archivos proporcionan todo lo necesario para compilar, ejecutar y estudiar el modelo **Simple Bus**. La documentación, scripts de construcción y componentes del modelo están organizados para facilitar la comprensión y el uso del ejemplo.

