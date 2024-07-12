/*

Alan Medina Wehbe (2019108956) e Diogo Delazare Brandão (2021100169)

Referências:

1 - Inicializacao de vetor com valor 0:
https://stackoverflow.com/questions/2589749/how-to-initialize-array-to-0-in-c
(acesso em 28/01/2023)

*/

#include <stdio.h>
#include <string.h>

#include "bitmap.h"
#include "huffman.h"

int main(int argc, char **argv) {
  // Mensagem de erro caso os argumentos de linha de comando sejam invalidos
  if ((argc != 3) ||
      (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-d") != 0)) {
    printf("Modo de uso:\n\tcompactar um arquivo:\n\t\t./huffman -c "
           "arquivo-de-entrada\n\n\tdescompactar um arquivo "
           ".huffman\n\t\t./huffman -d arquivo-de-entrada\n");
    return 0;
  }

  // Modo compactador
  if (strcmp(argv[1], "-c") == 0) {
    FILE *arq = fopen(argv[2], "rb");
    if (arq == NULL) {
      printf("Nao foi possivel abrir o arquivo \"%s\"\n", argv[2]);
      return (0);
    }

    /*O compilador inicializa os campos nao inicializados do vetor com 0
      caso ao menos um deles tenha sido inicializado na declaracao
      (referencia 1)*/
    byteCount vet = {0};

    // Conta as ocorrencias de cada caractere no arquivo de entrada
    contaBytes(vet, arq);
    fclose(arq);

    printf("Gerando codificacao de compactacao... ");

    // Arvore para guardar a codificacao dos bytes
    Node *arvCod = criaArvore();

    // Contador de bytes distintos
    int diffBytes = 0;

    // Para cada caractere possivel...
    int i = 0;
    for (i = 0; i < 256; i++) {
      // Se esse caractere ocorrer no arquivo...
      if (vet[i]) {
        diffBytes++;

        // Cria um no com esse caractere e insere em arvCod
        Node *no = criaNo((byte)i, vet[i]);
        arvCod = insereNo(arvCod, no);
      }
    }

    byte eof = '\0';
    // Encontra uma sequencia de bits nao usada para indicar fim de arquivo
    for (i = 0; i < 256; i++) {
      if (vet[i] == 0) {
        eof = (byte)i;
        Node *no = criaNo(eof, 1);
        arvCod = insereNo(arvCod, no);
        break;
      }
    }

    // Enquanto houver mais de um elemento em arvCod...
    while (!ehNoFolha(arvCod)) {
      // faz a iteracao para codificacao de Huffman
      arvCod = reduzArvore(arvCod);
    }

    printf("pronto!\nGerando arquivo de saida...\n\tEscrevendo "
           "cabecalho... ");
    // Cria arquivo de saida
    char outFileName[strlen(argv[2]) + 4];
    outFileName[0] = '\0';
    strcat(outFileName, argv[2]);
    strcat(outFileName, ".hff");
    FILE *output = fopen(outFileName, "wb");

    byte B = 0;   // Byte a ser escrito no arquivo de saida
    int Bi = 0;   // Quantidade de bits escritos em "B"
    char cod = 0; // Codificacao do caractere a ser escrito no arquivo
    char caminho[128] = {0}; // string de codificadao de "cod"

    // Escrever o indicador de fim de arquivo primeiro
    writeByte(output, eof);

    // Salvar a arvore de codificacao no cabecalho do arquivo...
    /*
      Tamanho maximo do cabecalho:
      128 bits para nos folha +
      64 + 32 + 16 + 8 + 4 + 2 + 1 = 127 bits para nos galho +
      8 * 128 = 1024 bits para caracteres representados

      total: 1279 bits
    */
    bitmap *header = bitmapInit(1279);
    fillHeader(header, arvCod);

    /*
    A linguagem C (assim como a maioria dos sistemas operacionais) nao permite
    a escrita de um unico bit em um arquivo. O minimo que pode ser escrito em
    um arquivo sao 8 bits (1 byte).
    "witerByte" funciona como um buffer de bits. Preenchemos esse buffer com
    os bits que queremos escrever no arquivo de saida, e quando todo o byte
    tiver sido preenchido (indicado pro WBi == 8) escrevemos esse byte no
    arquivo.
    */
    byte writerByte = 0; // Byte a ser escrito no arquivo de saida
    int WBi = 0;         // numero de bits escritos em "writerByte"

    // Escreve o cabecaçalho
    for (i = 0; i < bitmapGetLength(header); i++) {
      // "Empurra" os bits do bitmap "header" para "writerByte" um por um
      writerByte = pushBit(writerByte, bitmapGetBit(header, i));
      WBi++;

      // Se tiver um byte completo, escreve ele no arquivo
      if (WBi == 8) {
        writeByte(output, writerByte);
        WBi = 0;
      }
    }
    bitmapLibera(header);

    printf("pronto!\n\tEscrevendo codificacao do arquivo de entrada... ");

    // Volta ao inicio do arquivo de entrada para fazer a leitura dos caracteres
    arq = fopen(argv[2], "rb");
    fseek(arq, 0, SEEK_SET);

    byte c; // byte lido do arquivo de entrada

    while (fscanf(arq, "%c", &c) == 1) { // Ate o fim do arquivo
      char path[128] = {0};
      codificaByte(arvCod, path, c);

      int i = 0;
      while (path[i] != '\0') { // Ate o fim do codigo do byte lido
        writerByte = pushBit(writerByte, path[i]);
        WBi++;
        i++;

        if (WBi == 8) { // Temos 1 byte completo, podemos escrever no arquivo
          writeByte(output, writerByte);
          WBi = 0;
        }
      } // Escreveu todo o byte lido
    }   // Acabou o arquivo

    printf("pronto!\n\tEscrevendo indicador de fim de arquivo... ");
    // Escreve o indicador de fim de arquivo
    char path[128] = {0};
    codificaByte(arvCod, path, eof);
    i = 0;

    while (path[i] != '\0') {
      writerByte = pushBit(writerByte, path[i]);
      WBi++;
      i++;

      if (WBi == 8) {
        writeByte(output, writerByte);
        WBi = 0;
      }
    }

    printf("pronto!\n\tFinalizando escrita... ");
    /*
      Caso nao tenha dado um numero exato de bytes, preenche o
      ultimo byte com 0
    */
    if (WBi != 0) {
      while (WBi < 8) {
        writerByte = pushBit(writerByte, '0');
        WBi++;
      }
      writeByte(output, writerByte);
    }

    printf("pronto!\nArquivo de saida gerado.\nCompactacao completa.\n");

    liberaArvore(arvCod);
    fclose(arq);
    fclose(output);
  }

  // Modo descompactador
  else {
    // Verifica se o arquivo tem a extensao .hff
    if (strlen(argv[2]) < 5 || // Tamanho de nome de arquivo muito pequeno
        strcmp(&(argv[2][strlen(argv[2]) - 4]), ".hff") != 0) {
      printf("Extensao do arquivo invalida.\nTenha certeza que o arquivo foi "
             "compactado por este programa.\n");
    }

    printf("Lendo informacao compactacao... ");

    FILE *arq = fopen(argv[2], "rb");

    /*
      Le o byte do indicador de fim de arquivo do arquivo compactado,
      salvo como o primeiro byte do arquivo
    */
    byte eof;
    fread(&eof, sizeof(byte), 1, arq);

    // Arvore de codificacao
    Node *arvCod = criaArvore();

    byte readerByte = 0; // Buffer de leitura de bits, analogo a "writerByte"
    int RBi = 0;         // Numero de bits lidos de "readerByte"

    // Ler descricao de "descompactaArvore()"
    readerByte = readByte(arq);

    arvCod = descompactaArvore(arq, &readerByte, &RBi);

    printf("Pronto!\nDescompactando... ");

    // Gerando arquivo de saida
    char outFileName[strlen(argv[2]) + 1];
    strcpy(outFileName, argv[2]);
    outFileName[strlen(argv[2]) - 4] = '\0'; // Retira a extensao ".hff"
    FILE *output = fopen(outFileName, "wb");

    /*
      Ponteiro que vai percorrer os caminhos da arvore como indicado
      pelos bits do arquivo de entrada.
      O nome dele eh Lucas e ele adora bananas nanicas e pular entre os
      galhos da arvore.
    */
    Node *Lucas_ONoMacaquinho = arvCod;

    // Loop que so vai encerrar com o fim da descompactacao
    while (1) {
      byte bit = (readerByte << RBi) & 0b10000000; // Le um bit
      (RBi)++;

      // Caso tenha lido todo o byte, le um byte novo do arquivo de entrada
      if (RBi == 8) {
        readerByte = readByte(arq);
        RBi = 0;
      }

      // Percorre em arvCod o caminho indicado pelo bit o arquivo de entrada
      if (bit == 0b10000000) {
        Lucas_ONoMacaquinho = retornaSubArvore(Lucas_ONoMacaquinho, '1');
      } else {
        Lucas_ONoMacaquinho = retornaSubArvore(Lucas_ONoMacaquinho, '0');
      }

      // Se chegou em um no folha, escreve o byte guardado naquele no
      if (ehNoFolhaInterno(Lucas_ONoMacaquinho)) {
        byte B = getByteNo(Lucas_ONoMacaquinho);

        // Se eh o byte que indica fim de arquivo, encerra a descompactacao
        if (B == eof) {
          fclose(output);
          liberaArvore(arvCod);

          printf("Pronto!\nArquivo descompactado com sucesso\n");
          return 0;
        } else { // Caso contrario, escreve o byte
          writeByte(output, B);
          Lucas_ONoMacaquinho = arvCod; // Retorna para a raiz da arvore
        }
      }
    }
  }
}

/*
OBS: Lucas foi recompensado com bananas por seu trabalho bem feito.
Obrigado Lucas por seu valioso serviço prestado a este programa.

''---.  .-.   .--,;._
  '   `-\  \-' ./|\\ `'-.
  .-----,| |--'/ | \\    \
 /     | | |  /  |  \\   /
       | | | /   |   \`'`
       ) | | |  /\   /                   ,,
      / ;| | `-'  `'`                     \`-.
     / / | |                   ,'          |   \
 \_.' /  ; |            .-'-.,','           |  '.
 ,_.' ,-'  ;               ) )/|            |   |
    ,'      \         __ ,''  ''`.         _|   |--,.
   /         `.     ,',./ .-.-.   \.--.  / _) _,'--. )
  |       ---..`.._| |   /__|__\   ,-. \ |`/ ( `--. _)
  \            `.  '\_;  ((o|o))    ,' | \<_, `'-, )
   `.            \   ,-''.--'--.''-. _,'  \    __. '.
     `.          |  /    \     /    \     /  ,'   `._)
       \        ,' | .'   `-.-'   '. |   /  /
       /      _,   | |      |      | |../  ,'
     .'  /'--'._,,_ \ \     |     / .'    /
    (_._ '''-._    `.\ `.   |   ,' ,' ___/
        `-.    \      '. `'-'-'` ,'-'`
           \ `. \      |`'-...-'`
           | ; \ |    /  /
           | | | /  ,' .'     ____
          (_/.'-'  (   `.   ,'    `'._
                     `.  `-'     --.. )
                       `.       .--. `\
                         `.__.-\ \  `\_)
                               ( /
                                `
*/