# Montador e Ligador - Software Básico

Este projeto desenvolve o **Montador** e o **Ligador** para a arquitetura hipotética utilizada na disciplina de **Software Básico**

## Funcionalidades e Arquitetura

1. ### Montador (`montador`)

- **Processamento de Duas Passagens:**
- **1ª Passagem:** Mapeamento da Tabela de Símbolos, verificação de redefinições, validação de rótulos, tratamento de diretivas de pré-processamento (`EQU` e `IF`) e cálculo das posições dos símbolos nas seções (`TEXT` e `DATA`).
- **2ª Passagem:** Tradução de instruções e operandos para bytecode, geração de tabela de uso (`USO`), tabela de definições (`DEF`) e do vetor de relocação (`RELOC`).

- **Tratamento Estrito de Erros:** Identificação e ordenação cronológica por número de linha de erros **Léxicos**, **Sintáticos** e **Semânticos**. Caso qualquer erro seja encontrado, a montagem é interrompida e nenhum arquivo objeto (`.obj`) é gerado.

---

### 2. Ligador (`ligador`)

O **Ligador** combina até 3 módulos objeto independentes (gerados com as diretivas `BEGIN`/`END`, `PUBLIC` e `EXTERN`) em um único arquivo de máquina executável.

#### Como o Ligador Funciona:
1. **Ligação de Módulos:** Combina 2 ou 3 ficheiros .obj gerados pelo montador em modo módulo (com diretivas BEGIN/END, EXTERN e PUBLIC).

2. **Resolução de Endereços e Relocação de Símbolos:** Concatena os códigos, atualiza as posições dos símbolos internos com base no fator de relocação e resolve referências a símbolos externos.

---

## Estrutura do Projeto

```text
.
├── Makefile                # Automação de compilação dos binários
├── README.md               # Documentação do projeto
├── simulador               # Executável de simulação da CPU (Linux 64-bits)
├── simulador.exe           # Executável de simulação da CPU (Windows)
├── include/                # Arquivos de cabeçalho (.hpp)
│   ├── montador.hpp
│   ├── tabelas.hpp
│   └── utils.hpp
├── src/                    # Código-fonte em C++ (.cpp)
│   ├── main.cpp
│   ├── montador.cpp
│   └── ligador.cpp
└── exemplos/               # Códigos assembly e módulos de teste
    ├── asm-sem-ligacao.asm
    ├── mod1.asm
    ├── mod2.asm
    └── teste_de_erros.asm
```

---

## Compilação e Execução

### Requisitos
- Compilador **g++** com suporte a **C++17** (ou superior).
- Utilitário **Make** (opcional no Windows, caso utilize compilação manual).
- Ambiente **Linux**, **WSL** (Windows Subsystem for Linux) ou **Windows Nativo**.

---

### Opção A: No Linux ou WSL (Recomendado)

1. **Compilar o Projeto:**
   Para compilar o montador e o ligador, execute na raiz do projeto:
   ```bash
   make
   ```
   Isso gerará os executáveis `./montador` e `./ligador`.

2. **Limpar arquivos compilados:**
   ```bash
   make clean
   ```

3. **Como Usar:**
   - **Montador:**
     ```bash
     ./montador exemplos/asm-sem-ligacao.asm
     ```
   - **Ligador:**
     ```bash
     ./ligador exemplos/mod1.asm exemplos/mod2.asm
     ```
   - **Simulador (Linux):**
     ```bash
     chmod +x simulador
     ./simulador exemplos/asm-sem-ligacao.obj
     # Ou para programas ligados:
     ./simulador exemplos/mod1.lig
     ```

---

### Opção B: No Windows (Nativo via PowerShell ou CMD)

Se você estiver utilizando o Windows de forma nativa e possuir o compilador `g++` instalado (via **MinGW-w64** ou **MSYS2**):

1. **Compilar os Executáveis Manualmente:**
   Abra o PowerShell na raiz do projeto e execute os comandos de compilação:
   ```powershell
   g++ -std=c++17 src/main.cpp src/montador.cpp src/utils.cpp -I include -o montador.exe
   g++ -std=c++17 src/ligador.cpp src/montador.cpp src/utils.cpp -I include -o ligador.exe
   ```

2. **Como Usar:**
   - **Montador:**
     ```powershell
     .\montador.exe exemplos\asm-sem-ligacao.asm
     ```
   - **Ligador:**
     ```powershell
     .\ligador.exe exemplos\mod1.asm exemplos\mod2.asm
     ```
   - **Simulador (Windows):**
     Utilize o executável `simulador.exe` incluído no projeto:
     ```powershell
     .\simulador.exe exemplos\asm-sem-ligacao.obj
     # Ou para programas ligados:
     .\simulador.exe exemplos\mod1.lig
     ```

---

## Saídas de Erro e Exemplos

1. ### Exemplo de Relatório de Erros do Montador

Ao montar um arquivo que contenha falhas (como `exemplos/teste_de_erros.asm`), o montador intercepta os problemas, lista-os em ordem de linha e cancela a criação do executável:

```text
ERRO LEXICO (Linha 2): Rotulo invalido '1VAR'.
ERRO SINTATICO (Linha 3): Quantidade de operandos errada para diretiva 'SPACE'.
ERRO SINTATICO (Linha 8): Comando COPY nao deve utilizar virgula entre os operandos.
ERRO SINTATICO (Linha 9): Quantidade de operandos errada para instrucao 'COPY'.
ERRO SINTATICO (Linha 10): Instrucao ou diretiva invalida 'INSTRUCAO_INVALIDA'.
ERRO SEMANTICO (Linha 11): Declaracao ou rotulo ausente 'ROTULO_AUSENTE'.
ERRO SEMANTICO (Linha 12): Rotulo 'N1' redefinido.
ERRO SINTATICO (Linha 13): Quantidade de operandos errada para diretiva 'IF'.

Erros detectados durante o processo. Nenhum arquivo objeto foi gerado.
```

### 2. Exemplo de Relatório de Erros do Ligador

Caso um dos módulos tente utilizar um símbolo que não foi exportado por nenhum outro módulo ligado:

```text
ERRO SEMANTICO (Ligador): Simbolo publico 'MOD_B_FUNC' nao foi encontrado na Tabela Global de Definicoes (TGD).
Ligacao interrompida. Nenhum arquivo final foi gerado.
```

---

## Exemplos de Teste Incluídos

- `asm-sem-ligacao.asm`: Programa assembly básico único contendo seções `TEXT` e `DATA`.
- `mod1.asm` & `mod2.asm`: Exemplos de módulos com conexões `PUBLIC` e `EXTERN` para teste do ligador.
- `teste_de_erros.asm`: Programa de teste contendo intencionalmente erros léxicos, sintáticos e semânticos para validação completa da suíte de testes.