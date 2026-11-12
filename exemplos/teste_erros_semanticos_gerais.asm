SECAO DATA
VAR1: SPACE
VAR1: CONST 20  ; ERRO SEMANTICO: Rótulo 'VAR1' redefinido
LOAD VAR1       ; ERRO SEMANTICO: Instrução executável fora da SECAO TEXT

SECAO TEXT
IF VAR1         ; ERRO SEMANTICO: Diretiva IF fora da SECAO TEXT (deve ser verificada conforme o escopo)
JMP ROT_FANTASMA ; ERRO SEMANTICO: Rótulo referenciado mas ausente / não declarado
OUTPUT VAR1
STOP