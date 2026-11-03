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

        int offsetToken = 0;

        // Trata Rótulos
        if (tokens[0].back() == ':')
        {
            std::string rotulo = tokens[0].substr(0, tokens[0].size() - 1);

            if (!Utils::isRotuloValido(rotulo))
            {
                emitirErro(numeroLinha, "LEXICO", "Rotulo invalido '" + rotulo + "'.");
            }
            else if (tabela_simbolos.count(rotulo) || tabela_equ.count(rotulo))
            {
                emitirErro(numeroLinha, "SEMANTICO", "Rotulo '" + rotulo + "' redefinido.");
            }
            else
            {
                // Se a próxima palavra for EQU, salva em tabela_equ e NÃO incremente o LC
                if (tokens.size() > 1 && tokens[1] == "EQU")
                {
                    tabela_equ[rotulo] = std::stoi(tokens[2]);
                    numeroLinha++;
                    continue; // Pula o resto, EQU não ocupa espaço
                }
                else
                {
                    tabela_simbolos[rotulo] = contadorPosicao;
                }
            }
            offsetToken = 1;
            if (tokens.size() == 1)
            {
                numeroLinha++;
                continue;
            }
        }

        std::string operacao = tokens[offsetToken];

        if (operacao == "SECAO")
        {
            if (tokens.size() > offsetToken + 1)
            {
                if (tokens[offsetToken + 1] == "TEXT")
                    secao_text_encontrada = true;
                if (tokens[offsetToken + 1] == "DATA")
                    secao_data_encontrada = true;
            }
            numeroLinha++;
            continue;
        }

        // Não contamos IF no LC (diretiva de controle apenas)
        if (operacao == "IF")
        {
            numeroLinha++;
            continue;
        }

        if (tabela_instrucoes.count(operacao))
        {
            contadorPosicao += tabela_instrucoes.at(operacao).tamanho;
        }
        else if (operacao == "SPACE")
        {
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

        // Verifica erro sintático de vírgula no COPY antes da tokenização remover os espaços
        if (linha.find("COPY") != std::string::npos && linha.find(',') != std::string::npos)
        {
            emitirErro(numeroLinha, "SINTATICO", "Instrucao COPY possui virgula.");
        }

        // Substitui valores de EQU no código antes de tokenizar
        for (const auto &par : tabela_equ)
        {
            size_t pos = linha.find(par.first);
            while (pos != std::string::npos)
            {
                // Substituição simples (pode ser aprimorada para evitar substituir parte de outras palavras)
                linha.replace(pos, par.first.length(), std::to_string(par.second));
                pos = linha.find(par.first, pos + std::to_string(par.second).length());
            }
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
            int valor = std::stoi(tokens[offsetToken + 1]);
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

            // Valida operandos
            int numOperandos = tokens.size() - offsetToken - 1;
            if (numOperandos != (inst.tamanho - 1))
            {
                emitirErro(numeroLinha, "SINTATICO", "Quantidade de operandos errada para " + operacao);
            }

            // Traduz Operandos (Rótulos -> Endereços)
            for (size_t i = offsetToken + 1; i < tokens.size(); ++i)
            {
                std::string operando = tokens[i];
                if (tabela_simbolos.count(operando))
                {
                    codigoObjeto.push_back(tabela_simbolos[operando]);
                }
                else
                {
                    emitirErro(numeroLinha, "SEMANTICO", "Rotulo '" + operando + "' ausente (nao declarado).");
                    codigoObjeto.push_back(0); // Insere 0 para não quebrar a compilação
                }
            }
        }
        else if (operacao == "SPACE")
        {
            int qtd = (tokens.size() > (size_t)(offsetToken + 1)) ? std::stoi(tokens[offsetToken + 1]) : 1;
            for (int i = 0; i < qtd; ++i)
            {
                codigoObjeto.push_back(0); // '00' ou '0' de acordo com a especificação
            }
        }
        else if (operacao == "CONST")
        {
            int valor = std::stoi(tokens[offsetToken + 1]); // Trata negativos e positivos automaticamente
            codigoObjeto.push_back(valor);
        }

        numeroLinha++;
    }

    // Escreve no arquivo de saída (Tudo numa linha só separada por espaços)
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