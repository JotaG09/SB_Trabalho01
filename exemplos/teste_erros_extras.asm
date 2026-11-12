SECAO DATA
ESTE_ROTULO_TEM_MUITOS_E_MUITOS_CARACTERES_ACIMA_DE_TRINTA: SPACE ; ERRO LEXICO: Rótulo com mais de 30 caracteres
VALOR_EQU EQU string_invalida                                  ; ERRO SINTATICO: Valor inválido para EQU
LOAD N1                                                        ; ERRO SEMANTICO: Instrução executável fora da SECAO TEXT
IF N1                                                          ; ERRO SEMANTICO: Diretiva IF fora da SECAO TEXT

SECAO TEXT
N1: CONST 10
STOP