MOD2: BEGIN
SECAO TEXT
; Exporta o ponto de entrada da rotina de soma
PUBLIC SOMA_MOD2

; Importa as variáveis e o ponto de retorno do Módulo 1
EXTERN N1
EXTERN N2
EXTERN RESULTADO
EXTERN FINALIZAR

SOMA_MOD2: LOAD N1
ADD N2
STORE RESULTADO

; Salta de volta para o Módulo 1 para imprimir e parar
JMP FINALIZAR

SECAO DATA
; Não precisa de variáveis próprias, usa as do Módulo 1
END