#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "montador.hpp"

struct ModuloObjeto
{
    std::string nome;
    std::vector<std::pair<std::string, int>> tabela_uso;
    std::unordered_map<std::string, int> tabela_def;
    std::vector<int> mapa_reloc;
    std::vector<int> codigo;
    int fator_relocacao = 0;
};

// Lê a estrutura do ficheiro .obj gerado no modo de ligação
ModuloObjeto lerFicheiroObjeto(const std::string &nomeArquivoObj)
{
    ModuloObjeto mod;
    mod.nome = nomeArquivoObj;
    std::ifstream is(nomeArquivoObj);

    if (!is.is_open())
    {
        std::cerr << "Erro ao abrir o ficheiro objeto: " << nomeArquivoObj << std::endl;
        return mod;
    }

    std::string linha;
    while (std::getline(is, linha))
    {
        if (linha.rfind("USO:", 0) == 0)
        {
            std::stringstream ss(linha.substr(4));
            std::string sym;
            int addr;
            while (ss >> sym >> addr)
            {
                mod.tabela_uso.push_back({sym, addr});
            }
        }
        else if (linha.rfind("DEF:", 0) == 0)
        {
            std::stringstream ss(linha.substr(4));
            std::string sym;
            int addr;
            while (ss >> sym >> addr)
            {
                mod.tabela_def[sym] = addr;
            }
        }
        else if (linha.rfind("RELOC:", 0) == 0)
        {
            std::stringstream ss(linha.substr(6));
            int bit;
            while (ss >> bit)
            {
                mod.mapa_reloc.push_back(bit);
            }
        }
        else if (linha.rfind("CODE:", 0) == 0)
        {
            std::stringstream ss(linha.substr(5));
            int val;
            while (ss >> val)
            {
                mod.codigo.push_back(val);
            }
        }
    }
    return mod;
}

int main(int argc, char *argv[])
{
    // O ligador aceita no máximo 3 programas na linha de comando
    if (argc < 2 || argc > 4)
    {
        std::cerr << "Erro: Quantidade de argumentos invalida." << std::endl;
        std::cerr << "Uso: ./ligador prog1.asm [prog2.asm] [prog3.asm]" << std::endl;
        return 1;
    }

    std::vector<std::string> ficheirosAsm;
    for (int i = 1; i < argc; ++i)
    {
        ficheirosAsm.push_back(argv[i]);
    }

    std::vector<ModuloObjeto> modulos;

    // 1. Invoca o Montador internamente para criar os objetos intermediários
    for (const auto &asmPath : ficheirosAsm)
    {
        std::ifstream input(asmPath);
        if (!input.is_open())
        {
            std::cerr << "Erro ao abrir o ficheiro ASM: " << asmPath << std::endl;
            return 1;
        }

        std::string objPath = asmPath;
        size_t posDot = objPath.find_last_of('.');
        if (posDot != std::string::npos)
        {
            objPath = objPath.substr(0, posDot) + ".obj";
        }
        else
        {
            objPath += ".obj";
        }

        std::ofstream output(objPath);
        if (!output.is_open())
        {
            std::cerr << "Erro ao criar o ficheiro objeto intermediario: " << objPath << std::endl;
            return 1;
        }

        Montador montador;
        montador.executar(input, output);
        input.close();
        output.close();

        modulos.push_back(lerFicheiroObjeto(objPath));
    }

    // 2. Passo 1 do Ligador: Fatores de Relocação e Tabela Global de Definições
    std::unordered_map<std::string, int> tabela_global_def;
    int fatorAtual = 0;

    for (auto &mod : modulos)
    {
        mod.fator_relocacao = fatorAtual;
        for (const auto &par : mod.tabela_def)
        {
            if (tabela_global_def.count(par.first))
            {
                std::cerr << "ERRO SEMANTICO: Simbolo publico '" << par.first << "' redefinido em multiplos modulos." << std::endl;
            }
            else
            {
                tabela_global_def[par.first] = par.second + mod.fator_relocacao;
            }
        }
        fatorAtual += mod.codigo.size();
    }

    // 3. Passo 2 do Ligador: Relocação e Resolução de Símbolos Externos
    std::vector<int> codigoExecutavelFinal;

    for (auto &mod : modulos)
    {
        // Reloca endereços internos de acordo com a tabela de relocação
        for (size_t i = 0; i < mod.codigo.size(); ++i)
        {
            if (i < mod.mapa_reloc.size() && mod.mapa_reloc[i] == 1)
            {
                mod.codigo[i] += mod.fator_relocacao;
            }
        }

        // Resolve os endereços dos símbolos da Tabela de Uso
        for (const auto &uso : mod.tabela_uso)
        {
            std::string simbolo = uso.first;
            int enderecoUso = uso.second;

            if (tabela_global_def.count(simbolo))
            {
                mod.codigo[enderecoUso] += tabela_global_def[simbolo];
            }
            else
            {
                std::cerr << "ERRO SEMANTICO: Simbolo externo '" << simbolo << "' nao encontrado na Tabela de Definicoes." << std::endl;
            }
        }

        // Junta o código do módulo ao executável unificado
        codigoExecutavelFinal.insert(codigoExecutavelFinal.end(), mod.codigo.begin(), mod.codigo.end());
    }

    // 4. Grava o executável final .LIG
    std::string nomeSaidaLig = ficheirosAsm[0];
    size_t posPonto = nomeSaidaLig.find_last_of('.');
    if (posPonto != std::string::npos)
    {
        nomeSaidaLig = nomeSaidaLig.substr(0, posPonto) + ".lig";
    }
    else
    {
        nomeSaidaLig += ".lig";
    }

    std::ofstream arqLig(nomeSaidaLig);
    if (!arqLig.is_open())
    {
        std::cerr << "Erro ao criar ficheiro executavel " << nomeSaidaLig << std::endl;
        return 1;
    }

    for (size_t i = 0; i < codigoExecutavelFinal.size(); ++i)
    {
        arqLig << codigoExecutavelFinal[i];
        if (i < codigoExecutavelFinal.size() - 1)
        {
            arqLig << " ";
        }
    }
    arqLig << "\n";
    arqLig.close();

    std::cout << "Ligacao concluida com sucesso. Ficheiro gerado: " << nomeSaidaLig << std::endl;

    return 0;
}