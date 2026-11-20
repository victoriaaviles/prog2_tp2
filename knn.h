#ifndef KNN_H
#define KNN_H

#include "dataset.h"

/* Distancia Euclidiana entre dois vetores com n atributos. */
double distanciaEuclidiana(const double *a, const double *b, int n);

/* Classifica o vetor "amostra" pelos k vizinhos mais proximos do treino
 * (1 <= k <= treino->n_amostras). Devolve o rotulo vencedor, que pertence
 * ao treino e nao deve ser liberado. Devolve NULL se faltar memoria. */
const char *classifica(const Base *treino, const double *amostra, int k);

#endif