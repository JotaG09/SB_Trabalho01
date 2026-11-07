# Montador e Ligador - Software Básico

Este projeto desenvolve o **Montador** e o **Ligador** para a arquitetura hipotética utilizada na disciplina de **Software Básico** 

O sistema é responsável por traduzir programas em linguagem assembly hipotética de duas passagens para código de máquina, suportando montagem de código único, pré-processamento de diretivas (como `EQU` e `IF`), verificação completa de erros e ligação de múltiplos módulos.

---

## Funcionalidades

### 1. Montador (`montador`)
* **Duas Passagens:**
  * **1ª Passagem:** Mapeamento da Tabela de Símbolos, validação de rótulos, tratamento das diretivas `EQU` e `IF`, e contagem do contador de localização (LC).
  * **2ª Passagem:** Geração do código objeto (`.obj`), resolução de endereços, construção das tabelas de uso (`USO`), definições (`DEF`) e mapa de relocação (`RELOC`).
* **Tratamento de Erros:** Identificação e ordenação por linha de erros **Léxicos**, **Sintáticos** e **Semânticos**. O ficheiro objeto só é gerado se o código estiver 100% livre de erros.

### 2. Ligador (`ligador`)
* **Ligação de Módulos:** Combina 2 ou 3 ficheiros `.obj` gerados pelo montador em modo módulo (com diretivas `BEGIN`/`END`, `EXTERN` e `PUBLIC`).
* **Relocação e Resolução de Símbolos:** Concatena os códigos, atualiza as posições dos símbolos internos com base no fator de relocação e resolve referências a símbolos externos.

---

## Estrutura do Projeto

```text
.
├── Makefile                # Automação de compilação dos binários
├── README.md               # Documentação do projeto
├── simulador               # Executável de simulação da CPU
├── include/                # Ficheiros de cabeçalho (.hpp)
│   ├── montador.hpp
│   ├── tabelas.hpp
│   └── utils.hpp
├── src/                    # Código-fonte em C++ (.cpp)
│   ├── main.cpp
│   ├── montador.cpp
│   └── ligador.cpp
└── exemplos/               # Códigos assembly e módulos de teste
    ├── asm-com-ligacao.asm
    ├── asm-sem-ligacao.asm
    ├── mod1.asm
    ├── mod2.asm
    └── teste_de_erros.asm
```

---

## Compilação e Execução

### Requisitos
* Compilador **g++** com suporte a **C++17** (ou superior).
* Utilitário **Make**.
* Ambiente Linux ou WSL.

### 1. Compilar o Projeto

Para compilar o montador e o ligador, execute na raiz do projeto:

```bash
make
```

Isso gera dois executáveis: `./montador` e `./ligador`.

Para limpar os binários e ficheiros temporários:

```bash
make clean
```

---

## Como Usar

### Executando o Montador

Para montar um ficheiro de código assembly:

```bash
./montador exemplos/asm-sem-ligacao.asm
```

* **Saída com Sucesso:** Gera o ficheiro `exemplos/asm-sem-ligacao.obj`.
* **Saída com Erros:** Imprime as mensagens de erro detalhadas por linha e **interrompe** a geração do ficheiro objeto.

---

### Executando o Ligador

Para ligar múltiplos módulos (de 2 a 3 ficheiros `.obj`):

```bash
./ligador exemplos/mod1.obj exemplos/mod2.obj
```

* Gera o ficheiro final compilado (ex: `exemplos/mod1.e` ou código objeto consolidado) pronto para ser executado no simulador.

---

### Executando no Simulador

Após gerar o ficheiro executável/objeto final:

```bash
chmod +x simulador
./simulador exemplos/asm-sem-ligacao.obj
```

---

## Exemplos de Teste Incluídos

- `asm-sem-ligacao.asm`: Programa assembly básico único de secção `TEXT` e `DATA`.
- `mod1.asm` & `mod2.asm`: Exemplos de módulos com ligações `PUBLIC` e `EXTERN` para teste do ligador.
- `teste_de_erros.asm`: Programa de teste com propositados erros léxicos, sintáticos e semânticos para verificação da validação do montador.