#ifndef TABELAS_HPP
#define TABELAS_HPP

#include <string>
#include <unordered_map>

// Estrutura para armazenar propriedades das instruções
struct Instrucao
{
    int opcode;
    int tamanho;
};

// Tabela de Instruções (TI) populada com as 14 instruções da máquina hipotética
const std::unordered_map<std::string, Instrucao> tabela_instrucoes = {
    {"ADD", {1, 2}},
    {"SUB", {2, 2}},
    {"MUL", {3, 2}},
    {"DIV", {4, 2}},
    {"JMP", {5, 2}},
    {"JMPN", {6, 2}},
    {"JMPP", {7, 2}},
    {"JMPZ", {8, 2}},
    {"COPY", {9, 3}}, // COPY usa 3 palavras de memória e operandos separados por espaço (COPY A B)
    {"LOAD", {10, 2}},
    {"STORE", {11, 2}},
    {"INPUT", {12, 2}},
    {"OUTPUT", {13, 2}},
    {"STOP", {14, 1}}};

#endif // TABELAS_HPP