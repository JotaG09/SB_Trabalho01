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

    // 1. Invocação interna do Montador
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

    // 2. Tabela Global de Definições e Fatores de Relocação
    std::unordered_map<std::string, int> tabela_global_def;
    int fatorAtual = 0;
    bool houveErro = false;

    for (auto &mod : modulos)
    {
        mod.fator_relocacao = fatorAtual;
        for (const auto &par : mod.tabela_def)
        {
            if (tabela_global_def.count(par.first))
            {
                std::cerr << "ERRO SEMANTICO: Simbolo publico '" << par.first << "' redefinido em multiplos modulos." << std::endl;
                houveErro = true;
            }
            else
            {
                tabela_global_def[par.first] = par.second + mod.fator_relocacao;
            }
        }
        fatorAtual += mod.codigo.size();
    }

    // 3. Relocação e Resolução de Símbolos Externos
    std::vector<int> codigoExecutavelFinal;

    for (auto &mod : modulos)
    {
        for (size_t i = 0; i < mod.codigo.size(); ++i)
        {
            if (i < mod.mapa_reloc.size() && mod.mapa_reloc[i] == 1)
            {
                mod.codigo[i] += mod.fator_relocacao;
            }
        }

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
                houveErro = true;
            }
        }

        codigoExecutavelFinal.insert(codigoExecutavelFinal.end(), mod.codigo.begin(), mod.codigo.end());
    }

    // Se houve algum erro semântico na ligação, encerra imediatamente sem gerar o .LIG
    if (houveErro)
    {
        std::cerr << "Falha no processo de ligacao devido a erros semanticos." << std::endl;
        return 1;
    }

    // 4. Geração do arquivo .LIG
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