# Montador e Ligador - Software Básico

Este projeto desenvolve o **Montador** e o **Ligador** para a arquitetura hipotética utilizada na disciplina de **Software Básico**.

## Funcionalidades e Arquitetura

### 1. Montador (`montador` / `montador.exe`)

- **Processamento de Duas Passagens:**
  - **1ª Passagem:** Mapeamento da Tabela de Símbolos, verificação de redefinições, validação de rótulos, tratamento de diretivas de pré-processamento (`EQU` e `IF`) e cálculo das posições dos símbolos nas seções (`TEXT` e `DATA`).
  - **2ª Passagem:** Tradução de instruções e operandos para bytecode, geração de tabela de uso (`USO`), tabela de definições (`DEF`) e do vetor de relocação (`RELOC`).

- **Tratamento Estrito de Erros com Linha Original:** Identificação e ordenação cronológica por número de linha de erros **Léxicos**, **Sintáticos** e **Semânticos**, exibindo na tela o tipo do erro, o número da linha e o conteúdo exato da linha correspondente no arquivo fonte. Caso qualquer erro seja encontrado, a montagem é interrompida e nenhum arquivo objeto (`.obj`) é gerado.

---

### 2. Ligador (`ligador` / `ligador.exe`)

O **Ligador** combina até 3 módulos objeto independentes (gerados com as diretivas `BEGIN`/`END`, `PUBLIC` e `EXTERN`) em um único arquivo de máquina executável (`.lig`).

#### Como o Ligador Funciona:
1. **Ligação de Módulos:** Combina os ficheiros `.obj` gerados pelo montador em modo módulo.
2. **Resolução de Endereços e Relocação de Símbolos:** Concatena os códigos, atualiza as posições dos símbolos internos com base no fator de relocação e resolve referências a símbolos externos utilizando a Tabela Global de Definições (TGD).

---

## Estrutura do Projeto

```text
.
├── Makefile                # Automação de compilação multiplataforma (Linux/Windows)
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
- Utilitário **Make**.
- Ambiente **Linux**, **WSL** (Windows Subsystem for Linux) ou **Windows Nativo** (PowerShell/CMD).

---

### Compilação Universal (Linux, WSL ou Windows)

Graças ao suporte nativo no `Makefile`, basta executar o mesmo comando em qualquer sistema operacional:

1. **Compilar o Projeto:**
   ```bash
   make
   ```
   *No Linux/WSL*, isso gerará os executáveis `./montador` e `./ligador`.
   *No Windows*, isso gerará automaticamente `montador.exe` e `ligador.exe`.

2. **Limpar arquivos compilados e objetos/relatórios temporários:**
   ```bash
   make clean
   ```

---

### Como Usar

#### A. No Linux ou WSL

- **Montador:**
  ```bash
  ./montador exemplos/asm-sem-ligacao.asm
  ```
- **Ligador:**
  ```bash
  ./ligador exemplos/mod1.asm exemplos/mod2.asm
  ```
- **Simulador:**
  ```bash
  chmod +x simulador
  ./simulador exemplos/asm-sem-ligacao.obj
  # Ou para programas ligados:
  ./simulador exemplos/mod1.lig
  ```

#### B. No Windows (PowerShell ou CMD)

- **Montador:**
  ```powershell
  .\montador.exe exemplos\asm-sem-ligacao.asm
  ```
- **Ligador:**
  ```powershell
  .\ligador.exe exemplos\mod1.asm exemplos\mod2.asm
  ```
- **Simulador:**
  ```powershell
  .\simulador.exe exemplos\asm-sem-ligacao.obj
  # Ou para programas ligados:
  .\simulador.exe exemplos\mod1.lig
  ```

---

## Saídas de Erro e Exemplos

1. ### Exemplo de Relatório de Erros do Montador

Ao montar um arquivo que contenha falhas, o montador intercepta os problemas, lista-os em ordem de linha (exibindo o código-fonte original) e cancela a criação do arquivo objeto:

```text
ERRO LEXICO (Linha 2 [1VAR: SPACE]): Rotulo invalido '1VAR'.
ERRO SINTATICO (Linha 3 [N1: SPACE 1 2]): Quantidade de operandos errada para diretiva 'SPACE'.
ERRO SINTATICO (Linha 8 [COPY N1, N2]): Comando COPY nao deve utilizar virgula entre os operandos.
ERRO SINTATICO (Linha 9 [COPY N1]): Quantidade de operandos errada para instrucao 'COPY'.

Erros detectados durante o processo. Nenhum arquivo objeto foi gerado.
```

2. ### Exemplo de Relatório de Erros do Ligador

Caso um dos módulos tente utilizar um símbolo externo que não foi exportado por nenhum outro módulo ligado:

```text
ERRO SEMANTICO: Simbolo externo 'ROTULO_FANTASMA' nao encontrado na Tabela de Definicoes.
Falha no processo de ligacao devido a erros semanticos.
```

---

## Exemplos de Teste Incluídos

- `asm-sem-ligacao.asm`: Programa assembly básico único contendo seções `TEXT` e `DATA`.
- `mod1.asm` & `mod2.asm`: Exemplos de módulos com conexões `PUBLIC` e `EXTERN` para teste do ligador.
- `teste_de_erros.asm`: Programa de teste contendo intencionalmente erros léxicos, sintáticos e semânticos para validação completa da suíte de testes.