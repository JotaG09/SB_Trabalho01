#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace Utils
{
    inline void paraMaiusculo(std::string &str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    inline void removerComentarios(std::string &linha)
    {
        size_t pos = linha.find(';');
        if (pos != std::string::npos)
        {
            linha = linha.substr(0, pos);
        }
    }

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

    // Verifica erros léxicos em rótulos (Max 30 chars, letras/numeros/_, não inicia com numero)
    inline bool isRotuloValido(const std::string &rotulo)
    {
        if (rotulo.empty() || rotulo.size() > 30)
            return false;
        if (std::isdigit(rotulo[0]))
            return false;
        for (char c : rotulo)
        {
            if (!std::isalnum(c) && c != '_')
                return false;
        }
        return true;
    }
}

#endif // UTILS_HPP