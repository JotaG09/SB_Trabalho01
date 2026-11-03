#ifndef MONTADOR_HPP
#define MONTADOR_HPP

#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

class Montador
{
private:
    // Tabela de Símbolos (armazena o Rótulo e seu Endereço correspondente)
    std::unordered_map<std::string, int> tabela_simbolos;

    // Tabela de Definições de EQU (Rótulo -> Valor)
    std::unordered_map<std::string, std::string> tabela_equ;

    void primeiraPassagem(std::ifstream &arquivoEntrada);
    void segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);

public:
    Montador();
    void executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
};

#endif // MONTADOR_HPP