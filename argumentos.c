#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argumentos.h"

static void imprimir_uso(const char *programa)
{
  fprintf(stderr, "Uso: %s -t <treinamento> -v <teste> -k <k> [-n <min-max|z-score>] [-d]\n", programa);
}

int parse_argumentos(int argc, char **argv, Argumentos *args)
{
  // valores iniciais: 0/NULL significam "não informado"
  args->k = 0;
  args->treino = NULL;
  args->teste = NULL;
  args->normalizacao = NORM_NENHUMA;
  args->detalhado = 0;

  for (int i = 1; i < argc; i++)   // argv[0] é o nome do programa
  {
    if (strcmp(argv[i], "-k") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Erro: A opção -k exige um argumento.\n");
        return 0;
      }
      // strtol detecta lixo no número (ex: "5abc"); fim aponta onde a conversão parou
      char *fim;
      long valor = strtol(argv[++i], &fim, 10);
      if (*fim != '\0' || valor < 1 || valor > 1000000)
      {
        fprintf(stderr, "Erro: valor inválido para -k. Use um inteiro positivo.\n");
        return 0;
      }
      args->k = (int)valor;
    }
    else if (strcmp(argv[i], "-t") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Erro: A opção -t exige um argumento.\n");
        return 0;
      }
      args->treino = argv[++i];
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Erro: A opção -v exige um argumento.\n");
        return 0;
      }
      args->teste = argv[++i];
    }
    else if (strcmp(argv[i], "-n") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Erro: A opção -n exige um argumento.\n");
        return 0;
      }
      const char *metodo = argv[++i];
      if (strcmp(metodo, "min-max") == 0)
      {
        args->normalizacao = NORM_MINMAX;
      }
      else if (strcmp(metodo, "z-score") == 0)
      {
        args->normalizacao = NORM_ZSCORE;
      }
      else
      {
        fprintf(stderr, "Erro: valor inválido para -n. Valores permitidos: min-max ou z-score.\n");
        return 0;
      }
    }
    else if (strcmp(argv[i], "-d") == 0)
    {
      args->detalhado = 1;   // opção sem valor: não usa ++i
    }
    else
    {
      fprintf(stderr, "Erro: Opção desconhecida '%s'.\n", argv[i]);
      return 0;
    }
  }

  // -k, -t e -v são obrigatórios; -n e -d são opcionais
  if (args->k == 0 || args->treino == NULL || args->teste == NULL)
  {
    imprimir_uso(argv[0]);
    return 0;
  }

  return 1;
}