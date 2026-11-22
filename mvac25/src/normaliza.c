#include <math.h>
#include <stdlib.h>

#include "normaliza.h"

/* Funcao que calcula deslocamento e escala do atributo j na base. */
typedef void (*CalculaParametros)(const Base *base, int j, double *deslocamento, double *escala);

static void parametrosMinMax(const Base *base, int j, double *deslocamento, double *escala)
{
    double menor = amostraDaBase(base, 0)[j];
    double maior = menor;

    for (int i = 1; i < base->n_amostras; i++) {
        double x = amostraDaBase(base, i)[j];
        if (x < menor)
            menor = x;
        if (x > maior)
            maior = x;
    }

    *deslocamento = menor;
    *escala = maior - menor;
}

/* O desvio padrao e o populacional (divisao por N, nao por N - 1). */
static void parametrosZScore(const Base *base, int j, double *deslocamento, double *escala)
{
    double soma = 0.0;
    for (int i = 0; i < base->n_amostras; i++)
        soma += amostraDaBase(base, i)[j];
    double media = soma / base->n_amostras;

    double somaQuadrados = 0.0;
    for (int i = 0; i < base->n_amostras; i++) {
        double diferenca = amostraDaBase(base, i)[j] - media;
        somaQuadrados += diferenca * diferenca;
    }

    *deslocamento = media;
    *escala = sqrt(somaQuadrados / base->n_amostras);
}

Normalizador *criaNormalizador(const Base *treino, Metodo metodo)
{
    CalculaParametros calcula = (metodo == MIN_MAX) ? parametrosMinMax : parametrosZScore;

    Normalizador *normalizador = malloc(sizeof *normalizador);
    if (normalizador == NULL)
        return NULL;

    normalizador->n_atributos = treino->n_atributos;
    normalizador->deslocamento = calloc((size_t)treino->n_atributos, sizeof(double));
    normalizador->escala = calloc((size_t)treino->n_atributos, sizeof(double));
    if (normalizador->deslocamento == NULL || normalizador->escala == NULL) {
        liberaNormalizador(normalizador);
        return NULL;
    }

    for (int j = 0; j < treino->n_atributos; j++)
        calcula(treino, j, &normalizador->deslocamento[j], &normalizador->escala[j]);

    return normalizador;
}

void normaliza(const Normalizador *normalizador, Base *base)
{
    for (int i = 0; i < base->n_amostras; i++) {
        double *vetor = amostraDaBase(base, i);

        for (int j = 0; j < base->n_atributos; j++) {
            if (normalizador->escala[j] == 0.0)
                vetor[j] = 0.0;
            else
                vetor[j] = (vetor[j] - normalizador->deslocamento[j]) / normalizador->escala[j];
        }
    }
}

void liberaNormalizador(Normalizador *normalizador)
{
    if (normalizador == NULL)
        return;

    free(normalizador->deslocamento);
    free(normalizador->escala);
    free(normalizador);
}
