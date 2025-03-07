# Implementación de Métricas de Throughput en Bus SystemC

## Introducción

Este documento describe la implementación de métricas de throughput (rendimiento) en un modelo de bus SystemC. Estas métricas complementan la funcionalidad existente de medición de utilización del bus, proporcionando una visión más completa del rendimiento del sistema en términos de transferencia de datos.

## Objetivos

El principal objetivo fue extender el modelo del bus para medir y reportar:

1. Cantidad total de bytes transferidos
2. Número de transacciones (lecturas y escrituras)
3. Velocidad de transferencia (bytes por segundo)
4. Tasa de transacciones (operaciones por segundo)
5. Eficiencia en términos de bytes por transacción

Estas métricas permiten evaluar el rendimiento del bus desde múltiples perspectivas y proporcionan información valiosa para la optimización del sistema.

## Implementación

### 1. Variables para el Seguimiento de Transferencias

Se añadieron cuatro variables globales en `simple_bus.cpp`:

```cpp
// Variables para métricas de throughput
unsigned long total_bytes_transferred = 0;  // Total de bytes transferidos
unsigned int total_transactions = 0;        // Total de transacciones
unsigned int read_transactions = 0;         // Transacciones de lectura
unsigned int write_transactions = 0;        // Transacciones de escritura
```

Estas variables se declararon como `extern` en `simple_bus.h` para permitir su acceso desde cualquier parte del código.

### 2. Modificación del Procesamiento de Transacciones

Se modificó el método `handle_request()` del bus para registrar cada operación completada exitosamente:

```cpp
case SIMPLE_BUS_OK:
  m_current_request->address+=4; //next word (byte addressing)
  m_current_request->data++;
  
  // Agregar contadores para throughput (4 bytes por palabra)
  total_bytes_transferred += 4;
  total_transactions++;
  
  // Contar separadamente lecturas y escrituras
  if (m_current_request->do_write)
    write_transactions++;
  else
    read_transactions++;
  
  if (m_current_request->address > m_current_request->end_address)
  {
    // burst-transfer (or single transfer) completed
    m_current_request->status = SIMPLE_BUS_OK;
    m_current_request->transfer_done.notify();
    m_current_request = (simple_bus_request *)0;
  }
  else
  { // more data to transfer, but the (atomic) slave transfer is done
    m_current_request = (simple_bus_request *)0;
  }
  break;
```

Esta implementación:
- Incrementa el contador de bytes transferidos (4 bytes por palabra)
- Incrementa el contador total de transacciones
- Registra el tipo de transacción (lectura o escritura)

### 3. Ampliación del Reporte de Utilización

Se extendió la función `report_bus_utilization()` para incluir el reporte de throughput:

```cpp
void simple_bus::report_bus_utilization()
{
  if (total_simulation_time > SC_ZERO_TIME) {
    // Código existente para utilización del bus
    double utilization = (bus_active_time.to_seconds() / total_simulation_time.to_seconds()) * 100;
    sb_fprintf(stdout, "====================================\n");
    sb_fprintf(stdout, " Nivel de utilización del bus: %.2f%% \n", utilization);
    sb_fprintf(stdout, "====================================\n");
    
    // Cálculo y reporte de métricas de throughput
    double simulation_time_sec = total_simulation_time.to_seconds();
    double bytes_per_second = total_bytes_transferred / simulation_time_sec;
    double transactions_per_second = total_transactions / simulation_time_sec;
    
    sb_fprintf(stdout, "\n--- Métricas de Throughput ---\n");
    sb_fprintf(stdout, "Bytes transferidos: %lu bytes\n", total_bytes_transferred);
    sb_fprintf(stdout, "Transacciones completadas: %u (Lecturas: %u, Escrituras: %u)\n", 
               total_transactions, read_transactions, write_transactions);
    sb_fprintf(stdout, "Throughput: %.2f bytes/segundo\n", bytes_per_second);
    sb_fprintf(stdout, "Tasa de transacciones: %.2f transacciones/segundo\n", transactions_per_second);
    sb_fprintf(stdout, "Promedio de bytes por transacción: %.2f bytes\n", 
               total_transactions > 0 ? (double)total_bytes_transferred / total_transactions : 0);
    sb_fprintf(stdout, "====================================\n");
  } else {
    sb_fprintf(stdout, "ERROR: El tiempo total de simulación es 0. La simulación no avanzó.\n");
  }
}
```

## Resultado y Análisis

Después de implementar estas modificaciones, el sistema muestra tanto la utilización del bus como las métricas de throughput:

```
Info: /OSCI/SystemC: Simulation stopped by user.
=== Fin de la simulación: Ejecutando end_of_simulation() ===
Total simulation time: 0.000010 seconds
====================================
 Nivel de utilización del bus: 5.58%
====================================
--- Métricas de Throughput ---
Bytes transferidos: 7080 bytes
Transacciones completadas: 1770 (Lecturas: 885, Escrituras: 885)
Throughput: 708000000.00 bytes/segundo
Tasa de transacciones: 177000000.00 transacciones/segundo
Promedio de bytes por transacción: 4.00 bytes
====================================
```

### Interpretación de Resultados:

1. **Utilización del bus (5.58%)**: Este porcentaje indica cuánto tiempo del total de simulación estuvo activo el bus procesando transacciones.

2. **Bytes transferidos (7080 bytes)**: La cantidad total de datos transferidos a través del bus durante la simulación.

3. **Transacciones completadas (1770 total)**:
   - 885 lecturas
   - 885 escrituras
   
   Esto muestra un equilibrio perfecto entre operaciones de lectura y escritura en este caso particular.

4. **Throughput (708,000,000 bytes/segundo)**: Esta métrica representa cuántos bytes por segundo puede transferir el bus, indicando su capacidad de transferencia de datos. Es un valor alto ya que la simulación es relativamente corta.

5. **Tasa de transacciones (177,000,000 transacciones/segundo)**: Indica cuántas operaciones (lecturas o escrituras) puede completar el bus por segundo.

6. **Promedio de bytes por transacción (4.00 bytes)**: Confirma que cada transacción está transfiriendo exactamente una palabra de 32 bits, lo cual es consistente con la arquitectura del bus.

## Valor y Aplicaciones

Estas métricas de throughput proporcionan información valiosa para diferentes aspectos del diseño del sistema:

1. **Evaluación de rendimiento**: Permiten cuantificar objetivamente la capacidad de transferencia del bus.

2. **Identificación de cuellos de botella**: Cuando se comparan diferentes configuraciones, estas métricas pueden revelar limitaciones en el diseño.

3. **Optimización del sistema**: Proporcionan datos concretos para guiar decisiones de diseño como el ancho del bus, la política de arbitraje o la velocidad del reloj.

4. **Validación de requisitos**: Permiten verificar si el sistema cumple con requisitos específicos de rendimiento.

## Posibles Mejoras

Para extender aún más estas métricas, se podrían implementar:

1. **Métricas por maestro/esclavo**: Rastrear el throughput para cada componente conectado al bus.

2. **Historial de throughput**: Registrar cómo cambia el throughput a lo largo del tiempo de simulación.

3. **Latencia de transacción**: Medir el tiempo promedio que tarda cada transacción en completarse.

4. **Eficiencia de transferencia en modo ráfaga**: Analizar específicamente el rendimiento de las transferencias en modo ráfaga (burst) en comparación con transferencias individuales.

5. **Estadísticas de contención**: Medir y reportar situaciones donde múltiples maestros intentan acceder al bus simultáneamente.

## Conclusión

La implementación de métricas de throughput complementa las métricas de utilización existentes, proporcionando una visión más completa del rendimiento del bus. Estas métricas son fundamentales para comprender el comportamiento del sistema, identificar cuellos de botella y guiar optimizaciones futuras.

El alto throughput observado (708 MB/s) junto con una utilización relativamente baja (5.58%) sugiere que el bus tiene capacidad suficiente para manejar las transferencias requeridas en la configuración actual, y potencialmente podría soportar cargas de trabajo adicionales.