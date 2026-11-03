#ifndef MONTADOR_HPP
#define MONTADOR_HPP

#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

class Montador
{
private:
    std::unordered_map<std::string, int> tabela_simbolos;
    std::unordered_map<std::string, int> tabela_equ;

    // Estruturas de suporte à ligação
    std::vector<std::string> declaracoes_publicas;
    std::unordered_set<std::string> simbolos_externos;
    std::vector<std::pair<std::string, int>> tabela_uso;
    std::unordered_map<std::string, int> tabela_definicoes;
    std::vector<int> mapa_relocacao;

    bool secao_text_encontrada;
    bool secao_data_encontrada;
    bool modo_modulo; // Ativado ao detectar diretivas de ligação (BEGIN/END/PUBLIC/EXTERN)

    void primeiraPassagem(std::ifstream &arquivoEntrada);
    void segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
    void emitirErro(int linha, const std::string &tipo, const std::string &mensagem);

    // Auxiliar para decompor operandos do tipo N1+2 ou ROTULO
    void processarOperando(const std::string &token, int linha, int posAtual, std::vector<int> &codigoObjeto);

public:
    Montador();
    void executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
};

#endif // MONTADOR_HPP