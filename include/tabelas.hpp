#ifndef TABELAS_HPP
#define TABELAS_HPP

#include <string>
#include <unordered_map>

struct Instrucao {
    int opcode;
    int tamanho;
};

// Tabela contendo mnemônicos e alias aceitos pela máquina hipotética
const std::unordered_map<std::string, Instrucao> tabela_instrucoes = {
    {"ADD",    {1, 2}},
    {"SUB",    {2, 2}},
    {"MUL",    {3, 2}},
    {"MULT",   {3, 2}}, // Aceita a variação MULT presente no arquivo de teste
    {"DIV",    {4, 2}},
    {"JMP",    {5, 2}},
    {"JMPN",   {6, 2}},
    {"JMPP",   {7, 2}},
    {"JMPZ",   {8, 2}},
    {"COPY",   {9, 3}},
    {"LOAD",   {10, 2}},
    {"STORE",  {11, 2}},
    {"INPUT",  {12, 2}},
    {"OUTPUT", {13, 2}},
    {"STOP",   {14, 1}}
};

#endif // TABELAS_HPP