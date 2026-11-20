#ifndef NORMALIZA_H
#define NORMALIZA_H

#include "dataset.h"

typedef enum {
    NENHUM,
    MIN_MAX,
    Z_SCORE
} Metodo;

/* Parametros de normalizacao de cada atributo j. O valor normalizado e
 *     (x - deslocamento[j]) / escala[j]
 * ou zero, quando escala[j] vale zero.
 *   Min-Max: deslocamento = minimo,  escala = maximo - minimo
 *   Z-Score: deslocamento = media,   escala = desvio padrao */
typedef struct {
    int n_atributos;
    double *deslocamento;
    double *escala;
} Normalizador;

/* Calcula os parametros usando somente a base informada (o treino).
 * Devolve NULL se faltar memoria. */
Normalizador *criaNormalizador(const Base *treino, Metodo metodo);

/* Normaliza a base no proprio lugar. */
void normaliza(const Normalizador *normalizador, Base *base);

/* Libera o normalizador. Aceita NULL. */
void liberaNormalizador(Normalizador *normalizador);

#endif