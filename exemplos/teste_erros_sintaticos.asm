SECAO DATA
N1: SPACE 1 2   ; ERRO SINTATICO: SPACE aceita no máximo UM argumento
N2: CONST 10

SECAO TEXT
INPUT N1 N2     ; ERRO SINTATICO: INPUT aceita apenas 1 operando
COPY N1, N2     ; ERRO SINTATICO: COPY não deve utilizar vírgula entre os operandos
COPY N1         ; ERRO SINTATICO: COPY exige exatamente 2 operandos
INSTRUCAO_INVALIDA N1 ; ERRO SINTATICO: Instrução ou mnemônico inválido
IF              ; ERRO SINTATICO: Diretiva IF sem argumento
STOP