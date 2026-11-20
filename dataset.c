#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataset.h"

#define SEPARADORES " \t\r\n"
#define CAPACIDADE_INICIAL 256

double *amostraDaBase(const Base *base, int i)
{
    return base->valores + (size_t)i * (size_t)base->n_atributos;
}

/* Conta quantos campos separados por espacos existem na linha. */
static int contaCampos(const char *linha)
{
    char *copia = strdup(linha);
    if (copia == NULL)
        return -1;

    int total = 0;
    for (char *campo = strtok(copia, SEPARADORES); campo != NULL;
         campo = strtok(NULL, SEPARADORES))
        total++;

    free(copia);
    return total;
}

/* Dobra o espaco reservado para amostras (ou reserva o espaco inicial).
 * Devolve 0 se faltar memoria. */
static int ampliaBase(Base *base, int *capacidade)
{
    int nova = (*capacidade == 0) ? CAPACIDADE_INICIAL : 2 * *capacidade;

    Amostra *amostras = realloc(base->amostras, (size_t)nova * sizeof *amostras);
    if (amostras == NULL)
        return 0;
    base->amostras = amostras;
    /* As posicoes novas comecam zeradas para que liberaBase funcione
     * mesmo se a leitura for interrompida no meio de uma amostra. */
    memset(&amostras[*capacidade], 0, (size_t)(nova - *capacidade) * sizeof *amostras);

    double *valores = realloc(base->valores,
                              (size_t)nova * (size_t)base->n_atributos * sizeof *valores);
    if (valores == NULL)
        return 0;
    base->valores = valores;

    *capacidade = nova;
    return 1;
}

/* Interpreta uma linha do arquivo e a acrescenta na base.
 * Devolve NULL se tudo deu certo, ou o texto do problema encontrado. */
static const char *adicionaAmostra(Base *base, int *capacidade, char *linha)
{
    int campos = contaCampos(linha);
    if (campos < 0)
        return "memoria insuficiente";
    if (campos == 0)
        return NULL; /* linha em branco: nada a fazer */
    if (campos < 3)
        return "esperado: nome, ao menos um valor e o rotulo";

    /* Todo campo que nao e o nome nem o rotulo e um atributo. */
    int atributos = campos - 2;
    if (base->n_amostras == 0)
        base->n_atributos = atributos;
    else if (atributos != base->n_atributos)
        return "quantidade de valores diferente da das linhas anteriores";

    if (base->n_amostras == *capacidade && !ampliaBase(base, capacidade))
        return "memoria insuficiente";

    int i = base->n_amostras++;
    Amostra *amostra = &base->amostras[i];
    double *valores = amostraDaBase(base, i);

    amostra->nome = strdup(strtok(linha, SEPARADORES));
    for (int j = 0; j < atributos; j++) {
        char *fim;
        valores[j] = strtod(strtok(NULL, SEPARADORES), &fim);
        if (*fim != '\0')
            return "valor numerico invalido";
    }
    amostra->rotulo = strdup(strtok(NULL, SEPARADORES));

    if (amostra->nome == NULL || amostra->rotulo == NULL)
        return "memoria insuficiente";
    return NULL;
}

Base *carregaBase(const char *caminho)
{
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", caminho);
        return NULL;
    }

    Base *base = calloc(1, sizeof *base);
    const char *erro = (base == NULL) ? "memoria insuficiente" : NULL;
    char *linha = NULL;
    size_t tamanhoLinha = 0;
    int capacidade = 0;
    int numeroLinha = 0;

    while (erro == NULL && getline(&linha, &tamanhoLinha, arquivo) != -1) {
        numeroLinha++;
        erro = adicionaAmostra(base, &capacidade, linha);
    }
    if (erro == NULL && base->n_amostras == 0)
        erro = "arquivo sem nenhuma amostra";

    free(linha);
    fclose(arquivo);

    if (erro != NULL) {
        fprintf(stderr, "Erro em '%s' (linha %d): %s\n", caminho, numeroLinha, erro);
        liberaBase(base);
        return NULL;
    }
    return base;
}

void liberaBase(Base *base)
{
    if (base == NULL)
        return;

    for (int i = 0; i < base->n_amostras; i++) {
        free(base->amostras[i].nome);
        free(base->amostras[i].rotulo);
    }
    free(base->amostras);
    free(base->valores);
    free(base);
}