#include "montador.hpp"
#include "utils.hpp"
#include "tabelas.hpp"
#include <iostream>

Montador::Montador() {}

void Montador::executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    std::cout << "Iniciando Primeira Passagem..." << std::endl;
    primeiraPassagem(arquivoEntrada);

    // Retorna o ponteiro de leitura do arquivo para o início antes da segunda passagem
    arquivoEntrada.clear();
    arquivoEntrada.seekg(0, std::ios::beg);

    std::cout << "Iniciando Segunda Passagem..." << std::endl;
    segundaPassagem(arquivoEntrada, arquivoSaida);
}

void Montador::primeiraPassagem(std::ifstream &arquivoEntrada)
{
    std::string linha;
    int contadorPosicao = 0; // LC (Location Counter)
    int numeroLinha = 1;

    while (std::getline(arquivoEntrada, linha))
    {
        Utils::removerComentarios(linha);
        Utils::paraMaiusculo(linha);
        std::vector<std::string> tokens = Utils::tokenizar(linha);

        if (tokens.empty())
        {
            numeroLinha++;
            continue;
        }

        // Processamento de rótulos (identificados pelo ':')
        int offsetToken = 0;
        if (tokens[0].back() == ':')
        {
            std::string rotulo = tokens[0].substr(0, tokens[0].size() - 1); // Remove o ':'

            // Verifica se o rótulo já existe (Erro Semântico de re-declaração)
            if (tabela_simbolos.find(rotulo) != tabela_simbolos.end())
            {
                std::cerr << "ERRO SEMANTICO (Linha " << numeroLinha << "): Rotulo '" << rotulo << "' redefinido." << std::endl;
            }
            else
            {
                tabela_simbolos[rotulo] = contadorPosicao;
            }

            offsetToken = 1; // Pula o rótulo para analisar a instrução que vem na frente

            // Tratamento: Rótulo sozinho na linha com a instrução na linha de baixo
            if (tokens.size() == 1)
            {
                numeroLinha++;
                continue;
            }
        }

        std::string operacao = tokens[offsetToken];

        // Ignora pseudo-instruções de marcação de seção no incremento do LC
        if (operacao == "SECAO")
        {
            numeroLinha++;
            continue;
        }

        // Atualização do LC baseado no tamanho da instrução ou diretiva
        if (tabela_instrucoes.find(operacao) != tabela_instrucoes.end())
        {
            contadorPosicao += tabela_instrucoes.at(operacao).tamanho;
        }
        else if (operacao == "SPACE")
        {
            // SPACE pode receber UM argumento, alocando mais espaços
            if (tokens.size() > (size_t)(offsetToken + 1))
            {
                contadorPosicao += std::stoi(tokens[offsetToken + 1]);
            }
            else
            {
                contadorPosicao += 1;
            }
        }
        else if (operacao == "CONST")
        {
            contadorPosicao += 1;
        }

        numeroLinha++;
    }
}

void Montador::segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    // TODO: Utilizar arquivoSaida aqui para gerar o .OBJ em linha única
    // Isso resolverá o warning do compilador.
    arquivoSaida << "codigo objeto provisorio";
}