# Compilador a usar
CC=g++

# Flags de compilación, incluyendo los obtenidos de root-config
CFLAGS=$(shell root-config --cflags --glibs)

# Nombre del archivo ejecutable
EXECUTABLE=plot

# Archivos fuente
SOURCES=main.cpp financialFunctions.cpp

# Regla por defecto
all: $(EXECUTABLE)

# Cómo construir el ejecutable
$(EXECUTABLE): $(SOURCES)
	$(CC) $(SOURCES) -o $(EXECUTABLE) $(CFLAGS)

# Regla para limpiar archivos compilados
clean:
	rm -f $(EXECUTABLE)

# Regla para ejecutar el programa y luego limpiar
run: all
	./$(EXECUTABLE)
	rm -f $(EXECUTABLE)