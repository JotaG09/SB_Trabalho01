#include <iostream>
#include <fstream>
#include <string>
#include "montador.hpp"

int main(int argc, char *argv[])
{
    // Validação de terminal: ex ./montador myprogram.asm
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

    // Altera a extensão do arquivo para .obj
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

    // Instancia o Montador e executa as passagens
    Montador montador;
    montador.executar(arquivoEntrada, arquivoSaida);

    std::cout << "Montagem finalizada com sucesso. Arquivo gerado: " << nomeArquivoSaida << std::endl;

    arquivoEntrada.close();
    arquivoSaida.close();

    return 0;
}