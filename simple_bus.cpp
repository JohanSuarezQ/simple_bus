/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************
 
  simple_bus.cpp : The bus.

                   The main_action process is active at falling clock edge.

       		   Handling of the requests. A request can result in 
		   different requests to slaves. These are atomic requests 
		   and cannot be interrupted. A slave can take several 
		   cycles to complete: each time the request has to be 
		   re-issued to the slave. Once the slave transaction 
		   is completed, the m_current_request is cleared, and 
		   the request form is updated (address+=4, data++). 

		   When m_current_request is clear, the next request is
		   selected. This can be the same one, if the transfer is
		   not completed (burst-mode), or it can be a request with
		   a higher priority. Intrusion to a non-locked burst-mode
		   transaction is possible. 

		   When a transaction sets a lock, then the corresponding
		   field in the request is set to SIMPLE_BUS_LOCK_SET. If
		   the locked transaction is granted by the arbiter for the
		   first time, the lock is set to SIMPLE_BUS_LOCK_GRANTED. 
		   At the end of the transaction, the lock is set to 
		   SIMPLE_BUS_LOCK_SET. If now a new locked request is made,
		   with the same priority, the status of the lock is set
		   to SIMPLE_BUS_LOCK_GRANTED, and the arbiter will pick
		   this transaction to be the best. If the locked trans-
		   action was not selected by the arbiter in the first 
		   round (request with higher priority preceeded), then the
		   lock is not set. After the completion of the transaction,
		   the lock is set from SIMPLE_BUS_LOCK_SET (set during the
		   bus-interface function), to SIMPLE_BUS_LOCK_NO. 
		   
		   The bus is derived from the following interfaces, and
		   contains the implementation of these: 
		   - blocking : burst_read/burst_write
		   - non-blocking : read/write/get_status
		   - direct : direct_read/direct_write
 
  Original Author: Ric Hilderink, Synopsys, Inc., 2001-10-11
 
 *****************************************************************************/

 #include "simple_bus.h"

 // Definición de variables globales para medir el uso del bus
 sc_time bus_active_time = SC_ZERO_TIME;
 sc_time total_simulation_time = SC_ZERO_TIME;
 sc_time last_time_stamp = SC_ZERO_TIME;

 // Agrega estas líneas junto a las otras variables globales al principio del archivo
unsigned long total_bytes_transferred = 0;
unsigned int total_transactions = 0;
unsigned int read_transactions = 0;
unsigned int write_transactions = 0;

sc_time total_read_time = SC_ZERO_TIME;   // Tiempo total usado en lecturas
sc_time total_write_time = SC_ZERO_TIME;  // Tiempo total usado en escrituras
sc_time current_transfer_start_time;      // Tiempo de inicio de la transferencia actual

unsigned int read_transfers_started = 0;   // Número de transferencias de lectura iniciadas
unsigned int write_transfers_started = 0;  // Número de transferencias de escritura iniciadas
 
 //----------------------------------------------------------------------------
 //-- SystemC callback methods
 //----------------------------------------------------------------------------
 void simple_bus::end_of_elaboration()
 {
   // perform a static check for overlapping memory areas of the slaves
   bool no_overlap;
   for (int i = 1; i < slave_port.size(); ++i) {
     simple_bus_slave_if *slave1 = slave_port[i];
     for (int j = 0; j < i; ++j) {
       simple_bus_slave_if *slave2 = slave_port[j]; 
       no_overlap = ( slave1->end_address() < slave2->start_address() ) ||
                ( slave1->start_address() > slave2->end_address() );
       if ( !no_overlap ) {
         sb_fprintf(stdout,"Error: overlapping address spaces of 2 slaves : \n");
         sb_fprintf(stdout,"slave %i : %0X..%0X\n",i,slave1->start_address(),slave1->end_address()); 
         sb_fprintf(stdout,"slave %i : %0X..%0X\n",j,slave2->start_address(),slave2->end_address());
         exit(0);
       }
     }
   }
 }
 
 void simple_bus::end_of_simulation()
{
  // Registrar el tiempo total de simulación
  total_simulation_time = sc_time_stamp();
  
  // Imprimir mensaje para depuración
  printf("=== Fin de la simulación: Ejecutando end_of_simulation() ===\n");
  printf("Total simulation time: %f seconds\n", total_simulation_time.to_seconds());
  
  // Reportar la utilización del bus
  report_bus_utilization();
}
 
 //----------------------------------------------------------------------------
 //-- process
 //----------------------------------------------------------------------------
 void simple_bus::main_action()
 {
  // Añadir un pequeño retraso artificial
   // Reducir el divisor si quieres más transacciones
   static int count = 0;
   if (++count % 1 != 0) return;  // Ejecutar cada ciclo en lugar de cada 2
   // Si el bus está en uso, contar el tiempo activo
   if (m_current_request) {
     sc_time current_time = sc_time_stamp();
     bus_active_time += current_time - last_time_stamp;
   }
   
   // Actualizar el timestamp para el próximo cálculo
   last_time_stamp = sc_time_stamp();
   
   // m_current_request is cleared after the slave is done with a
   // single data transfer. Burst requests require the arbiter to
   // select the request again.
 
   if (!m_current_request)
     m_current_request = get_next_request();
   else
     // monitor slave wait states
     if (m_verbose)
       sb_fprintf(stdout, "%s SLV [%d]\n", sc_time_stamp().to_string().c_str(),
      m_current_request->address);
   if (m_current_request)
     handle_request();
   if (!m_current_request)
     clear_locks();
 }
 
 //----------------------------------------------------------------------------
 //-- direct BUS interface
 //----------------------------------------------------------------------------
 
 bool simple_bus::direct_read(int *data, unsigned int address)
 {
   if (address%4 != 0 ) {// address not word alligned
     sb_fprintf(stdout, "  BUS ERROR --> address %04X not word alligned\n",address);
     return false; 
   }
   // Registrar el inicio de una transferencia de lectura
 read_transfers_started++;  
   simple_bus_slave_if *slave = get_slave(address);
   if (!slave) return false;
   return slave->direct_read(data, address);
 }
 
 
 bool simple_bus::direct_write(int *data, unsigned int address)
 {
   if (address%4 != 0 ) {// address not word alligned
     sb_fprintf(stdout, "  BUS ERROR --> address %04X not word alligned\n",address);
     return false; 
   }
   write_transfers_started++;
   simple_bus_slave_if *slave = get_slave(address);
   if (!slave) return false;
   return slave->direct_write(data, address);
 }
 
 //----------------------------------------------------------------------------
 //-- non-blocking BUS interface
 //----------------------------------------------------------------------------
 
 void simple_bus::read(unsigned int unique_priority, int *data, unsigned int address, bool lock)
{
  if (m_verbose)
    sb_fprintf(stdout, "%s %s : read(%d) @ %x\n",
         sc_time_stamp().to_string().c_str(), name(), unique_priority, address);
  
  // Incrementar contador de transferencias iniciadas
  // read_transfers_started++;
  
  simple_bus_request *request = get_request(unique_priority);

  // abort when the request is still not finished
  sc_assert((request->status == SIMPLE_BUS_OK) ||
      (request->status == SIMPLE_BUS_ERROR));

  request->do_write           = false; // we are reading
  request->address            = address;
  request->end_address        = address;
  request->data               = data;

  if (lock)
    request->lock = (request->lock == SIMPLE_BUS_LOCK_SET) ? 
      SIMPLE_BUS_LOCK_GRANTED : SIMPLE_BUS_LOCK_SET;

  request->status = SIMPLE_BUS_REQUEST;
}
 
void simple_bus::write(unsigned int unique_priority, int *data, unsigned int address, bool lock)
{
  if (m_verbose) 
    sb_fprintf(stdout, "%s %s : write(%d) @ %x\n",
         sc_time_stamp().to_string().c_str(), name(), unique_priority, address);

  // Incrementar contador de transferencias iniciadas
  // write_transfers_started++;

  simple_bus_request *request = get_request(unique_priority);

  // abort when the request is still not finished
  sc_assert((request->status == SIMPLE_BUS_OK) ||
      (request->status == SIMPLE_BUS_ERROR));

  request->do_write           = true; // we are writing
  request->address            = address;
  request->end_address        = address;
  request->data               = data;

  if (lock)
    request->lock = (request->lock == SIMPLE_BUS_LOCK_SET) ?
      SIMPLE_BUS_LOCK_GRANTED : SIMPLE_BUS_LOCK_SET;

  request->status = SIMPLE_BUS_REQUEST;
}
 
 simple_bus_status simple_bus::get_status(unsigned int unique_priority)
 {
   return get_request(unique_priority)->status;
 }
 
 //----------------------------------------------------------------------------
 //-- blocking BUS interface
 //----------------------------------------------------------------------------
 
 simple_bus_status simple_bus::burst_read(unsigned int unique_priority
            , int *data
            , unsigned int start_address
            , unsigned int length
            , bool lock)
 {
   if (m_verbose) 
   {
     sb_fprintf(stdout, "%s %s : burst_read(%d) @ %x\n",
   sc_time_stamp().to_string().c_str(), name(), unique_priority, 
   start_address);
   }
  //  read_transfers_started++;
   simple_bus_request *request = get_request(unique_priority);
 
   request->do_write           = false; // we are reading
   request->address            = start_address;
   request->end_address        = start_address + (length-1)*4;
   request->data               = data;
 
   if (lock)
     request->lock = (request->lock == SIMPLE_BUS_LOCK_SET) ? 
       SIMPLE_BUS_LOCK_GRANTED : SIMPLE_BUS_LOCK_SET;
 
   request->status = SIMPLE_BUS_REQUEST;
 
   wait(request->transfer_done);
   wait(clock->posedge_event());
   return request->status;
 }
 
 simple_bus_status simple_bus::burst_write(unsigned int unique_priority
             , int *data
             , unsigned int start_address
             , unsigned int length
             , bool lock)
 {
   if (m_verbose) 
     sb_fprintf(stdout, "%s %s : burst_write(%d) @ %x\n",
          sc_time_stamp().to_string().c_str(), name(), unique_priority, 
          start_address);
          // write_transfers_started++;
   simple_bus_request *request = get_request(unique_priority);
 
   request->do_write           = true; // we are writing
   request->address            = start_address;
   request->end_address        = start_address + (length-1)*4;
   request->data               = data;
 
   if (lock)
     request->lock = (request->lock == SIMPLE_BUS_LOCK_SET) ? 
       SIMPLE_BUS_LOCK_GRANTED : SIMPLE_BUS_LOCK_SET;
 
   request->status = SIMPLE_BUS_REQUEST;
 
   wait(request->transfer_done);
   wait(clock->posedge_event());
   return request->status;
 }
 
 //----------------------------------------------------------------------------
 //-- BUS methods:
 //
 //     handle_request()   : performs atomic bus-to-slave request
 //     get_request()      : BUS-interface: gets the request form of given 
 //                          priority
 //     get_next_request() : returns a valid request out of the list of 
 //                          pending requests
 //     clear_locks()      : downgrade the lock status of the requests once
 //                          the transfer is done
 //----------------------------------------------------------------------------
 
 void simple_bus::handle_request()
{
  if (m_verbose)
      sb_fprintf(stdout, "%s %s Handle Slave(%d)\n",
      sc_time_stamp().to_string().c_str(), name(), 
      m_current_request->priority);

  // Registrar el tiempo de inicio de la transferencia
  current_transfer_start_time = sc_time_stamp();
 
  m_current_request->status = SIMPLE_BUS_WAIT;
  simple_bus_slave_if *slave = get_slave(m_current_request->address);
 
  if ((m_current_request->address)%4 != 0 ) {// address not word alligned
    sb_fprintf(stdout, "  BUS ERROR --> address %04X not word alligned\n",m_current_request->address);
    m_current_request->status = SIMPLE_BUS_ERROR;
    m_current_request = (simple_bus_request *)0;
    return;
  }
  if (!slave) {
    sb_fprintf(stdout, "  BUS ERROR --> no slave for address %04X \n",m_current_request->address);
    m_current_request->status = SIMPLE_BUS_ERROR;
    m_current_request = (simple_bus_request *)0;
    return;
  }
 
  simple_bus_status slave_status = SIMPLE_BUS_OK;
  if (m_current_request->do_write)
    slave_status = slave->write(m_current_request->data, 
        m_current_request->address);
  else
    slave_status = slave->read(m_current_request->data,
            m_current_request->address);
 
  if (m_verbose)
    sb_fprintf(stdout, "  --> status=(%s)\n", simple_bus_status_str[slave_status]);
 
  switch(slave_status)
  {
    case SIMPLE_BUS_ERROR:
    {
      m_current_request->status = SIMPLE_BUS_ERROR;
      m_current_request->transfer_done.notify();
      m_current_request = (simple_bus_request *)0;
      break;
    }
    case SIMPLE_BUS_OK:
    {
      // Añadir un pequeño retraso artificial para hacer visible la duración
      // Añadir un pequeño retraso artificial para hacer visible la duración
      // next_trigger(SC_ZERO_TIME);  // Usar next_trigger() en lugar de wait()
      
      
      // Calcular la duración de esta transferencia y añadir un valor mínimo
      sc_time transfer_duration;
      if (m_current_request->do_write) {
        transfer_duration = sc_time(1, SC_NS); // 1 nanosegundo para escrituras
      } else {
        transfer_duration = sc_time(0.8, SC_NS); // 0.8 nanosegundos para lecturas
      }
      // Acumular el tiempo según el tipo de transferencia
      if (m_current_request->do_write)
        total_write_time += transfer_duration;
      else
        total_read_time += transfer_duration;
    
      // Código existente para incrementar address y data
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
    }
    case SIMPLE_BUS_WAIT:
    {
      // the slave is still processing: no clearance of the current request
      break;
    }
    default:
      break;
  }
}
 
 simple_bus_slave_if *simple_bus::get_slave(unsigned int address)
 {
   for (int i = 0; i < slave_port.size(); ++i)
     {
       simple_bus_slave_if *slave = slave_port[i];
       if ((slave->start_address() <= address) &&
     (address <= slave->end_address()))
   return slave;
     }
   return (simple_bus_slave_if *)0;		
 }
 
 simple_bus_request * simple_bus::get_request(unsigned int priority)
 {
   simple_bus_request *request = (simple_bus_request *)0;
   for (unsigned int i = 0; i < m_requests.size(); ++i)
     {
       request = m_requests[i];
       if ((request) &&
     (request->priority == priority))
   return request;
     }
   request = new simple_bus_request;
   request->priority = priority;
   m_requests.push_back(request);
   return request;		
 }
 
 simple_bus_request * simple_bus::get_next_request()
 {
   // the slave is done with its action, m_current_request is
   // empty, so go over the bag of request-forms and compose
   // a set of likely requests. Pass it to the arbiter for the
   // final selection
   simple_bus_request_vec Q;
   for (unsigned int i = 0; i < m_requests.size(); ++i)
     {
       simple_bus_request *request = m_requests[i];
       if ((request->status == SIMPLE_BUS_REQUEST) ||
     (request->status == SIMPLE_BUS_WAIT))
   {
     if (m_verbose) 
       sb_fprintf(stdout, "%s %s : request (%d) [%s]\n",
            sc_time_stamp().to_string().c_str(), name(), 
            request->priority, simple_bus_status_str[request->status]);
     Q.push_back(request);
   }
     }
   if (Q.size() > 0)
     return arbiter_port->arbitrate(Q);
   return (simple_bus_request *)0;
 }
 
 void simple_bus::clear_locks()
 {
   for (unsigned int i = 0; i < m_requests.size(); ++i)
     if (m_requests[i]->lock == SIMPLE_BUS_LOCK_GRANTED)
       m_requests[i]->lock = SIMPLE_BUS_LOCK_SET;
     else
       m_requests[i]->lock = SIMPLE_BUS_LOCK_NO;
 }
 
 // Función para reportar la utilización del bus
 void simple_bus::report_bus_utilization()
 {
   if (total_simulation_time > SC_ZERO_TIME) {
     // Reporte de utilización del bus
     double utilization = (bus_active_time.to_seconds() / total_simulation_time.to_seconds()) * 100;
     sb_fprintf(stdout, "====================================\n");
     sb_fprintf(stdout, " Nivel de utilización del bus: %.2f%% \n", utilization);
     sb_fprintf(stdout, "====================================\n");
     
     // Reporte de throughput
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
     
     // Reporte de duración de transferencias
     sb_fprintf(stdout, "\n--- Métricas de Duración de Transferencias ---\n");
     
     // Calcular duración promedio de lecturas
     double avg_read_duration = 0.0;
     if (read_transactions > 0) {
       avg_read_duration = total_read_time.to_seconds() / read_transactions;
       sb_fprintf(stdout, "Duración promedio de lectura: %.12f segundos\n", avg_read_duration);
     } else {
       sb_fprintf(stdout, "No se realizaron transferencias de lectura\n");
     }
     
     // Calcular duración promedio de escrituras
     double avg_write_duration = 0.0;
     if (write_transactions > 0) {
       avg_write_duration = total_write_time.to_seconds() / write_transactions;
       sb_fprintf(stdout, "Duración promedio de escritura: %.12f segundos\n", avg_write_duration);
     } else {
       sb_fprintf(stdout, "No se realizaron transferencias de escritura\n");
     }
     
     // Métricas adicionales de rendimiento - Corregidas para mostrar correctamente la relación
     if (read_transfers_started > 0) {
       double read_completion_ratio = (read_transactions * 100.0) / read_transfers_started;
       sb_fprintf(stdout, "Transacciones de lectura iniciadas: %u, completadas: %u (%.2f%%)\n", 
                 read_transfers_started, read_transactions, read_completion_ratio);
     }
     
     if (write_transfers_started > 0) {
       double write_completion_ratio = (write_transactions * 100.0) / write_transfers_started;
       sb_fprintf(stdout, "Transacciones de escritura iniciadas: %u, completadas: %u (%.2f%%)\n", 
                 write_transfers_started, write_transactions, write_completion_ratio);
     }
     
     // Comparación entre lecturas y escrituras - Con protección extra para división por cero
     if (read_transactions > 0 && write_transactions > 0 && avg_write_duration > 0) {
       sb_fprintf(stdout, "Relación de tiempo lectura/escritura: %.2f\n", 
                 avg_read_duration / avg_write_duration);
     } else {
       sb_fprintf(stdout, "No se puede calcular la relación de tiempo lectura/escritura (datos insuficientes)\n");
     }
     
     sb_fprintf(stdout, "====================================\n");
   } else {
     sb_fprintf(stdout, "ERROR: El tiempo total de simulación es 0. La simulación no avanzó.\n");
   }
 }