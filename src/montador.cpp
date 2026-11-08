#include "montador.hpp"
#include "utils.hpp"
#include "tabelas.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

Montador::Montador() : secao_text_encontrada(false), secao_data_encontrada(false), modo_modulo(false) {}

void Montador::emitirErro(int linha, const std::string &tipo, const std::string &mensagem)
{
    erros.push_back({linha, tipo, mensagem});
}

bool Montador::executar(std::ifstream &arquivoEntrada, std::ofstream &arquivoSaida)
{
    erros.clear();
    tabela_simbolos.clear();
    tabela_secao_simbolo.clear();
    tabela_equ.clear();

    primeiraPassagem(arquivoEntrada);

    if (!secao_text_encontrada)
    {
        emitirErro(0, "SEMANTICO", "Secao TEXT ausente no programa.");
    }

    arquivoEntrada.clear();
    arquivoEntrada.seekg(0, std::ios::beg);

    segundaPassagem(arquivoEntrada, arquivoSaida);

    if (!erros.empty())
    {
        // Ordena todos os erros acumulados por número de linha
        std::sort(erros.begin(), erros.end());

        for (const auto &err : erros)
        {
            std::cout << "ERRO " << err.tipo << " (Linha " << err.linha << "): " << err.mensagem << std::endl;
        }
        return false;
    }

    return true;
}

void Montador::primeiraPassagem(std::ifstream &arquivoEntrada)
{
    std::string linha;
    int numeroLinha = 1;

    // Coleta de EQU antes de qualquer seção
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

        if (tokens[0] == "SECAO" || tokens[0] == "SECTION")
        {
            break;
        }

        if (tokens.size() >= 3 && (tokens[1] == "EQU" || (tokens[0].back() == ':' && tokens[1] == "EQU")))
        {
            std::string rotulo = (tokens[0].back() == ':') ? tokens[0].substr(0, tokens[0].size() - 1) : tokens[0];
            std::string valStr = (tokens[1] == "EQU") ? tokens[2] : tokens[2];
            try
            {
                tabela_equ[rotulo] = std::stoi(valStr);
            }
            catch (...)
            {
                emitirErro(numeroLinha, "SINTATICO", "Valor invalido para EQU.");
            }
        }
        numeroLinha++;
    }

    arquivoEntrada.clear();
    arquivoEntrada.seekg(0, std::ios::beg);

    numeroLinha = 1;
    int contadorPosicaoText = 0;
    int contadorPosicaoData = 0;
    bool emSecaoText = false;
    bool ignorarProximaLinha = false;
    std::string rotuloPendente = "";

    while (std::getline(arquivoEntrada, linha))
    {
        std::string linhaOriginal = linha;
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

        size_t idx = 0;
        std::string rotulo = rotuloPendente;
        rotuloPendente = "";

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
        }

        if (idx >= tokens.size())
        {
            rotuloPendente = rotulo;
            numeroLinha++;
            continue;
        }

        std::string operacao = tokens[idx++];
        std::vector<std::string> args;
        while (idx < tokens.size())
        {
            args.push_back(tokens[idx++]);
        }

        if (operacao == "SECAO" || operacao == "SECTION")
        {
            if (!args.empty())
            {
                if (args[0] == "TEXT")
                {
                    secao_text_encontrada = true;
                    emSecaoText = true;
                }
                else if (args[0] == "DATA")
                {
                    secao_data_encontrada = true;
                    emSecaoText = false;
                }
            }
            numeroLinha++;
            continue;
        }

        if (operacao == "BEGIN")
        {
            modo_modulo = true;
            if (!rotulo.empty())
                tabela_simbolos[rotulo] = 0;
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
            std::string pubSym = !args.empty() ? args[0] : rotulo;
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

        if (operacao == "IF")
        {
            if (!emSecaoText)
                emitirErro(numeroLinha, "SEMANTICO", "Diretiva IF deve estar na SECAO TEXT.");

            if (args.empty() || args.size() > 1)
            {
                emitirErro(numeroLinha, "SINTATICO", "Quantidade de operandos errada para diretiva 'IF'.");
            }
            else
            {
                std::string arg = args[0];
                int valor = tabela_equ.count(arg) ? tabela_equ[arg] : (std::isdigit(arg[0]) || arg[0] == '-' ? std::stoi(arg) : 0);
                if (valor == 0)
                    ignorarProximaLinha = true;
            }
            numeroLinha++;
            continue;
        }

        if (!rotulo.empty())
        {
            int posFinal = emSecaoText ? contadorPosicaoText : contadorPosicaoData;
            tabela_simbolos[rotulo] = posFinal;
            tabela_secao_simbolo[rotulo] = emSecaoText ? "TEXT" : "DATA";
        }

        if (tabela_instrucoes.count(operacao))
        {
            if (!emSecaoText)
                emitirErro(numeroLinha, "SEMANTICO", "Instrucao executavel fora da SECAO TEXT.");
            contadorPosicaoText += tabela_instrucoes.at(operacao).tamanho;
        }
        else if (operacao == "SPACE")
        {
            if (args.size() > 1)
            {
                emitirErro(numeroLinha, "SINTATICO", "Quantidade de operandos errada para diretiva 'SPACE'.");
            }
            int qtd = 1;
            if (!args.empty())
            {
                try
                {
                    qtd = std::stoi(args[0]);
                }
                catch (...)
                {
                    qtd = 1;
                }
            }
            contadorPosicaoData += qtd;
        }
        else if (operacao == "CONST")
        {
            contadorPosicaoData += 1;
        }
        else
        {
            emitirErro(numeroLinha, "SINTATICO", "Instrucao ou diretiva invalida '" + operacao + "'.");
        }

        numeroLinha++;
    }

    tamanho_text = contadorPosicaoText;

    for (auto &par : tabela_simbolos)
    {
        if (tabela_secao_simbolo[par.first] == "DATA")
        {
            par.second += tamanho_text;
        }
    }
}

void Montador::processarOperando(const std::string &token, int linha, int posAtual, std::vector<int> &codigoObjeto)
{
    std::string argLimpo = token;
    if (!argLimpo.empty() && argLimpo.back() == ',')
        argLimpo.pop_back();

    std::string baseSymbol = argLimpo;
    int offset = 0;

    size_t posMais = argLimpo.find('+');
    if (posMais != std::string::npos)
    {
        baseSymbol = argLimpo.substr(0, posMais);
        try
        {
            offset = std::stoi(argLimpo.substr(posMais + 1));
        }
        catch (...)
        {
            offset = 0;
        }
    }

    if (simbolos_externos.count(baseSymbol))
    {
        tabela_uso.push_back({baseSymbol, posAtual});
        codigoObjeto.push_back(0 + offset);
        mapa_relocacao.push_back(0); // Sinalizado como 0 pq será resolvido pelo Ligador via Uso
    }
    else if (tabela_simbolos.count(baseSymbol))
    {
        codigoObjeto.push_back(tabela_simbolos[baseSymbol] + offset);
        mapa_relocacao.push_back(1); // SÍMBOLO RELOCÁVEL INTERNO (Aqui está a chave!)
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
            int valor = std::stoi(argLimpo);
            codigoObjeto.push_back(valor);
            mapa_relocacao.push_back(0); // Imediato
        }
        catch (...)
        {
            emitirErro(linha, "SEMANTICO", "Declaracao ou rotulo ausente '" + baseSymbol + "'.");
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

    std::vector<int> codigoText;
    std::vector<int> codigoData;
    std::vector<int> relocText;
    std::vector<int> relocData;

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
        std::string linhaOriginal = linha;
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

        size_t idx = 0;
        if (tokens[idx].back() == ':')
            idx++;
        else if (tokens.size() > 1 && (tokens[1] == "BEGIN" || tokens[1] == "EXTERN" || tokens[1] == "PUBLIC" || tokens[1] == "EQU"))
            idx++;

        if (idx >= tokens.size())
        {
            numeroLinha++;
            continue;
        }

        std::string operacao = tokens[idx++];
        std::vector<std::string> args;
        while (idx < tokens.size())
            args.push_back(tokens[idx++]);

        if (operacao == "SECAO" || operacao == "SECTION" || operacao == "BEGIN" ||
            operacao == "END" || operacao == "PUBLIC" || operacao == "EXTERN" || operacao == "EQU")
        {
            numeroLinha++;
            continue;
        }

        if (operacao == "IF")
        {
            if (!args.empty() && args.size() == 1)
            {
                std::string arg = args[0];
                int valor = tabela_equ.count(arg) ? tabela_equ[arg] : (std::isdigit(arg[0]) || arg[0] == '-' ? std::stoi(arg) : 0);
                if (valor == 0)
                    ignorarProximaLinha = true;
            }
            numeroLinha++;
            continue;
        }

        if (operacao == "COPY")
        {
            if (linhaOriginal.find(',') != std::string::npos)
            {
                emitirErro(numeroLinha, "SINTATICO", "Comando COPY nao deve utilizar virgula entre os operandos.");
            }
        }

        if (tabela_instrucoes.count(operacao))
        {
            Instrucao inst = tabela_instrucoes.at(operacao);
            codigoText.push_back(inst.opcode);
            relocText.push_back(0); // Opcode nunca é relocável

            if (args.size() != static_cast<size_t>(inst.tamanho - 1))
            {
                emitirErro(numeroLinha, "SINTATICO", "Quantidade de operandos errada para instrucao '" + operacao + "'.");
            }

            for (size_t i = 0; i < args.size(); ++i)
            {
                int posAtual = codigoText.size();
                mapa_relocacao.clear(); // Limpa o mapa temporário do operando

                processarOperando(args[i], numeroLinha, posAtual, codigoText);

                // Transfere o bit coletado pela processarOperando para o relocText oficial!
                if (!mapa_relocacao.empty())
                {
                    relocText.push_back(mapa_relocacao.back());
                }
                else
                {
                    relocText.push_back(0);
                }
            }
        }
        else if (operacao == "SPACE")
        {
            int qtd = 1;
            if (!args.empty())
            {
                try
                {
                    qtd = std::stoi(args[0]);
                }
                catch (...)
                {
                    qtd = 1;
                }
            }
            for (int i = 0; i < qtd; ++i)
            {
                codigoData.push_back(0);
                relocData.push_back(0);
            }
        }
        else if (operacao == "CONST")
        {
            if (!args.empty())
            {
                std::string arg = args[0];
                int valor = 0;
                if (tabela_equ.count(arg))
                    valor = tabela_equ[arg];
                else
                {
                    try
                    {
                        valor = std::stoi(arg);
                    }
                    catch (...)
                    {
                        valor = 0;
                    }
                }
                codigoData.push_back(valor);
                relocData.push_back(0);
            }
        }

        numeroLinha++;
    }

    std::vector<int> codigoObjetoFinal = codigoText;
    codigoObjetoFinal.insert(codigoObjetoFinal.end(), codigoData.begin(), codigoData.end());

    std::vector<int> mapaRelocFinal = relocText;
    mapaRelocFinal.insert(mapaRelocFinal.end(), relocData.begin(), relocData.end());

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
        for (size_t i = 0; i < mapaRelocFinal.size(); ++i)
        {
            arquivoSaida << " " << mapaRelocFinal[i];
        }
        arquivoSaida << "\n";

        arquivoSaida << "CODE:";
        for (size_t i = 0; i < codigoObjetoFinal.size(); ++i)
        {
            arquivoSaida << " " << codigoObjetoFinal[i];
        }
        arquivoSaida << "\n";
    }
    else
    {
        for (size_t i = 0; i < codigoObjetoFinal.size(); ++i)
        {
            arquivoSaida << codigoObjetoFinal[i];
            if (i < codigoObjetoFinal.size() - 1)
            {
                arquivoSaida << " ";
            }
        }
        arquivoSaida << "\n";
    }
}