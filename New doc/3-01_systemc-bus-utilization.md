# Implementación de Métricas de Utilización del Bus en SystemC

## Introducción

Este documento detalla la implementación de una funcionalidad para medir y reportar la utilización del bus en un modelo de simulación SystemC. La funcionalidad permite calcular qué porcentaje del tiempo total de simulación está activo el bus procesando solicitudes, proporcionando una métrica útil para analizar el rendimiento del sistema.

## Problema Inicial

El modelo original del bus (`simple_bus`) no incluía ninguna funcionalidad para medir su utilización a lo largo del tiempo. Esto dificultaba la evaluación del rendimiento del sistema y la identificación de posibles cuellos de botella en la comunicación.

El objetivo era implementar una solución que:
1. Registrara el tiempo durante el cual el bus está activo (procesando solicitudes)
2. Calculara la utilización como un porcentaje del tiempo total de simulación
3. Presentara los resultados al final de la simulación

## Enfoque y Solución

La solución consistió en implementar un contador de tiempo que registra cuándo el bus está activo y calcular la proporción respecto al tiempo total de simulación.

### 1. Variables para el Seguimiento del Tiempo

Se añadieron tres variables globales en `simple_bus.cpp`:

```cpp
// Definición de variables globales para medir el uso del bus
sc_time bus_active_time = SC_ZERO_TIME;     // Tiempo que el bus está en uso
sc_time total_simulation_time = SC_ZERO_TIME; // Tiempo total de simulación
sc_time last_time_stamp = SC_ZERO_TIME;      // Marca de tiempo anterior
```

Estas variables se declararon como `extern` en `simple_bus.h` para permitir su acceso desde cualquier parte del código.

### 2. Modificación del Ciclo Principal del Bus

Se modificó el método `main_action()` del bus para registrar el tiempo activo:

```cpp
void simple_bus::main_action()
{
  // Si el bus está en uso, contar el tiempo activo
  if (m_current_request) {
    sc_time current_time = sc_time_stamp();
    bus_active_time += current_time - last_time_stamp;
  }
  
  // Actualizar el timestamp para el próximo cálculo
  last_time_stamp = sc_time_stamp();
  
  // Resto del código original...
}
```

Esta implementación:
- Verifica si el bus tiene una solicitud en proceso (`m_current_request`)
- Si hay una solicitud, suma al contador de tiempo activo la diferencia entre el tiempo actual y la última marca registrada
- Actualiza la marca de tiempo para el siguiente ciclo

### 3. Implementación del Reporte de Utilización

Se creó una función específica para calcular y mostrar la utilización del bus:

```cpp
void simple_bus::report_bus_utilization()
{
  if (total_simulation_time > SC_ZERO_TIME) {
    double utilization = (bus_active_time.to_seconds() / total_simulation_time.to_seconds()) * 100;
    sb_fprintf(stdout, "====================================\n");
    sb_fprintf(stdout, " Nivel de utilización del bus: %.2f%% \n", utilization);
    sb_fprintf(stdout, "====================================\n");
  } else {
    sb_fprintf(stdout, "ERROR: El tiempo total de simulación es 0. La simulación no avanzó.\n");
  }
}
```

### 4. Integración con el Ciclo de Vida de SystemC

Para garantizar que la utilización se calcule correctamente al final de la simulación, se implementó el método `end_of_simulation()`:

```cpp
void simple_bus::end_of_simulation()
{
  // Registrar el tiempo total de simulación
  total_simulation_time = sc_time_stamp();
  
  sb_fprintf(stdout, "=== Fin de la simulación: Ejecutando end_of_simulation() ===\n");
  sb_fprintf(stdout, "Total simulation time: %f seconds\n", total_simulation_time.to_seconds());
  
  // Reportar la utilización del bus
  report_bus_utilization();
}
```

Además, se modificó `simple_bus_main.cpp` para asegurar que el reporte de utilización se mostrara correctamente:

```cpp
int sc_main(int, char **)
{
  simple_bus_test top("top");

  sc_start(10000, SC_NS);
  
  // Forzar una llamada a end_of_simulation
  sc_stop();
  
  // Llamar manualmente a la función de reporte
  printf("\n=== Reporte de utilización del bus ===\n");
  top.bus->report_bus_utilization();

  return 0;
}
```

## Problemas Encontrados y Soluciones

Durante la implementación, se encontraron varios problemas:

### 1. Error en el Tiempo de Cálculo

Inicialmente, se intentó calcular la utilización en `end_of_elaboration()`, lo que no funcionaba ya que este método se ejecuta antes de que comience la simulación, cuando el tiempo es cero.

**Solución**: Mover el cálculo a `end_of_simulation()` que se ejecuta después de que termina la simulación, cuando los valores de tiempo son válidos.

### 2. Errores del Linker

Al modificar `simple_bus.cpp`, inicialmente se omitieron las implementaciones originales de varios métodos, lo que causó errores del linker.

**Solución**: Mantener todas las implementaciones originales y solo añadir/modificar los métodos relevantes para la funcionalidad de utilización.

### 3. Registro del Callback para Fin de Simulación

Hubo problemas con el registro del callback para `end_of_simulation()`.

**Solución**: Simplificar la implementación y añadir una llamada explícita a `report_bus_utilization()` desde `sc_main()` como respaldo.

## Resultados y Validación

Después de implementar las modificaciones, el sistema mostró correctamente la utilización del bus:

```
Info: /OSCI/SystemC: Simulation stopped by user.
=== Fin de la simulación: Ejecutando end_of_simulation() ===
Total simulation time: 0.000010 seconds
====================================
 Nivel de utilización del bus: 5.58%
====================================
```

El valor de 5.58% indica que, durante la simulación de 10,000 ns, el bus estuvo activo aproximadamente 558 ns, o el 5.58% del tiempo total. Esta métrica proporciona información valiosa sobre la eficiencia del uso del bus en el sistema.

## Conclusiones

La implementación de la funcionalidad de medición de utilización del bus proporciona una herramienta útil para evaluar el rendimiento del sistema. Esta métrica permite:

1. Identificar si el bus está siendo subutilizado o sobreutilizado
2. Evaluar el impacto de diferentes algoritmos de arbitraje en la eficiencia del sistema
3. Optimizar la configuración del sistema para maximizar el rendimiento

Esta información es invaluable para el diseño y optimización de sistemas basados en buses, especialmente en aplicaciones donde el rendimiento y la latencia son críticos.

## Posibles Mejoras Futuras

Algunas posibles mejoras para esta funcionalidad incluyen:

1. Medir la utilización por maestro, para identificar qué componentes utilizan más el bus
2. Calcular estadísticas adicionales como tiempo promedio de espera, número de solicitudes por unidad de tiempo, etc.
3. Implementar visualización gráfica de la utilización del bus a lo largo del tiempo
4. Añadir soporte para guardar los resultados en un archivo para análisis posterior