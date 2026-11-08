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

# Detecção Automática de Sistema Operacional (Windows vs Linux/WSL)
ifeq ($(OS),Windows_NT)
    TARGET_MONTADOR = montador.exe
    TARGET_LIGADOR = ligador.exe
    CLEAN_CMD = if exist obj rmdir /s /q obj & if exist montador.exe del /f /q montador.exe & if exist ligador.exe del /f /q ligador.exe & if exist exemplos\*.obj del /f /q exemplos\*.obj & if exist exemplos\*.lig del /f /q exemplos\*.lig
    MKDIR_CMD = if not exist obj mkdir obj
else
    TARGET_MONTADOR = montador
    TARGET_LIGADOR = ligador
    CLEAN_CMD = rm -rf obj montador ligador exemplos/*.obj exemplos/*.lig
    MKDIR_CMD = mkdir -p obj
endif

.PHONY: all clean run

all: obj $(TARGET_MONTADOR) $(TARGET_LIGADOR)

# Cria a pasta obj caso não exista (Atenção: use a tecla TAB no início da linha abaixo)
obj:
	$(MKDIR_CMD)

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

# Limpeza universal (compatível com Windows e Linux)
clean:
	@$(CLEAN_CMD)

# Regra para testar o montador
run: all
	./$(TARGET_MONTADOR) "exemplos/asm-sem-ligacao.asm"