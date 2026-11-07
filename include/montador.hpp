#ifndef MONTADOR_HPP
#define MONTADOR_HPP

#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <algorithm>

struct ErroMontador
{
    int linha;
    std::string tipo;
    std::string mensagem;

    bool operator<(const ErroMontador &outro) const
    {
        return linha < outro.linha;
    }
};

class Montador
{
private:
    std::unordered_map<std::string, int> tabela_simbolos;
    std::unordered_map<std::string, std::string> tabela_secao_simbolo;
    std::unordered_map<std::string, int> tabela_equ;

    // Estruturas para ligação
    std::vector<std::string> declaracoes_publicas;
    std::unordered_set<std::string> simbolos_externos;
    std::vector<std::pair<std::string, int>> tabela_uso;
    std::unordered_map<std::string, int> tabela_definicoes;
    std::vector<int> mapa_relocacao;

    std::vector<ErroMontador> erros;

    int tamanho_text = 0;
    bool secao_text_encontrada;
    bool secao_data_encontrada;
    bool modo_modulo;

    void primeiraPassagem(std::ifstream &arquivoEntrada);
    void segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
    void emitirErro(int linha, const std::string &tipo, const std::string &mensagem);
    void processarOperando(const std::string &token, int linha, int posAtual, std::vector<int> &codigoObjeto);

public:
    Montador();
    bool executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida);
};

#endif // MONTADOR_HPP