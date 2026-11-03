# Configurações do Compilador
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Fontes dos Binários
MONTADOR_SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/montador.cpp
LIGADOR_SOURCES = $(SRC_DIR)/ligador.cpp $(SRC_DIR)/montador.cpp

# Ficheiros de Objeto e Dependências (.d)
MONTADOR_OBJECTS = $(MONTADOR_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
LIGADOR_OBJECTS = $(LIGADOR_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEPS = $(sort $(MONTADOR_OBJECTS:.o=.d) $(LIGADOR_OBJECTS:.o=.d))

# Executáveis
TARGET_MONTADOR = montador
TARGET_LIGADOR = ligador

.PHONY: all clean run

all: $(OBJ_DIR) $(TARGET_MONTADOR) $(TARGET_LIGADOR)

# Cria a pasta obj caso não exista
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilação do Montador
$(TARGET_MONTADOR): $(MONTADOR_OBJECTS)
	$(CXX) $(CXXFLAGS) $(MONTADOR_OBJECTS) -o $(TARGET_MONTADOR)

# Compilação do Ligador
$(TARGET_LIGADOR): $(LIGADOR_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIGADOR_OBJECTS) -o $(TARGET_LIGADOR)

# Compilação genérica dos ficheiros .cpp para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

# Limpeza dos objetos e binários
clean:
	rm -rf $(OBJ_DIR) $(TARGET_MONTADOR) $(TARGET_LIGADOR) *.obj *.lig *.exe

# Regra para testar o montador monolítico
run: all
	./$(TARGET_MONTADOR) "asm-sem-ligacao.asm"