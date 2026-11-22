#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dataset.h"
#include "knn.h"
#include "normaliza.h"

typedef struct {
    const char *arquivoTreino;
    const char *arquivoTeste;
    int k;
    Metodo metodo;
    int detalhado;
} Opcoes;

static void mostraUso(const char *programa)
{
    fprintf(stderr,
            "Uso: %s -k <vizinhos> -t <treino> -v <teste> [-n min-max|z-score] [-d]\n",
            programa);
}

/* Converte o texto de -k em um inteiro positivo. Devolve 0 se for invalido. */
static int leK(const char *texto, int *k)
{
    char *fim;
    long valor = strtol(texto, &fim, 10);

    if (*texto == '\0' || *fim != '\0' || valor <= 0 || valor > INT_MAX)
        return 0;
    *k = (int)valor;
    return 1;
}

/* Preenche "opcoes" a partir da linha de comando. Devolve 0 se ela for invalida. */
static int leOpcoes(int argc, char **argv, Opcoes *opcoes)
{
    int opcao;

    opcoes->arquivoTreino = NULL;
    opcoes->arquivoTeste = NULL;
    opcoes->k = 0;
    opcoes->metodo = NENHUM;
    opcoes->detalhado = 0;

    while ((opcao = getopt(argc, argv, "k:t:v:n:d")) != -1) {
        switch (opcao) {
        case 'k':
            if (!leK(optarg, &opcoes->k)) {
                fprintf(stderr, "Erro: -k deve ser um inteiro positivo\n");
                return 0;
            }
            break;
        case 't':
            opcoes->arquivoTreino = optarg;
            break;
        case 'v':
            opcoes->arquivoTeste = optarg;
            break;
        case 'n':
            if (strcmp(optarg, "min-max") == 0) {
                opcoes->metodo = MIN_MAX;
            } else if (strcmp(optarg, "z-score") == 0) {
                opcoes->metodo = Z_SCORE;
            } else {
                fprintf(stderr, "Erro: -n aceita apenas min-max ou z-score\n");
                return 0;
            }
            break;
        case 'd':
            opcoes->detalhado = 1;
            break;
        default:
            return 0;
        }
    }

    if (opcoes->arquivoTreino == NULL || opcoes->arquivoTeste == NULL || opcoes->k == 0 || optind != argc) {
        fprintf(stderr, "Erro: -k, -t e -v sao obrigatorios\n");
        return 0;
    }
    return 1;
}

/* Normaliza treino e teste com parametros calculados apenas no treino. */
static int normalizaBases(Metodo metodo, Base *treino, Base *teste)
{
    Normalizador *normalizador = criaNormalizador(treino, metodo);
    if (normalizador == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente\n");
        return 0;
    }

    normaliza(normalizador, treino);
    normaliza(normalizador, teste);
    liberaNormalizador(normalizador);
    return 1;
}

/* Classifica cada exemplo de teste e imprime o resultado. */
static int executa(const Opcoes *opcoes, Base *treino, Base *teste)
{
    if (treino->n_atributos != teste->n_atributos) {
        fprintf(stderr, "Erro: treino e teste tem quantidades de valores diferentes\n");
        return EXIT_FAILURE;
    }
    if (opcoes->metodo != NENHUM && !normalizaBases(opcoes->metodo, treino, teste))
        return EXIT_FAILURE;

    int k = (opcoes->k < treino->n_amostras) ? opcoes->k : treino->n_amostras;
    int acertos = 0;

    for (int i = 0; i < teste->n_amostras; i++) {
        const char *previsto = classifica(treino, amostraDaBase(teste, i), k);
        if (previsto == NULL) {
            fprintf(stderr, "Erro: memoria insuficiente\n");
            return EXIT_FAILURE;
        }

        int correta = strcmp(previsto, teste->amostras[i].rotulo) == 0;
        acertos += correta;

        if (opcoes->detalhado)
            printf("%s: %s\n", teste->amostras[i].nome,
                   correta ? "correta" : "incorreta");
    }

    printf("Acurácia = %.1f%%\n", 100.0 * acertos / teste->n_amostras);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    Opcoes opcoes;
    if (!leOpcoes(argc, argv, &opcoes)) {
        mostraUso(argv[0]);
        return EXIT_FAILURE;
    }

    Base *treino = carregaBase(opcoes.arquivoTreino);
    Base *teste = carregaBase(opcoes.arquivoTeste);

    int status = EXIT_FAILURE;
    if (treino != NULL && teste != NULL)
        status = executa(&opcoes, treino, teste);

    liberaBase(treino);
    liberaBase(teste);
    return status;
}
