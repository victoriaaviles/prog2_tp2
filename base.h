#ifndef BASE_H
#define BASE_H

// tabela com os rótulos (classes) diferentes encontrados. O número de classes
// é descoberto automaticamente: cada rótulo novo ganha o próximo id (0, 1, 2...).
// a mesma tabela é usada para treino e teste, para que o id de uma classe
// seja igual nas duas bases.
typedef struct
{
  char **rotulos;   // rotulos[id] = texto da classe
  int quantidade;   // quantas classes existem
  int capacidade;   // tamanho alocado
} TabelaClasses;

// uma base (treino ou teste) carregada na memória.
typedef struct
{
  char **nomes;      // nomes[i]    = nome da imagem do exemplo i
  int *classes;      // classes[i]  = id da classe do exemplo i
  double **vetores;  // vetores[i][j] = característica j do exemplo i
  int quantidade;    // número de exemplos
  int capacidade;    // tamanho alocado dos vetores acima
  int dim;           // número de características (256 ou 65536 para o LBP)
} Base;

void tabela_inicializar(TabelaClasses *t);
void tabela_liberar(TabelaClasses *t);

// devolve o id do rótulo (criando-o se for novo) ou -1 se faltar memória.
int tabela_obter_id(TabelaClasses *t, const char *rotulo);

// lê um arquivo gerado pelo lbp. Retorna 1 em sucesso e 0 em erro
// (nesse caso nada fica alocado em 'base').
int base_ler(const char *caminho, Base *base, TabelaClasses *tabela);

// libera toda a memória da base.
void base_liberar(Base *base);

#endif