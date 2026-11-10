#ifndef ARGUMENTOS_H
#define ARGUMENTOS_H

// métodos de normalização possíveis (opção -n)
typedef enum
{
  NORM_NENHUMA,   // -n não informada: vetores originais
  NORM_MINMAX,    // -n min-max
  NORM_ZSCORE     // -n z-score
} MetodoNorm;

// opções da linha de comando
typedef struct
{
  int k;                   // -k: número de vizinhos
  const char *treino;      // -t: arquivo da base de treinamento
  const char *teste;       // -v: arquivo da base de teste
  MetodoNorm normalizacao; // -n: método de normalização
  int detalhado;           // -d: 1 se o modo detalhado foi pedido
} Argumentos;

// interpreta argc/argv. Retorna 1 em sucesso e 0 em erro (mensagem já impressa).
int parse_argumentos(int argc, char **argv, Argumentos *args);

#endif