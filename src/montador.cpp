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

    // Pré-processamento de diretivas EQU
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

        // Verificação de rótulos
        if (tokens[0].back() == ':')
        {
            std::string rotulo = tokens[0].substr(0, tokens[0].size() - 1);

            if (tokens.size() > 1 && tokens[1] == "EQU")
            {
                numeroLinha++;
                continue;
            }

            if (tokens.size() > 1 && tokens[1] == "BEGIN")
            {
                modo_modulo = true;
                numeroLinha++;
                continue;
            }

            if (tokens.size() > 1 && tokens[1] == "EXTERN")
            {
                modo_modulo = true;
                simbolos_externos.insert(rotulo);
                numeroLinha++;
                continue;
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

        if (operacao == "BEGIN" || operacao == "END")
        {
            modo_modulo = true;
            numeroLinha++;
            continue;
        }

        if (operacao == "PUBLIC")
        {
            modo_modulo = true;
            if (tokens.size() > (size_t)(offsetToken + 1))
            {
                declaracoes_publicas.push_back(tokens[offsetToken + 1]);
            }
            numeroLinha++;
            continue;
        }

        if (operacao == "EXTERN")
        {
            modo_modulo = true;
            numeroLinha++;
            continue;
        }

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

void Montador::processarOperando(const std::string &token, int linha, int posAtual, std::vector<int> &codigoObjeto)
{
    std::string baseSymbol = token;
    int offset = 0;

    // Trata sintaxe de array como ROTULO+2
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
        mapa_relocacao.push_back(0); // Referência externa resolvida na ligação
    }
    else if (tabela_simbolos.count(baseSymbol))
    {
        codigoObjeto.push_back(tabela_simbolos[baseSymbol] + offset);
        mapa_relocacao.push_back(1); // Relocável (sofrerá deslocamento de módulo)
    }
    else if (tabela_equ.count(baseSymbol))
    {
        codigoObjeto.push_back(tabela_equ[baseSymbol] + offset);
        mapa_relocacao.push_back(0); // Absoluto
    }
    else
    {
        try
        {
            int valor = std::stoi(token);
            codigoObjeto.push_back(valor);
            mapa_relocacao.push_back(0); // Constante / Absoluto
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

    // Constrói a Tabela de Definições
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

        if (operacao == "SECAO" || operacao == "SECTION" || operacao == "EQU" ||
            operacao == "BEGIN" || operacao == "END" || operacao == "PUBLIC" || operacao == "EXTERN")
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
            mapa_relocacao.push_back(0); // Opcode é instrução absoluta

            for (size_t i = offsetToken + 1; i < tokens.size(); ++i)
            {
                int posAtual = codigoObjeto.size();
                processarOperando(tokens[i], numeroLinha, posAtual, codigoObjeto);
            }
        }
        else if (operacao == "SPACE")
        {
            int qtd = (tokens.size() > (size_t)(offsetToken + 1)) ? std::stoi(tokens[offsetToken + 1]) : 1;
            for (int i = 0; i < qtd; ++i)
            {
                codigoObjeto.push_back(0);
                mapa_relocacao.push_back(0);
            }
        }
        else if (operacao == "CONST")
        {
            std::string arg = tokens[offsetToken + 1];
            int valor = tabela_equ.count(arg) ? tabela_equ[arg] : std::stoi(arg);
            codigoObjeto.push_back(valor);
            mapa_relocacao.push_back(0);
        }

        numeroLinha++;
    }

    // Formatação de saída do arquivo objeto
    if (modo_modulo)
    {
        // Formato estruturado para o Ligador
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
        // Formato de linha única (compatível com programas monolíticos sem ligação)
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