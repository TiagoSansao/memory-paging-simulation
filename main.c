#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

unsigned int pageSizeInBytes;
unsigned int physicalMemorySizeInBytes;
unsigned int maximumProcessSizeInBytes;

char *physicalMemory;
unsigned int numberOfFrames;
char *frameUsageBitMap;

int allocateFrame()
{
  for (int i = 0; i < numberOfFrames; i++)
  {
    if (frameUsageBitMap[i] == 0)
    {
      frameUsageBitMap[i] = 1;
      return i;
    }
  }

  return -1;
}

void copyPageContentToFrame(char *processVirtualMemory, int pageIndex, int allocatedFrameIndex)
{
  int pageOffset = (pageIndex * pageSizeInBytes);
  char *pageAddress = processVirtualMemory + pageOffset;

  int frameOffset = allocatedFrameIndex * pageSizeInBytes;
  char *frameAddress = physicalMemory + frameOffset;

  for (int j = 0; j < pageSizeInBytes; j++)
  {

    frameAddress[j] = pageAddress[j];
  }
}

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf("Parametros insuficientes, use: ./a.out {tamanho da página/quadro} {tamanho da memória física} {tamanho máximo de um processo}\n");
    printf("Obs.: passe apenas números, serão lidos como bytes.\n");
    exit(0);
  }

  pageSizeInBytes = atoi(argv[1]);
  physicalMemorySizeInBytes = atoi(argv[2]);
  maximumProcessSizeInBytes = atoi(argv[3]);

  physicalMemory = malloc(physicalMemorySizeInBytes * sizeof(char));

  numberOfFrames = physicalMemorySizeInBytes / pageSizeInBytes;
  frameUsageBitMap = calloc(numberOfFrames, sizeof(char)); // o calloc inicializa os valores zerado

  while (1)
  {
    printf("--- Gerenciador de memória ---\n");
    printf("[1] Visualizar memória\n");
    printf("[2] Criar processo\n");
    printf("[3] Visualizar tabela de páginas\n");
    printf("[4] Sair\n");

    int promptAnswer;

    scanf("%d", &promptAnswer);

    switch (promptAnswer)
    {
    case 1:
      break;
    case 2:
      int identifier, memoryNeededInBytes;
      bool validAnswer = false;

      while (!validAnswer)
      {
        printf("Digite um inteiro identificador do processo e a memória que ele precisará em bytes, separados por espaço. Ex.: \"1 512\"\n");
        scanf("%d %d", &identifier, &memoryNeededInBytes);

        if (memoryNeededInBytes > maximumProcessSizeInBytes)
        {
          printf("A memória solicitada ao processo não pode ser maior que a memória máxima definida na inicialização do programa [%d Bytes].\n", maximumProcessSizeInBytes);
          validAnswer = false;
        }

        validAnswer = true;
      }

      char *processVirtualMemory = malloc(sizeof(char) * memoryNeededInBytes);

      int numberOfPages = memoryNeededInBytes / pageSizeInBytes;

      int *pageMap = malloc(sizeof(int) * numberOfPages);

      for (int pageIndex = 0; pageIndex < numberOfPages; pageIndex++)
      {
        int allocatedFrameIndex = allocateFrame();
        if (allocatedFrameIndex == -1)
        {
          printf("[ERRO] Não foi possível alocar inteiramente o processo na memória física, pois todos os quadros estão sendo utilizados\n");
          printf("E este gerenciador de memória não tem algoritmo de substituição :/\n");
          break;
        }

        copyPageContentToFrame(processVirtualMemory, pageIndex, allocatedFrameIndex);

        pageMap[pageIndex] = allocatedFrameIndex;
      }

      break;
    case 3:

      break;
    case 4:
      exit(1);
    default:
      printf("Resposta inválida.\n");
      break;
    }
  }

  return 0;
}
