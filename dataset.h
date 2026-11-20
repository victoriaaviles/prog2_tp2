#ifndef DATASET_H
#define DATASET_H

/* Identificacao de uma imagem: nome do arquivo e classe a que pertence. */
typedef struct {
    char *nome;
    char *rotulo;
} Amostra;

/* Uma base (treino ou teste) carregada na memoria.
 * Os valores de todos os vetores ficam em um unico bloco, linha a linha:
 * o atributo j da amostra i esta em valores[i * n_atributos + j]. */
typedef struct {
    int n_amostras;
    int n_atributos;
    double *valores;
    Amostra *amostras;
} Base;

/* Le uma base de um arquivo texto. Cada linha tem o formato:
 *     nome_da_imagem valor_1 valor_2 ... valor_n rotulo
 * Devolve NULL (e escreve o motivo em stderr) se algo der errado. */
Base *carregaBase(const char *caminho);

/* Libera a base. Aceita NULL. */
void liberaBase(Base *base);

/* Devolve o vetor de atributos da amostra i. */
double *amostraDaBase(const Base *base, int i);

#endif