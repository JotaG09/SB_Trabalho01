#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "montador.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Erro: Quantidade de argumentos invalida." << std::endl;
        std::cerr << "Uso: ./montador <arquivo.asm>" << std::endl;
        return 1;
    }

    std::string nomeArquivoEntrada = argv[1];
    std::ifstream arquivoEntrada(nomeArquivoEntrada);

    if (!arquivoEntrada.is_open())
    {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo " << nomeArquivoEntrada << std::endl;
        return 1;
    }

    std::string nomeArquivoSaida;
    size_t posPonto = nomeArquivoEntrada.find_last_of('.');
    if (posPonto != std::string::npos)
    {
        nomeArquivoSaida = nomeArquivoEntrada.substr(0, posPonto) + ".obj";
    }
    else
    {
        nomeArquivoSaida = nomeArquivoEntrada + ".obj";
    }

    std::ofstream arquivoSaida(nomeArquivoSaida);
    if (!arquivoSaida.is_open())
    {
        std::cerr << "Erro: Nao foi possivel criar o arquivo " << nomeArquivoSaida << std::endl;
        return 1;
    }

    Montador montador;
    bool sucesso = montador.executar(arquivoEntrada, arquivoSaida);

    arquivoEntrada.close();
    arquivoSaida.close();

    if (sucesso)
    {
        std::cout << "Montagem finalizada com sucesso. Arquivo gerado: " << nomeArquivoSaida << std::endl;
    }
    else
    {
        // Se houver erros, remove o arquivo .obj gerado
        std::remove(nomeArquivoSaida.c_str());
        std::cout << "Erros detectados durante o processo. Nenhum arquivo objeto foi gerado." << std::endl;
    }

    return 0;
}