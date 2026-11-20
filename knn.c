#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "knn.h"

typedef struct {
    double distancia;
    const char *rotulo;
} Vizinho;

double distanciaEuclidiana(const double *a, const double *b, int n)
{
    double soma = 0.0;

    for (int i = 0; i < n; i++) {
        double diferenca = a[i] - b[i];
        soma += diferenca * diferenca;
    }
    return sqrt(soma);
}

/* Mantem em "vizinhos" os k mais proximos vistos ate agora, do mais perto
 * para o mais longe. "total" e quantos ja estao guardados (no maximo k). */
static void insereVizinho(Vizinho *vizinhos, int *total, int k, Vizinho novo)
{
    int posicao;

    if (*total < k)
        posicao = (*total)++;
    else if (novo.distancia < vizinhos[k - 1].distancia)
        posicao = k - 1; /* o mais distante sai para dar lugar ao novo */
    else
        return;

    while (posicao > 0 && vizinhos[posicao - 1].distancia > novo.distancia) {
        vizinhos[posicao] = vizinhos[posicao - 1];
        posicao--;
    }
    vizinhos[posicao] = novo;
}

/* Votacao majoritaria. Em caso de empate vence, entre as classes empatadas,
 * a do vizinho mais proximo (os vizinhos ja chegam ordenados por distancia). */
static const char *rotuloMaisVotado(const Vizinho *vizinhos, int total)
{
    const char *vencedor = vizinhos[0].rotulo;
    int maiorVotacao = 0;

    for (int i = 0; i < total; i++) {
        int votos = 0;
        for (int j = 0; j < total; j++)
            if (strcmp(vizinhos[i].rotulo, vizinhos[j].rotulo) == 0)
                votos++;

        if (votos > maiorVotacao) {
            maiorVotacao = votos;
            vencedor = vizinhos[i].rotulo;
        }
    }
    return vencedor;
}

const char *classifica(const Base *treino, const double *amostra, int k)
{
    Vizinho *vizinhos = malloc((size_t)k * sizeof *vizinhos);
    if (vizinhos == NULL)
        return NULL;

    int total = 0;
    for (int i = 0; i < treino->n_amostras; i++) {
        Vizinho candidato = {
            distanciaEuclidiana(amostra, amostraDaBase(treino, i), treino->n_atributos),
            treino->amostras[i].rotulo
        };
        insereVizinho(vizinhos, &total, k, candidato);
    }

    const char *rotulo = rotuloMaisVotado(vizinhos, total);
    free(vizinhos);
    return rotulo;
}