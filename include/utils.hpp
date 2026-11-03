#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

namespace Utils
{
    // Converte a string para maiúsculo (case-insensitive)
    inline void paraMaiusculo(std::string &str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    // Remove os comentários (tudo a partir do caractere ';')
    inline void removerComentarios(std::string &linha)
    {
        size_t pos = linha.find(';');
        if (pos != std::string::npos)
        {
            linha = linha.substr(0, pos);
        }
    }

    // Separa a linha em tokens, removendo tabulações e espaços desnecessários
    inline std::vector<std::string> tokenizar(const std::string &linha)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(linha);
        std::string token;

        while (ss >> token)
        {
            tokens.push_back(token);
        }
        return tokens;
    }
}

#endif // UTILS_HPP