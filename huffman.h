#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bitmap.h"
#include <stdio.h>

// Arvore binaria para codificacao Huffman.
typedef struct node Node;

// Definição de um tipo "byte", uma sequência genérica de 8 bits.
typedef unsigned char byte;

/*
Alias para um vetor usado para contar o número de ocorrênciaa dos 256
bytes possíveis.
*/
typedef int byteCount[256];

// retorna uma arvore vazia (NULL).
Node *criaArvore();

// Libera uma arvore recursivamente
Node* liberaArvore(Node* a);

// Cria um no folha para o caracter "c" com o peso "p" e o retorna.
Node *criaNo(byte c, int p);

/*
Cria um no codificador com "a" e "b" como filhos internos
(in_esq e in_dir, respectivamente) e a soma de seus pesos e o retorna.
*/
Node *criaNoCodificador(Node *a, Node *b);

// retorna o byte armazenado em "no"
byte getByteNo(Node *no);

/*
Retorna 1 se "no" for um no folha, ou 0 caso contrario
(incluindo "no" = NULL).
*/
int ehNoFolha(Node *no);

/*
Retorna 1 se "no" nao tiver filhos internos, ou 0 caso contrario
(incluindo "no" = NULL).
*/
int ehNoFolhaInterno(Node *no);

/*
Insere "no" na arvore "raiz", ordenado por peso, e retorna "raiz" com
o "no" inserido.
*/
Node *insereNo(Node *raiz, Node *no);

/*
Retira o no "no" da arvore "raiz" e retorna "raiz" com "no" removido.
Note que "no" deve ser um ponteiro para o no a ser retirado, e nao o
caractere guardado por aquele no, visto que a implementacao do
codificador nao impede a existencia de multiplos nos com o mesmo
caractere, especificamente os nos codificadores.
*/
Node *retiraNo(Node *raiz, Node *alvo);

/*
Retorna um ponteiro para o no de menor peso na arvore "raiz"
ou NULL caso a arvore esteja vazia.

Retorna NULL caso "raiz" seja NULL.
*/
Node *menorPesoArvore(Node *raiz);

/*
Retira os dois menores nos ("a" e "b") de "raiz", cria um novo no folha
"c" que tem "a" e "b" como filhos internos e tem a soma de seus pesos,
e encadeia "c" em raiz. Retorna "raiz" apos essas modificacoes.
Tambem "limpa" os campos "esq" e "dir" de "a" e "b" para impedir
refenencias circulares.

Retorna a propria arvore caso tenha apenas 1 elemento.
Retorna a arvore NULL caso "raiz" seja NULL.
*/
Node *reduzArvore(Node *raiz);

/*
Imprime uma arvore recursivamente no formato:
      < <esq> <in_esq> char (peso) <in_dir> <dir> >
*/
void imprimeArvore(Node *raiz);

/*
Imprime uma arvore recursivamente no formato:
      < <in_esq> char (peso) <in_dir> >
*/
void imprimeCodificacao(Node *raiz);

/*
Escreve no vetor "str" a sequencia de bits (como 'char's '0' ou '1') que
codifica o byte "b" segundo os caminhos internos da arvore "a".
*/
void codificaByte(Node *a, char str[128], byte b);

/*
Escreve o caractere "b" como bit menos significativo do byte "B".
"b" deve ser '0' para um bit '0' ou '1' para um bit '1'.
Retorna "B" apos a adicao de "b".
Retorna "B" inalterado caso "b" nao seja '0' ou '1'.
*/
byte pushBit(byte B, char b);

/*
Popula "vet" com o numero de ocorrencias de cada byte no  arquivo de
entrada "arquivo".
*/
void contaBytes(byteCount vet, FILE *arquivo);

/*
Escreve o byte "b" no arquivo de saida "arq".
Obs: verifique que "arq" esta aberto para escrita em binario antes de usar
essa funcao! Sujeito a resultados inesperados!
*/
void writeByte(FILE *arq, byte b);

/*
Le um byte do arquivo de entrada "arq";
Obs: verifique que "arq" esta aberto para leitura em binario antes de usar
essa funcao! Sujeito a resultados inesperados!
*/
byte readByte(FILE *arq);

/*
imprime os bits do byte "b" como caracteres em stdout
*/
void printByte(byte b);

/*
Escreve a codificacao da arvore "a" no bitmap "bm" no seguinte formato:
→ A arvore e escrita com notacao pre-order;
→ Um bit '0' representa um no galho;
→ Um bit '1' representa um no folha;
→ Os 8 bits apos um bit '1' representam o caractere representado por aquele
  no folha.
*/
void fillHeader(bitmap *bm, Node *a);

/*
Le a arvore codificada no cabecalho do arquivo "arq" e retorna a
arvore em sua forma original.
Atualiza os valores de "readerByte" e "RBi" por referencia para que a
leitura do arquivo possa ser feita linearmente caso a arvore
codificada nao ocupe um numero inteiro de bytes.

OBS: Tenha certeza de ter lido o indicador de fim de arquivo
guardado no primeiro byte do arquivo de codificacao e guardado
o segundo byte do arquivo em "*readerByte" antes de chamar esta
funcao!
*/
Node *descompactaArvore(FILE *arq, byte *readerByte, int *RBi);

/*
retorna "a->in_esq" caso "path" seja 1;
retorna "a->in_dir" caso "path" seja 0;
retorna "a" em outros casos.
*/
Node *retornaSubArvore(Node *a, char path);

#endif