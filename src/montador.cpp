#include "montador.hpp"
#include "utils.hpp"
#include "tabelas.hpp"
#include <iostream>

Montador::Montador() : secao_text_encontrada(false), secao_data_encontrada(false) {}

void Montador::emitirErro(int linha, const std::string &tipo, const std::string &mensagem)
{
    std::cerr << "ERRO " << tipo << " (Linha " << linha << "): " << mensagem << std::endl;
}

void Montador::executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    primeiraPassagem(arquivoEntrada);

    if (!secao_text_encontrada)
    {
        emitirErro(0, "SEMANTICO", "Secao TEXT ausente no programa.");
    }

    arquivoEntrada.clear();
    arquivoEntrada.seekg(0, std::ios::beg);

    segundaPassagem(arquivoEntrada, arquivoSaida);
}

void Montador::primeiraPassagem(std::ifstream &arquivoEntrada)
{
    std::string linha;
    int contadorPosicao = 0;
    int numeroLinha = 1;
    bool ignorarProximaLinha = false;

    // Etapa de Pré-processamento de EQU
    while (std::getline(arquivoEntrada, linha))
    {
        Utils::removerComentarios(linha);
        Utils::paraMaiusculo(linha);
        std::vector<std::string> tokens = Utils::tokenizar(linha);
        if (tokens.size() >= 3 && tokens[0].back() == ':' && tokens[1] == "EQU")
        {
            std::string rotulo = tokens[0].substr(0, tokens[0].size() - 1);
            tabela_equ[rotulo] = std::stoi(tokens[2]);
        }
    }

    // Reset do ponteiro para a Primeira Passagem real
    arquivoEntrada.clear();
    arquivoEntrada.seekg(0, std::ios::beg);

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

        if (ignorarProximaLinha)
        {
            ignorarProximaLinha = false;
            numeroLinha++;
            continue;
        }

        int offsetToken = 0;

        if (tokens[0].back() == ':')
        {
            std::string rotulo = tokens[0].substr(0, tokens[0].size() - 1);

            if (tokens.size() > 1 && tokens[1] == "EQU")
            {
                numeroLinha++;
                continue; // EQU já processado
            }

            if (!Utils::isRotuloValido(rotulo))
            {
                emitirErro(numeroLinha, "LEXICO", "Rotulo invalido '" + rotulo + "'.");
            }
            else if (tabela_simbolos.count(rotulo))
            {
                emitirErro(numeroLinha, "SEMANTICO", "Rotulo '" + rotulo + "' redefinido.");
            }
            else
            {
                tabela_simbolos[rotulo] = contadorPosicao;
            }
            offsetToken = 1;
            if (tokens.size() == 1)
            {
                numeroLinha++;
                continue;
            }
        }

        std::string operacao = tokens[offsetToken];

        if (operacao == "SECAO" || operacao == "SECTION")
        {
            if (tokens.size() > (size_t)(offsetToken + 1))
            {
                if (tokens[offsetToken + 1] == "TEXT")
                    secao_text_encontrada = true;
                if (tokens[offsetToken + 1] == "DATA")
                    secao_data_encontrada = true;
            }
            numeroLinha++;
            continue;
        }

        if (operacao == "IF")
        {
            std::string arg = tokens[offsetToken + 1];
            int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
            if (valor == 0)
            {
                ignorarProximaLinha = true;
            }
            numeroLinha++;
            continue;
        }

        if (tabela_instrucoes.count(operacao))
        {
            contadorPosicao += tabela_instrucoes.at(operacao).tamanho;
        }
        else if (operacao == "SPACE")
        {
            int qtd = (tokens.size() > (size_t)(offsetToken + 1)) ? std::stoi(tokens[offsetToken + 1]) : 1;
            contadorPosicao += qtd;
        }
        else if (operacao == "CONST")
        {
            contadorPosicao += 1;
        }
        else
        {
            emitirErro(numeroLinha, "SINTATICO", "Instrucao ou diretiva invalida '" + operacao + "'.");
        }

        numeroLinha++;
    }
}

void Montador::segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    std::string linha;
    int numeroLinha = 1;
    bool ignorarProximaLinha = false;
    std::vector<int> codigoObjeto;

    while (std::getline(arquivoEntrada, linha))
    {
        Utils::removerComentarios(linha);
        Utils::paraMaiusculo(linha);

        if (linha.find("COPY") != std::string::npos && linha.find(',') != std::string::npos)
        {
            emitirErro(numeroLinha, "SINTATICO", "Instrucao COPY possui virgula.");
        }

        std::vector<std::string> tokens = Utils::tokenizar(linha);

        if (tokens.empty())
        {
            numeroLinha++;
            continue;
        }

        if (ignorarProximaLinha)
        {
            ignorarProximaLinha = false;
            numeroLinha++;
            continue;
        }

        int offsetToken = 0;
        if (tokens[0].back() == ':')
        {
            offsetToken = 1;
            if (tokens.size() == 1)
            {
                numeroLinha++;
                continue;
            }
        }

        std::string operacao = tokens[offsetToken];

        if (operacao == "SECAO" || operacao == "EQU")
        {
            numeroLinha++;
            continue;
        }

        if (operacao == "IF")
        {
            std::string arg = tokens[offsetToken + 1];
            int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
            if (valor == 0)
            {
                ignorarProximaLinha = true;
            }
            numeroLinha++;
            continue;
        }

        if (tabela_instrucoes.count(operacao))
        {
            Instrucao inst = tabela_instrucoes.at(operacao);
            codigoObjeto.push_back(inst.opcode);

            for (size_t i = offsetToken + 1; i < tokens.size(); ++i)
            {
                std::string operando = tokens[i];
                if (tabela_simbolos.count(operando))
                {
                    codigoObjeto.push_back(tabela_simbolos[operando]);
                }
                else if (tabela_equ.count(operando))
                {
                    codigoObjeto.push_back(tabela_equ[operando]);
                }
                else
                {
                    emitirErro(numeroLinha, "SEMANTICO", "Rotulo '" + operando + "' ausente.");
                    codigoObjeto.push_back(0);
                }
            }
        }
        else if (operacao == "SPACE")
        {
            int qtd = (tokens.size() > (size_t)(offsetToken + 1)) ? std::stoi(tokens[offsetToken + 1]) : 1;
            for (int i = 0; i < qtd; ++i)
            {
                codigoObjeto.push_back(0);
            }
        }
        else if (operacao == "CONST")
        {
            std::string arg = tokens[offsetToken + 1];
            int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
            codigoObjeto.push_back(valor);
        }

        numeroLinha++;
    }

    for (size_t i = 0; i < codigoObjeto.size(); ++i)
    {
        arquivoSaida << codigoObjeto[i];
        if (i < codigoObjeto.size() - 1)
        {
            arquivoSaida << " ";
        }
    }
    arquivoSaida << "\n";
}