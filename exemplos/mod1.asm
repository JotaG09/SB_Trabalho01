MOD1: BEGIN
SECAO TEXT
; Exporta os rótulos para que o Módulo 2 possa enxergá-los
PUBLIC N1
PUBLIC N2
PUBLIC RESULTADO
PUBLIC FINALIZAR

; Importa a rotina de soma do Módulo 2
EXTERN SOMA_MOD2

; Lê os dois números do usuário
INPUT N1
INPUT N2

; Salta para o Módulo 2 para calcular a soma
JMP SOMA_MOD2

; O Módulo 2 saltará de volta para cá!
FINALIZAR: OUTPUT RESULTADO
STOP

SECAO DATA
N1: SPACE
N2: SPACE
RESULTADO: SPACE
END