# Variáveis de configuração
CXX = g++
# A flag -Iinclude diz ao compilador para buscar os arquivos .hpp dentro da pasta include
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Arquivos fonte (dentro de src/)
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/montador.cpp

# Mapeia os arquivos de objeto para uma pasta temporária obj/ para não poluir a raiz
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Nome do executável final
TARGET = programa

# Regra principal
all: $(OBJ_DIR) $(TARGET)

# Cria a pasta obj se ela não existir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Como gerar o executável final a partir dos objetos
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Como compilar cada arquivo .cpp de src/ para um arquivo .o em obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para limpar os arquivos temporários gerados
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	rm -f *.exe

# Regra para rodar o projeto diretamente
run:
	./programa "asm-sem-ligacao.txt"