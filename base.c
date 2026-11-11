#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "base.h"

// Copia um texto para memória nova (strdup não existe no C11 estrito).
static char *copiar_texto(const char *origem)
{
  size_t n = strlen(origem);
  char *copia = malloc(n + 1);
  if (copia != NULL)
  {
    memcpy(copia, origem, n + 1);   // n + 1 copia também o '\0'
  }
  return copia;
}


// Tabela de classes

void tabela_inicializar(TabelaClasses *t)
{
  t->rotulos = NULL;
  t->quantidade = 0;
  t->capacidade = 0;
}

void tabela_liberar(TabelaClasses *t)
{
  for (int i = 0; i < t->quantidade; i++)
  {
    free(t->rotulos[i]);
  }
  free(t->rotulos);
  tabela_inicializar(t);
}

int tabela_obter_id(TabelaClasses *t, const char *rotulo)
{
  // se o rótulo já existe, devolve o id dele
  for (int i = 0; i < t->quantidade; i++)
  {
    if (strcmp(t->rotulos[i], rotulo) == 0)
    {
      return i;
    }
  }

  // rótulo novo: aumenta o vetor se estiver cheio
  if (t->quantidade == t->capacidade)
  {
    int nova = (t->capacidade == 0) ? 16 : t->capacidade * 2;
    char **tmp = realloc(t->rotulos, (size_t)nova * sizeof(char *));
    if (tmp == NULL)
    {
      return -1;
    }
    t->rotulos = tmp;
    t->capacidade = nova;
  }

  char *copia = copiar_texto(rotulo);
  if (copia == NULL)
  {
    return -1;
  }
  t->rotulos[t->quantidade] = copia;
  return t->quantidade++;   // devolve o id novo e depois incrementa
}


// Rótulo a partir do nome da imagem
//
// REGRA ADOTADA: a classe é o trecho do nome ANTES do primeiro '_'.
//   "gato_001.pgm" -> "gato"
// Se o nome não tiver '_', usa o nome sem a extensão ("gato.pgm" -> "gato").
// Se o dataset usar outro padrão de nomes, é só mudar esta função.

static void extrair_rotulo(const char *nome, char *rotulo, size_t tam)
{
  const char *fim = strchr(nome, '_');   // primeiro '_'
  if (fim == NULL)
  {
    fim = strrchr(nome, '.');            // senão, o último '.'
  }

  size_t n = (fim != NULL) ? (size_t)(fim - nome) : strlen(nome);
  if (n >= tam)
  {
    n = tam - 1;   // evita estourar o buffer
  }
  memcpy(rotulo, nome, n);
  rotulo[n] = '\0';
}


// Leitura de uma linha de qualquer tamanho (a linha do LBP16 tem 65536 números)
// Retorna 1 se leu uma linha, 0 no fim do arquivo e -1 se faltou memória.

static int ler_linha(FILE *f, char **linha, size_t *capacidade)
{
  size_t tamanho = 0;

  if (*linha == NULL)
  {
    *capacidade = 4096;
    *linha = malloc(*capacidade);
    if (*linha == NULL)
    {
      return -1;
    }
  }
  (*linha)[0] = '\0';

  // fgets lê no máximo (capacidade - tamanho - 1) caracteres de cada vez
  while (fgets(*linha + tamanho, (int)(*capacidade - tamanho), f) != NULL)
  {
    tamanho += strlen(*linha + tamanho);
    if (tamanho > 0 && (*linha)[tamanho - 1] == '\n')
    {
      return 1;   // linha completa
    }

    // a linha não coube: dobra o buffer e continua lendo
    size_t nova = *capacidade * 2;
    char *tmp = realloc(*linha, nova);
    if (tmp == NULL)
    {
      return -1;
    }
    *linha = tmp;
    *capacidade = nova;
  }

  return (tamanho > 0) ? 1 : 0;   // última linha pode não ter '\n'
}

// Garante espaço para mais um exemplo na base (dobra os vetores se cheios).
static int garantir_capacidade(Base *base)
{
  if (base->quantidade < base->capacidade)
  {
    return 1;
  }

  int nova = (base->capacidade == 0) ? 64 : base->capacidade * 2;

  char **nomes = realloc(base->nomes, (size_t)nova * sizeof(char *));
  if (nomes == NULL)
  {
    return 0;
  }
  base->nomes = nomes;

  int *classes = realloc(base->classes, (size_t)nova * sizeof(int));
  if (classes == NULL)
  {
    return 0;
  }
  base->classes = classes;

  double **vetores = realloc(base->vetores, (size_t)nova * sizeof(double *));
  if (vetores == NULL)
  {
    return 0;
  }
  base->vetores = vetores;

  base->capacidade = nova;
  return 1;
}

// Interpreta UMA linha: "nome largura altura vizinhos h[0] ... h[N-1]"
// e acrescenta o exemplo na base. Retorna 1 em sucesso e 0 em erro.
static int processar_linha(const char *linha, int numero_linha, const char *caminho, Base *base, TabelaClasses *tabela)
{
  // linha em branco: ignora
  const char *inicio = linha;
  while (isspace((unsigned char)*inicio))
  {
    inicio++;
  }
  if (*inicio == '\0')
  {
    return 1;
  }

  // parte fixa: nome, largura, altura, vizinhos (%n guarda quantos caracteres foram lidos)
  char nome[256];
  int largura, altura, viz;
  int consumidos = 0;
  if (sscanf(linha, "%255s %d %d %d%n", nome, &largura, &altura, &viz, &consumidos) != 4 || viz < 1 || viz > 16)
  {
    fprintf(stderr, "Erro: formato inválido na linha %d do arquivo %s\n", numero_linha, caminho);
    return 0;
  }

  // número de características = 2^vizinhos (256 para 8, 65536 para 16)
  int dim = 1 << viz;
  if (base->dim == 0)
  {
    base->dim = dim;
  }
  else if (base->dim != dim)
  {
    fprintf(stderr, "Erro: linha %d de %s tem número de vizinhos diferente das anteriores.\n", numero_linha, caminho);
    return 0;
  }

  if (!garantir_capacidade(base))
  {
    fprintf(stderr, "Erro de alocação de memória.\n");
    return 0;
  }

  double *vetor = malloc((size_t)dim * sizeof(double));
  if (vetor == NULL)
  {
    fprintf(stderr, "Erro de alocação de memória.\n");
    return 0;
  }

  // lê os 'dim' valores do histograma, um a um
  const char *p = linha + consumidos;
  for (int j = 0; j < dim; j++)
  {
    char *fim;
    vetor[j] = strtod(p, &fim);
    if (fim == p)   // não conseguiu converter: faltam valores
    {
      fprintf(stderr, "Erro: linha %d de %s deveria ter %d valores.\n", numero_linha, caminho, dim);
      free(vetor);
      return 0;
    }
    p = fim;
  }

  // não pode sobrar nada depois dos 'dim' valores
  while (isspace((unsigned char)*p))
  {
    p++;
  }
  if (*p != '\0')
  {
    fprintf(stderr, "Erro: linha %d de %s tem valores a mais (esperados %d).\n", numero_linha, caminho, dim);
    free(vetor);
    return 0;
  }

  // classe (id) e cópia do nome
  char rotulo[128];
  extrair_rotulo(nome, rotulo, sizeof(rotulo));
  int id = tabela_obter_id(tabela, rotulo);
  char *copia_nome = copiar_texto(nome);
  if (id < 0 || copia_nome == NULL)
  {
    fprintf(stderr, "Erro de alocação de memória.\n");
    free(vetor);
    free(copia_nome);   // free(NULL) é seguro
    return 0;
  }

  // tudo certo: guarda o exemplo
  int i = base->quantidade;
  base->nomes[i] = copia_nome;
  base->classes[i] = id;
  base->vetores[i] = vetor;
  base->quantidade++;
  return 1;
}

void base_liberar(Base *base)
{
  for (int i = 0; i < base->quantidade; i++)
  {
    free(base->nomes[i]);
    free(base->vetores[i]);
  }
  free(base->nomes);
  free(base->classes);
  free(base->vetores);

  base->nomes = NULL;
  base->classes = NULL;
  base->vetores = NULL;
  base->quantidade = 0;
  base->capacidade = 0;
  base->dim = 0;
}

int base_ler(const char *caminho, Base *base, TabelaClasses *tabela)
{
  base->nomes = NULL;
  base->classes = NULL;
  base->vetores = NULL;
  base->quantidade = 0;
  base->capacidade = 0;
  base->dim = 0;

  FILE *f = fopen(caminho, "r");
  if (f == NULL)
  {
    fprintf(stderr, "Erro: não foi possível abrir o arquivo: %s\n", caminho);
    return 0;
  }

  char *linha = NULL;
  size_t cap_linha = 0;
  int numero_linha = 0;
  int ok = 1;
  int r = 0;

  // lê o arquivo linha a linha; cada linha é um exemplo
  while (ok && (r = ler_linha(f, &linha, &cap_linha)) == 1)
  {
    numero_linha++;
    ok = processar_linha(linha, numero_linha, caminho, base, tabela);
  }
  if (ok && r == -1)
  {
    fprintf(stderr, "Erro de alocação de memória.\n");
    ok = 0;
  }

  free(linha);
  fclose(f);

  if (ok && base->quantidade == 0)
  {
    fprintf(stderr, "Erro: o arquivo %s não contém nenhum exemplo.\n", caminho);
    ok = 0;
  }

  if (!ok)
  {
    base_liberar(base);   // não deixa nada alocado em caso de erro
    return 0;
  }
  return 1;
}