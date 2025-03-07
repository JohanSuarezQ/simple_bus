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
 
  simple_bus.h : The bus.

		 The bus is derived from the following interfaces, and
	         contains the implementation of these: 
		 - blocking : burst_read/burst_write
		 - non-blocking : read/write/get_status
		 - direct : direct_read/direct_write
 
  Original Author: Ric Hilderink, Synopsys, Inc., 2001-10-11
 
 *****************************************************************************/
 
/*****************************************************************************
 
  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.
 
      Name, Affiliation, Date:
  Description of Modification:
 
 *****************************************************************************/

 #ifndef __simple_bus_h
 #define __simple_bus_h
 
 #include <systemc.h>
 
 #include "simple_bus_types.h"
 #include "simple_bus_request.h"
 #include "simple_bus_direct_if.h"
 #include "simple_bus_non_blocking_if.h"
 #include "simple_bus_blocking_if.h"
 #include "simple_bus_arbiter_if.h"
 #include "simple_bus_slave_if.h"
 
 // Variables para medir el uso del bus
 extern sc_time bus_active_time;
 extern sc_time total_simulation_time;
 extern sc_time last_time_stamp;

 // Variables troghuput
extern unsigned long total_bytes_transferred;
extern unsigned int total_transactions;
extern unsigned int read_transactions;
extern unsigned int write_transactions;

// variables mediciones in out
extern sc_time total_read_time;
extern sc_time total_write_time;
extern sc_time current_transfer_start_time;

extern unsigned int read_transfers_started;
extern unsigned int write_transfers_started;
 
 class simple_bus
   : public simple_bus_direct_if
   , public simple_bus_non_blocking_if
   , public simple_bus_blocking_if
   , public sc_module
 {
 public:
   // ports
   sc_in_clk clock;
   sc_port<simple_bus_arbiter_if> arbiter_port;
   sc_port<simple_bus_slave_if, 0> slave_port;
 
 
// constructor
simple_bus(sc_module_name name_
  , bool verbose = false)
: sc_module(name_)
, m_verbose(verbose)
, m_current_request(0)
{
// process declaration
SC_METHOD(main_action);
dont_initialize();
sensitive << clock.neg();

// Inicializar el tiempo de la última marca
last_time_stamp = SC_ZERO_TIME;
}
 
   // process
   void main_action();
 
   // callbacks del ciclo de vida de SystemC
   void end_of_elaboration();
   void end_of_simulation();
 
   // direct BUS interface
   bool direct_read(int *data, unsigned int address);
   bool direct_write(int *data, unsigned int address);
 
   // non-blocking BUS interface
   void read(unsigned int unique_priority
       , int *data
       , unsigned int address
       , bool lock = false);
   void write(unsigned int unique_priority
        , int *data
        , unsigned int address
        , bool lock = false);
   simple_bus_status get_status(unsigned int unique_priority);
 
   // blocking BUS interface
   simple_bus_status burst_read(unsigned int unique_priority
              , int *data
              , unsigned int start_address
              , unsigned int length = 1
              , bool lock = false);
   simple_bus_status burst_write(unsigned int unique_priority
         , int *data
         , unsigned int start_address
         , unsigned int length = 1
         , bool lock = false);
 
 public:
   // Función para reportar la utilización del bus (ahora pública)
   void report_bus_utilization();

 private:
   void handle_request();
   simple_bus_slave_if * get_slave(unsigned int address);
   simple_bus_request * get_request(unsigned int priority);
   simple_bus_request * get_next_request();
   void clear_locks();
 
 private:
   bool m_verbose;
   simple_bus_request_vec m_requests;
   simple_bus_request *m_current_request;
 };
 // end class simple_bus
 
 #endif