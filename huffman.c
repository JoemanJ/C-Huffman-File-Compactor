#include "huffman.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
  Node *esq;    // Nó filho à esquerda
  Node *dir;    // Nó filho à direita
  int peso;     // Peso do nó
  byte c;       // caractere representado
  Node *in_esq; // Codificação para bit 0
  Node *in_dir; // Codificação para bit 1
};

// retorna uma arvore vazia (NULL).
Node *criaArvore() { return NULL; }

// Libera uma arvore recursivamente
Node *liberaArvore(Node *a) {
  if (a == NULL) {
    return a;
  }

  if (a->esq != NULL) {
    liberaArvore(a->esq);
  }

  if (a->in_esq != NULL) {
    liberaArvore(a->in_esq);
  }

  if (a->in_dir != NULL) {
    liberaArvore(a->in_dir);
  }

  if (a->dir != NULL) {
    liberaArvore(a->dir);
  }

  free(a);
}

// Cria um no folha para o caracter "c" com o peso "p" e o retorna.
Node *criaNo(byte c, int p) {
  Node *no = (Node *)malloc(sizeof(Node));

  no->c = c;
  no->peso = p;

  no->esq = NULL;
  no->dir = NULL;
  no->in_esq = NULL;
  no->in_dir = NULL;

  return no;
}

/*
Cria um no codificador com "a" e "b" como filhos internos
(in_esq e in_dir, respectivamente) e a soma de seus pesos e o retorna.
*/
Node *criaNoCodificador(Node *a, Node *b) {
  Node *n = criaNo((byte)255, a->peso + b->peso);
  n->in_esq = a;
  n->in_dir = b;

  return n;
}

// retorna o byte armazenado em "no"
byte getByteNo(Node *no) { return no->c; }

/*
Retorna 1 se "no" for um no folha, ou 0 caso contrario
(incluindo "no" = NULL).
*/
int ehNoFolha(Node *no) {
  if (no == NULL || no->esq != NULL || no->dir != NULL) {
    return 0;
  }

  return 1;
}

/*
Retorna 1 se "no" nao tiver filhos internos, ou 0 caso contrario
(incluindo "no" = NULL).
*/
int ehNoFolhaInterno(Node *no) {
  if (no == NULL || no->in_esq != NULL || no->in_dir != NULL) {
    return 0;
  }

  return 1;
}

/*
Insere "no" na arvore "raiz", ordenado por peso, e retorna "raiz" com
o "no" inserido.
*/
Node *insereNo(Node *raiz, Node *no) {
  // Caso a arvore esteja vazia, ela se torna o proprio no
  if (raiz == NULL) {
    return no;
  }

  // Caso o no tenha um peso menor, insere na esquerda
  if (no->peso < raiz->peso) {
    raiz->esq = insereNo(raiz->esq, no);
    return raiz;
  }

  // Caso contrario (peso maior ou igual), insere na direita
  raiz->dir = insereNo(raiz->dir, no);
  return raiz;
}

/*
Retira o no "no" da arvore "raiz" e retorna "raiz" com "no" removido.
Note que "no" deve ser um ponteiro para o no a ser retirado, e nao o
caractere guardado por aquele no, visto que a implementacao do
codificador nao impede a existencia de multiplos nos com o mesmo
caractere, especificamente os nos codificadores.
*/
Node *retiraNo(Node *raiz, Node *alvo) {
  if (raiz == NULL) {
    printf("No nao encontrado\n");
    return raiz;
  }

  // Peso menor
  if (alvo->peso < raiz->peso) {
    raiz->esq = retiraNo(raiz->esq, alvo);
    return raiz;
  }

  // Peso maior
  if (alvo->peso > raiz->peso) {
    raiz->dir = retiraNo(raiz->dir, alvo);
    return raiz;
  }

  // Peso igual, mas nao eh o no alvo
  if (raiz != alvo) {
    raiz->dir = retiraNo(raiz->dir, alvo);
    return raiz;
  }

  // Achou o no alvo
  // E um no folha
  if (raiz->esq == NULL && raiz->dir == NULL) {
    return NULL;
  }

  // Tem apenas filhos a esquerda
  if (raiz->esq != NULL && raiz->dir == NULL) {
    return raiz->esq;
  }

  // Tem apenas filhos a direita
  if (raiz->dir != NULL && raiz->esq == NULL) {
    return raiz->dir;
  }

  // Tem 2 filhos.
  /*Vale ressaltar que devido a natureza do algoritmo para codificacao Huffman,
    este e um caso que nunca acontece, uma vez que os unicos nos retirados de
    uma arvore sao os de menor peso, que nao possuem filhos a esquerda, porem
    este caso foi implementado por questao de completude*/
  // Acha o no substituto
  Node *sub = menorPesoArvore(raiz->dir);

  // Troca o conteudo  com o no substituto
  byte auxC = raiz->c;
  int auxPeso = raiz->peso;
  raiz->c = sub->c;
  raiz->peso = sub->peso;
  sub->c = auxC;
  sub->peso = auxPeso;

  return retiraNo(sub, sub);
}

/*
Retorna um ponteiro para o no de menor peso na arvore "raiz"
ou NULL caso a arvore esteja vazia.

Retorna NULL caso "raiz" seja NULL.
*/
Node *menorPesoArvore(Node *raiz) {
  if (raiz == NULL) {
    printf("Arvore vazia\n");
    return NULL;
  }

  if (raiz->esq == NULL) {
    return raiz;
  }
  return menorPesoArvore(raiz->esq);
}

/*
Retira os dois menores nos ("a" e "b") de "raiz", cria um novo no folha
"c" que tem "a" e "b" como filhos internos e tem a soma de seus pesos,
e encadeia "c" em raiz. Retorna "raiz" apos essas modificacoes.
Tambem "limpa" os campos "esq" e "dir" de "a" e "b" para impedir
refenencias circulares.

Retorna a propria arvore caso tenha apenas 1 elemento.
Retorna a arvore NULL caso "raiz" seja NULL.
*/
Node *reduzArvore(Node *raiz) {
  if (ehNoFolha(raiz)) {
    printf("Impossivel reduzir arvore com 1 elemento\n");
    return raiz;
  }

  if (raiz == NULL) {
    printf("Arvore vazia\n");
    return NULL;
  }

  Node *menor1 = menorPesoArvore(raiz);
  raiz = retiraNo(raiz, menor1);
  menor1->esq = NULL;
  menor1->dir = NULL;
  Node *menor2 = menorPesoArvore(raiz);
  raiz = retiraNo(raiz, menor2);
  menor2->esq = NULL;
  menor2->dir = NULL;

  Node *novo = criaNoCodificador(menor1, menor2);

  raiz = insereNo(raiz, novo);

  return raiz;
}

/*
Imprime uma arvore recursivamente no formato:
      <<esq><in_esq>[(char) (peso)]<in_dir><dir>>
*/
void imprimeArvore(Node *raiz) {
  printf("<");

  if (raiz == NULL) {
    printf(">");
    return;
  }

  imprimeArvore(raiz->esq);
  imprimeArvore(raiz->in_esq);
  printf("[(%c) (%d)]", raiz->c, raiz->peso);
  imprimeArvore(raiz->in_dir);
  imprimeArvore(raiz->dir);
  printf(">");
}

/*
Imprime uma arvore recursivamente no formato:
      <<in_esq>[(char) (peso)]<in_dir>>
*/
void imprimeCodificacao(Node *raiz) {
  printf("<");

  if (raiz == NULL) {
    printf(">");
    return;
  }

  imprimeCodificacao(raiz->in_esq);
  printf("[(%c) (%d)]", raiz->c, raiz->peso);
  imprimeCodificacao(raiz->in_dir);
  printf(">");
}

/*
Substitui o primeiro caractere '\0' em "str" pela string "app" e adiciona
um '\0' apos "app".
*/
static void strAppend(char *str, char *app) {
  int i = 0;
  while (str[i] != '\0') {
    i++;
  }

  int j = 0;
  for (j = 0; app[j] != '\0'; j++, i++) {
    str[i] = app[j];
  }

  str[i] = '\0';
}

// Inverte a string "str".
static void strInv(char *str) {
  int len = 0;
  while (str[len] != '\0') {
    len++;
  }

  if (len < 2) {
    return;
  }

  int i = 0;
  for (i = 0; i < (len / 2); i++) {
    char aux = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = aux;
  }
}

/* Escreve a codificacao em "str", porem na ordem inversa (do no folha
ate a raiz)
*/
static int codificaByte_(Node *a, char str[128], byte b) {
  if (a == NULL) {
    return 0;
  }

  if (codificaByte_(a->in_esq, str, b)) { //se o caminho for pra esquerda 
    strAppend(str, "0");
    return 1;
  }

  if (codificaByte_(a->in_dir, str, b)) { 
    strAppend(str, "1");
    return 1;
  }

  if (a->c == b) {
    return 1;
  }

  return 0;
}

/*
Escreve no vetor "str" a sequencia de bits (como 'char's '0' ou '1') que
codifica o byte "b" segundo os caminhos internos da arvore "a".
*/
void codificaByte(Node *a, char str[128], byte b) {
  // Preenche "str" com '\0'
  int i = 0;
  for (i = 0; i < 128; i++) {
    str[i] = '\0';
  }

  // Escreve a codificacao invertida e inverte para corrigir
  codificaByte_(a, str, b);
  strInv(str);
}

/*
Escreve o caractere "b" como bit menos significativo do byte "B".
"b" deve ser '0'(char) ou 0b00 para um bit '0' ou '1'(char) ou 0b01 para
um bit '1'
*/
byte pushBit(byte B, char b) {
  switch (b) {
  case 0b00:
  case '0':
    B = B << 1;
    B = B & 0b11111110;
    break;

  case 0b01:
  case '1':
    B = B << 1;
    B = B | 0b00000001;
    break;

  default:
    printf("pushBit:\tchar invalido. use '0' ou '1'\n");
  }

  return B;
}

/*
Popula "vet" com o numero de ocorrencias de cada byte no  arquivo de
entrada "arquivo".
*/
void contaBytes(byteCount vet, FILE *arquivo) {
  byte c = 'a';

  // Le todo o arquivo, byte por byte
  while (fread(&c, sizeof(byte), 1, arquivo) == 1) {
    /*Um byte (unsigned char) contendo um caractere e igual a um int com o
      codigo da tabela ascii (em decimal) daquele caractere.
      Verificacao:
      unsigned char c = '2';
      int i = 50;
      if (c == i){ printf("sao iguais"); }
    */

    vet[c]++;
  }
}

/*
Escreve o byte "b" no arquivo de saida "arq".
Obs: verifique que "arq" esta aberto para escrita em binario antes de usar
essa funcao! Sujeito a resultados inesperados!
*/
void writeByte(FILE *arq, byte b) { fwrite(&b, sizeof(byte), 1, arq); }

/*
Le um byte do arquivo de entrada "arq";
Obs: verifique que "arq" esta aberto para leitura em binario antes de usar
essa funcao! Sujeito a resultados inesperados!
*/
byte readByte(FILE *arq) {
  byte ret;
  fread(&ret, sizeof(byte), 1, arq);

  return ret;
}

/*
imprime os bits do byte "b" como caracteres em stdout
*/
void printByte(byte b) {
  int i = 0;
  for (i = 0; i < 8; i++) {
    if (((b << i) & 0b10000000) == 0b10000000) {
      printf("1");
    } else {
      printf("0");
    }
  }
}

/*
Escreve a codificacao da arvore "a" no bitmap "bm" no seguinte formato:
→ A arvore e escrita com notacao pre-order;
→ Um bit '0' representa um no galho;
→ Um bit '1' representa um no folha;
→ Os 8 bits apos um bit '1' representam o caractere representado por aquele
  no folha.
*/
void fillHeader(bitmap *bm, Node *a) {
  // No vazio
  if (a == NULL) {
    return;
  }

  // No folha
  if (a->in_esq == NULL && a->in_dir == NULL) {
    bitmapAppendLeastSignificantBit(bm, 0b1);
    // Escreve os 8 bits do byte guardade em "a"
    int i = 0;
    byte B = a->c;
    for (i = 0; i < 8; i++) {
      if ((B << i) & 0b10000000) { // Se o bit "(8-i)" for '1', append 1
        bitmapAppendLeastSignificantBit(bm, 0b01);
      } else { // Caso contrario (bit 0), append 0
        bitmapAppendLeastSignificantBit(bm, 0b00);
      }
    }
    return;
  }

  // No galho
  bitmapAppendLeastSignificantBit(bm, 0b00);
  fillHeader(bm, a->in_esq);
  fillHeader(bm, a->in_dir);
}

/*
Le a arvore codificada no cabecalho do arquivo "arq" e retorna a arvore
em sua forma original.
Atualiza os valores de "readerByte" e "RBi" por referencia para que a
leitura do arquivo possa ser feita linearmente caso a arvore codificada
nao ocupe um numero inteiro de bytes.

OBS: Tenha certeza de ter lido o indicador de fim de arquivo guardado no
primeiro byte do arquivo de codificacao antes de chamar esta funcao!
*/
Node *descompactaArvore(FILE *arq, byte *readerByte, int *RBi) {
  byte bit = (*readerByte << *RBi) & 0b10000000; // Le um bit
  (*RBi)++;

  // Caso tenha lido todo o byte, le um byte novo de "arq"
  if (*RBi == 8) {
    *readerByte = readByte(arq);
    *RBi = 0;
  }

  /*
    No caso de bit 0, cria um no "no", le os proximos 2 nos, encadeia em "no"
    e retorna "no"
  */
  if (bit == 0b00) {
    Node *no = criaNo(255, 0);
    no->in_esq = descompactaArvore(arq, readerByte, RBi);
    no->in_dir = descompactaArvore(arq, readerByte, RBi);
    return no;
  }

  /*
    Caso contrario (bit 1), retorna um no folha com o proximo byte do arquivo
    armazenado
  */
  byte c;
  int i = 0;
  for (i = 0; i < 8; i++) {
    c = pushBit(c, ((*readerByte << *RBi) & 0b10000000) >> 7);
    (*RBi)++;
    if (*RBi == 8) {
      *readerByte = readByte(arq);
      *RBi = 0;
    }
  }

  return criaNo(c, 0);
}

/*
retorna "a->in_esq" caso "path" seja 1;
retorna "a->in_dir" caso "path" seja 0;
retorna "a" em outros casos.
*/
Node *retornaSubArvore(Node *a, char path) {
  switch (path) {
  case '0':
    return a->in_esq;

  case '1':
    return a->in_dir;

  default:
    return a;
  }
}