#include "montador.hpp"
#include "utils.hpp"
#include "tabelas.hpp"
#include <iostream>

Montador::Montador() : secao_text_encontrada(false), secao_data_encontrada(false), modo_modulo(false) {}

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

    // Pré-processamento de EQU
    while (std::getline(arquivoEntrada, linha))
    {
        Utils::removerComentarios(linha);
        Utils::paraMaiusculo(linha);
        std::vector<std::string> tokens = Utils::tokenizar(linha);
        if (tokens.size() >= 3)
        {
            std::string rotulo = "";
            if (tokens[0].back() == ':')
                rotulo = tokens[0].substr(0, tokens[0].size() - 1);
            else if (tokens[1] == "EQU")
                rotulo = tokens[0];

            if (!rotulo.empty() && (tokens[1] == "EQU" || (tokens.size() > 2 && tokens[2] == "EQU")))
            {
                std::string valStr = (tokens[1] == "EQU") ? tokens[2] : tokens[3];
                tabela_equ[rotulo] = std::stoi(valStr);
            }
        }
    }

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

        std::string rotulo = "";
        std::string operacao = "";
        std::vector<std::string> args;
        size_t idx = 0;

        // Identifica rótulo (com ou sem dois pontos para diretivas de módulo)
        if (tokens[idx].back() == ':')
        {
            rotulo = tokens[idx].substr(0, tokens[idx].size() - 1);
            idx++;
        }
        else if (tokens.size() > 1 && (tokens[1] == "BEGIN" || tokens[1] == "EXTERN" || tokens[1] == "PUBLIC" || tokens[1] == "EQU"))
        {
            rotulo = tokens[idx];
            idx++;
        }

        if (idx < tokens.size())
        {
            operacao = tokens[idx];
            idx++;
            while (idx < tokens.size())
            {
                args.push_back(tokens[idx]);
                idx++;
            }
        }
        else
        {
            if (!rotulo.empty())
            {
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
            }
            numeroLinha++;
            continue;
        }

        // Diretivas de Módulo
        if (operacao == "BEGIN")
        {
            modo_modulo = true;
            if (!rotulo.empty())
                tabela_simbolos[rotulo] = contadorPosicao;
            numeroLinha++;
            continue;
        }

        if (operacao == "EXTERN")
        {
            modo_modulo = true;
            std::string extSym = !rotulo.empty() ? rotulo : (!args.empty() ? args[0] : "");
            if (!extSym.empty())
                simbolos_externos.insert(extSym);
            numeroLinha++;
            continue;
        }

        if (operacao == "PUBLIC")
        {
            modo_modulo = true;
            std::string pubSym = !rotulo.empty() ? rotulo : (!args.empty() ? args[0] : "");
            if (!pubSym.empty())
                declaracoes_publicas.push_back(pubSym);
            numeroLinha++;
            continue;
        }

        if (operacao == "END" || operacao == "EQU")
        {
            modo_modulo = true;
            numeroLinha++;
            continue;
        }

        if (!rotulo.empty())
        {
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
        }

        if (operacao == "SECAO" || operacao == "SECTION")
        {
            if (!args.empty())
            {
                if (args[0] == "TEXT")
                    secao_text_encontrada = true;
                if (args[0] == "DATA")
                    secao_data_encontrada = true;
            }
            numeroLinha++;
            continue;
        }

        if (operacao == "IF")
        {
            if (!args.empty())
            {
                std::string arg = args[0];
                int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
                if (valor == 0)
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
            int qtd = (!args.empty()) ? std::stoi(args[0]) : 1;
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

void Montador::processarOperando(const std::string &token, int linha, int posAtual, std::vector<int> &codigoObjeto)
{
    std::string baseSymbol = token;
    int offset = 0;

    size_t posMais = token.find('+');
    if (posMais != std::string::npos)
    {
        baseSymbol = token.substr(0, posMais);
        offset = std::stoi(token.substr(posMais + 1));
    }

    if (simbolos_externos.count(baseSymbol))
    {
        tabela_uso.push_back({baseSymbol, posAtual});
        codigoObjeto.push_back(0 + offset);
        mapa_relocacao.push_back(0);
    }
    else if (tabela_simbolos.count(baseSymbol))
    {
        codigoObjeto.push_back(tabela_simbolos[baseSymbol] + offset);
        mapa_relocacao.push_back(1);
    }
    else if (tabela_equ.count(baseSymbol))
    {
        codigoObjeto.push_back(tabela_equ[baseSymbol] + offset);
        mapa_relocacao.push_back(0);
    }
    else
    {
        try
        {
            int valor = std::stoi(token);
            codigoObjeto.push_back(valor);
            mapa_relocacao.push_back(0);
        }
        catch (...)
        {
            emitirErro(linha, "SEMANTICO", "Rotulo '" + baseSymbol + "' ausente.");
            codigoObjeto.push_back(0);
            mapa_relocacao.push_back(0);
        }
    }
}

void Montador::segundaPassagem(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    std::string linha;
    int numeroLinha = 1;
    bool ignorarProximaLinha = false;
    std::vector<int> codigoObjeto;

    for (const auto &pub : declaracoes_publicas)
    {
        if (tabela_simbolos.count(pub))
        {
            tabela_definicoes[pub] = tabela_simbolos[pub];
        }
        else
        {
            emitirErro(0, "SEMANTICO", "Simbolo publico '" + pub + "' nao foi definido no modulo.");
        }
    }

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

        std::string rotulo = "";
        std::string operacao = "";
        std::vector<std::string> args;
        size_t idx = 0;

        if (tokens[idx].back() == ':')
        {
            rotulo = tokens[idx].substr(0, tokens[idx].size() - 1);
            idx++;
        }
        else if (tokens.size() > 1 && (tokens[1] == "BEGIN" || tokens[1] == "EXTERN" || tokens[1] == "PUBLIC" || tokens[1] == "EQU"))
        {
            rotulo = tokens[idx];
            idx++;
        }

        if (idx < tokens.size())
        {
            operacao = tokens[idx];
            idx++;
            while (idx < tokens.size())
            {
                args.push_back(tokens[idx]);
                idx++;
            }
        }
        else
        {
            numeroLinha++;
            continue;
        }

        if (operacao == "SECAO" || operacao == "SECTION" || operacao == "EQU" ||
            operacao == "BEGIN" || operacao == "END" || operacao == "PUBLIC" || operacao == "EXTERN")
        {
            numeroLinha++;
            continue;
        }

        if (operacao == "IF")
        {
            if (!args.empty())
            {
                std::string arg = args[0];
                int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
                if (valor == 0)
                    ignorarProximaLinha = true;
            }
            numeroLinha++;
            continue;
        }

        if (tabela_instrucoes.count(operacao))
        {
            Instrucao inst = tabela_instrucoes.at(operacao);
            codigoObjeto.push_back(inst.opcode);
            mapa_relocacao.push_back(0);

            for (size_t i = 0; i < args.size(); ++i)
            {
                int posAtual = codigoObjeto.size();
                processarOperando(args[i], numeroLinha, posAtual, codigoObjeto);
            }
        }
        else if (operacao == "SPACE")
        {
            int qtd = (!args.empty()) ? std::stoi(args[0]) : 1;
            for (int i = 0; i < qtd; ++i)
            {
                codigoObjeto.push_back(0);
                mapa_relocacao.push_back(0);
            }
        }
        else if (operacao == "CONST")
        {
            if (!args.empty())
            {
                std::string arg = args[0];
                int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
                codigoObjeto.push_back(valor);
                mapa_relocacao.push_back(0);
            }
        }

        numeroLinha++;
    }

    if (modo_modulo)
    {
        arquivoSaida << "USO:";
        for (const auto &par : tabela_uso)
        {
            arquivoSaida << " " << par.first << " " << par.second;
        }
        arquivoSaida << "\n";

        arquivoSaida << "DEF:";
        for (const auto &par : tabela_definicoes)
        {
            arquivoSaida << " " << par.first << " " << par.second;
        }
        arquivoSaida << "\n";

        arquivoSaida << "RELOC:";
        for (size_t i = 0; i < mapa_relocacao.size(); ++i)
        {
            arquivoSaida << " " << mapa_relocacao[i];
        }
        arquivoSaida << "\n";

        arquivoSaida << "CODE:";
        for (size_t i = 0; i < codigoObjeto.size(); ++i)
        {
            arquivoSaida << " " << codigoObjeto[i];
        }
        arquivoSaida << "\n";
    }
    else
    {
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
}