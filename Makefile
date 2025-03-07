# Makefile para compilar y ejecutar simple_bus en WSL (Linux)
# clear && clear && make clean && make && make run

# Rutas de SystemC y variables (configuración que funciona en WSL)
SYSTEMC_HOME = /usr/local/systemc
CXX = g++
CXXFLAGS = -std=c++17 -I$(SYSTEMC_HOME)/include -I.
LDFLAGS = -L$(SYSTEMC_HOME)/lib -lsystemc -Wl,-rpath,$(SYSTEMC_HOME)/lib

# Archivos fuente
SRCS = simple_bus_main.cpp simple_bus.cpp simple_bus_arbiter.cpp simple_bus_tools.cpp \
       simple_bus_types.cpp simple_bus_master_blocking.cpp simple_bus_master_direct.cpp \
       simple_bus_master_non_blocking.cpp

# Nombre del ejecutable
TARGET = simple_bus
WAVEFORM = simple_bus_waveform.vcd

# Regla por defecto
all: $(TARGET)

# Compilación del programa
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDFLAGS)

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET) *.vcd waves/*.vcd

# Regla para ejecutar
run: $(TARGET)
	./$(TARGET)