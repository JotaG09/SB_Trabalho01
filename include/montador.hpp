#ifndef MONTADOR_HPP
#define MONTADOR_HPP

#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

class Montador
{
private:
    std::unordered_map<std::string, int> tabela_simbolos;
    std::unordered_map<std::string, int> tabela_equ;

    // Variáveis que o compilador acusou não estarem definidas
    bool secao_text_encontrada;
    bool secao_data_encontrada;

    // Funções internas da classe
    void primeiraPassagem(std::ifstream &arquivoEntrada);
    void segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
    void emitirErro(int linha, const std::string &tipo, const std::string &mensagem);

public:
    Montador();
    void executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
};

#endif // MONTADOR_HPP