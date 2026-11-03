# Variáveis de configuração
CXX = g++
# Flags -MMD -MP geram automaticamente arquivos .d para rastrear alterações em arquivos .hpp
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Arquivos fonte (dentro de src/)
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/montador.cpp

# Mapeia os arquivos de objeto e dependências para a pasta obj/
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

# Nome do executável final
TARGET = programa

# Declara regras que não geram arquivos diretamente
.PHONY: all clean run

# Regra principal
all: $(OBJ_DIR) $(TARGET)

# Cria a pasta obj se ela não existir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Gera o executável final a partir dos objetos
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Compila cada arquivo .cpp para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Inclui os arquivos de dependência auto-gerados (.d)
-include $(DEPS)

# Limpa os executáveis, a pasta obj e os arquivos .obj gerados pelos testes
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	rm -f *.exe *.obj

# Compila e roda o teste principal automaticamente
run: all
	./$(TARGET) "asm-sem-ligacao.txt"