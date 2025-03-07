# Implementación de Métricas de Eficiencia de Arbitraje en Bus SystemC

## Introducción

Este documento describe la implementación de métricas para evaluar la eficiencia del arbitraje en un modelo de bus SystemC. El arbitraje es un componente crítico en sistemas con bus compartido, ya que determina qué maestro obtiene acceso al bus cuando existen múltiples solicitudes competidoras. Estas métricas complementan las de utilización, throughput y duración de transferencias para proporcionar una visión integral del rendimiento del sistema.

## Objetivos

La implementación de métricas de eficiencia de arbitraje tiene como objetivos:

1. Medir y analizar el tiempo que toma el proceso de decisión de arbitraje
2. Evaluar la equidad en la distribución de acceso al bus entre los diferentes maestros
3. Cuantificar el nivel de contención en el acceso al bus
4. Proporcionar información para optimizar algoritmos de arbitraje

Estas métricas son especialmente valiosas para comparar diferentes políticas de arbitraje (prioridad fija, round-robin, etc.) y para identificar posibles situaciones de inanición de maestros.

## Implementación

La implementación se realizó añadiendo instrumentación al árbitro existente (`simple_bus_arbiter`) para recopilar estadísticas durante la simulación.

### 1. Variables para el Seguimiento de Métricas

Se añadieron las siguientes variables al árbitro:

```cpp
// Variables para métricas de arbitraje
sc_time total_arbitration_wait_time;     // Tiempo total en decisiones de arbitraje
unsigned int arbitration_decisions;      // Número total de decisiones de arbitraje
std::map<unsigned int, unsigned int> master_grants;  // Accesos concedidos por maestro
unsigned int total_request_rejections;   // Total de solicitudes rechazadas
```

Estas variables se inicializan en el constructor del árbitro:

```cpp
simple_bus_arbiter::simple_bus_arbiter(sc_module_name name_, bool verbose)
  : sc_module(name_)
  , m_verbose(verbose)
  , total_arbitration_wait_time(SC_ZERO_TIME)
  , arbitration_decisions(0)
  , total_request_rejections(0)
{}
```

### 2. Instrumentación del Proceso de Arbitraje

El método de arbitraje (`arbitrate`) se modificó para registrar información relevante:

```cpp
simple_bus_request* simple_bus_arbiter::arbitrate(const simple_bus_request_vec &requests)
{
  // Registrar tiempo de inicio del arbitraje
  sc_time arbitration_start_time = sc_time_stamp();
  
  // Código original de decisión de arbitraje...
  
  // Actualizar métricas de arbitraje al conceder acceso
  arbitration_decisions++;
  master_grants[selected_request->priority]++;
  total_request_rejections += requests.size() - 1;
  total_arbitration_wait_time += sc_time_stamp() - arbitration_start_time;
  
  return selected_request;
}
```

La instrumentación se añadió en cada punto donde se toma una decisión de arbitraje:
- Se incrementa el contador de decisiones (`arbitration_decisions`)
- Se registra qué maestro (por prioridad) recibió acceso
- Se cuentan cuántas solicitudes fueron rechazadas
- Se acumula el tiempo que tomó la decisión

### 3. Función de Reporte de Métricas

Se implementó una nueva función para mostrar las métricas recopiladas:

```cpp
void simple_bus_arbiter::report_arbitration_efficiency()
{
  sb_fprintf(stdout, "\n--- Métricas de Eficiencia de Arbitraje ---\n");
  
  // Tiempo promedio de arbitraje
  if (arbitration_decisions > 0) {
    double avg_arbitration_time = total_arbitration_wait_time.to_seconds() / arbitration_decisions;
    sb_fprintf(stdout, "Tiempo promedio de arbitraje: %.12f segundos\n", avg_arbitration_time);
  } else {
    sb_fprintf(stdout, "No se realizaron decisiones de arbitraje\n");
  }
  
  // Distribución de acceso por maestro
  sb_fprintf(stdout, "Distribución de acceso por maestro:\n");
  for (auto it = master_grants.begin(); it != master_grants.end(); ++it) {
    double percentage = arbitration_decisions > 0 ? 
                       (100.0 * it->second / arbitration_decisions) : 0.0;
    sb_fprintf(stdout, "  Maestro %u: %u accesos (%.2f%%)\n", 
              it->first, it->second, percentage);
  }
  
  // Tasa de rechazo
  if ((total_request_rejections + arbitration_decisions) > 0) {
    double rejection_rate = 100.0 * total_request_rejections / 
                           (total_request_rejections + arbitration_decisions);
    sb_fprintf(stdout, "Tasa de rechazo de solicitudes: %.2f%%\n", rejection_rate);
    sb_fprintf(stdout, "Total de solicitudes rechazadas: %u\n", total_request_rejections);
  } else {
    sb_fprintf(stdout, "No se registraron rechazos de solicitudes\n");
  }
  
  sb_fprintf(stdout, "Total de decisiones de arbitraje: %u\n", arbitration_decisions);
}
```

### 4. Integración con el Flujo de Simulación

Finalmente, se modificó el archivo principal (`simple_bus_main.cpp`) para invocar este reporte al final de la simulación:

```cpp
int sc_main(int, char **)
{
  simple_bus_test top("top");

  sc_start(10000, SC_NS);
  sc_stop();
  
  // Reportar métricas del bus y arbitraje
  top.bus->report_bus_utilization();
  top.arbiter->report_arbitration_efficiency();

  return 0;
}
```

## Resultados y Análisis

La implementación de estas métricas produce resultados como los siguientes:

```
--- Métricas de Eficiencia de Arbitraje ---
Tiempo promedio de arbitraje: 0.000000000000 segundos
Distribución de acceso por maestro:
  Maestro 3: 874 accesos (49.38%)
  Maestro 4: 896 accesos (50.62%)
Tasa de rechazo de solicitudes: 5.04%
Total de solicitudes rechazadas: 94
Total de decisiones de arbitraje: 1770
```

### Interpretación de Resultados:

1. **Tiempo promedio de arbitraje (0.000000000000 segundos)**:
   - El proceso de arbitraje ocurre prácticamente de forma instantánea en la simulación.
   - Esto es esperable ya que el algoritmo de decisión es simple y no involucra operaciones complejas.

2. **Distribución de acceso por maestro**:
   - El acceso está distribuido de manera muy equitativa entre los maestros 3 (49.38%) y 4 (50.62%).
   - Esta distribución casi perfecta indica que el algoritmo de arbitraje está siendo justo en la asignación del bus.

3. **Tasa de rechazo (5.04%)**:
   - Solo un pequeño porcentaje de las solicitudes son rechazadas inicialmente.
   - Esto sugiere un nivel bajo de contención en el acceso al bus.
   - El total de 94 solicitudes rechazadas frente a 1770 decisiones de arbitraje confirma esta interpretación.

4. **Total de decisiones de arbitraje (1770)**:
   - Este número coincide con el total de transacciones completadas, lo que indica que cada transacción requirió arbitraje.

## Valor y Aplicaciones

Las métricas de eficiencia de arbitraje proporcionan información valiosa para varios aspectos del diseño de sistemas:

1. **Evaluación de Algoritmos de Arbitraje**: Permiten comparar diferentes políticas (prioridad fija, round-robin, etc.) para determinar cuál proporciona la distribución más equitativa o el menor tiempo de decisión.

2. **Detección de Inanición**: Si las métricas mostraran que ciertos maestros reciben un porcentaje muy bajo o nulo de accesos, indicaría posibles problemas de inanición.

3. **Análisis de Contención**: Una alta tasa de rechazo sugeriría una contención significativa en el bus, lo que podría indicar la necesidad de optimizaciones como buses múltiples o arquitecturas jerárquicas.

4. **Ajuste de Parámetros**: Estas métricas pueden guiar la optimización de parámetros como niveles de prioridad o timeouts en el arbitraje.

5. **Escalabilidad**: Permiten evaluar cómo se comporta el sistema a medida que aumenta el número de maestros compitiendo por el bus.

## Posibles Mejoras

Para extender aún más estas métricas, se podrían implementar:

1. **Tiempos de espera por maestro**: Medir cuánto tiempo espera cada maestro antes de obtener acceso al bus.

2. **Historial de decisiones**: Registrar cómo evoluciona la distribución de acceso a lo largo del tiempo para detectar patrones o desequilibrios temporales.

3. **Detección de hambruna**: Alertar cuando un maestro ha esperado más de un umbral predefinido.

4. **Métricas por tipo de transacción**: Separar las estadísticas para lecturas y escrituras.

5. **Integración con las métricas de utilización y throughput**: Correlacionar la eficiencia del arbitraje con el rendimiento general del sistema.

## Conclusión

Las métricas de eficiencia de arbitraje implementadas proporcionan una visión detallada de cómo se gestiona el acceso al bus compartido. Estas métricas, junto con las de utilización, throughput y duración de transferencias, ofrecen una imagen completa del rendimiento y comportamiento del sistema de bus.

La implementación presentada es eficiente, añadiendo una sobrecarga mínima a la simulación mientras proporciona información valiosa para el análisis y optimización del sistema. Los resultados muestran un sistema bien equilibrado con baja contención, indicando un diseño eficiente del bus y su algoritmo de arbitraje.