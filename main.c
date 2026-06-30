#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
  int id;
  int numberOfPages;
  int *pageMap;
} ProcessDescriptor;

unsigned int pageSizeInBytes;
unsigned int physicalMemorySizeInBytes;
unsigned int maximumProcessSizeInBytes;

char *physicalMemory;
unsigned int numberOfFrames;
unsigned int numberOfUsedFrames;
char *frameUsageBitMap; // 0 -> not used, 1 -> used

// considerei a alternativa de alocar dinamicamente com maloc + realoc quando necessário incrementar,
// mas acho que daí fugiria muito do objetivo desse trabalho. Segui com um array de tamanho fixo para simplificar
#define MAX_PROCESSES_QTD 100
ProcessDescriptor processDescriptorList[MAX_PROCESSES_QTD];
unsigned int processCounter = 0;

int allocateFrame()
{
  for (int i = 0; i < numberOfFrames; i++)
  {
    if (frameUsageBitMap[i] == 0)
    {
      numberOfUsedFrames++;
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

void fillRandomCharacters(char *address, int maxOffset)
{
  for (int offset = 0; offset < maxOffset; offset++)
  {
    address[offset] = 'A' + (random() % 26);
  }
}

void createNewProcess()
{
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

  ProcessDescriptor *processDescriptor = &processDescriptorList[processCounter];
  (*processDescriptor).id = identifier;

  char *processVirtualMemory = malloc(sizeof(char) * memoryNeededInBytes);
  fillRandomCharacters(processVirtualMemory, memoryNeededInBytes);

  processDescriptor->numberOfPages = memoryNeededInBytes / pageSizeInBytes;

  processDescriptor->pageMap = malloc(sizeof(int) * processDescriptor->numberOfPages);

  for (int pageIndex = 0; pageIndex < processDescriptor->numberOfPages; pageIndex++)
  {
    int allocatedFrameIndex = allocateFrame();
    if (allocatedFrameIndex == -1)
    {
      printf("[ERRO] Não foi possível alocar inteiramente o processo na memória física, pois todos os quadros estão sendo utilizados\n");
      printf("E este gerenciador de memória não tem algoritmo de substituição :/\n");
      return;
    }

    copyPageContentToFrame(processVirtualMemory, pageIndex, allocatedFrameIndex);

    processDescriptor->pageMap[pageIndex] = allocatedFrameIndex;
  }

  processCounter++;
  free(processVirtualMemory);
}

void showCurrentPhysicalMemoryState()
{
  int freeMemoryInBytes = (numberOfFrames - numberOfUsedFrames) * pageSizeInBytes;
  float freeMemoryPercentage = ((float)freeMemoryInBytes / (float)physicalMemorySizeInBytes) * 100;
  printf("Memória física livre: %.2f%%/100%% | %d/%d Bytes\n", freeMemoryPercentage, freeMemoryInBytes, physicalMemorySizeInBytes);
  printf("Visualização dos frames:\n\n");

  for (int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++)
  {
    printf("Frame %d: ", frameIndex);
    for (int frameOffset = 0; frameOffset < pageSizeInBytes; frameOffset++)
    {
      int offsetFromPhysicalMemoryPointer = (frameIndex * pageSizeInBytes) + frameOffset;
      printf("%c", physicalMemory[offsetFromPhysicalMemoryPointer]);
    }
    printf("\n");
  }

  char c;
  printf("\n\n Escreva alguma letra e aperte enter para voltar a tela inicial.\n");
  scanf(" %c", &c);
}

ProcessDescriptor *findProcessDescriptorById(int id)
{
  for (int i = 0; i < MAX_PROCESSES_QTD; i++)
  {
    if (processDescriptorList[i].id == id)
      return &processDescriptorList[i];
  }

  return NULL;
}

void showProcessMemoryState()
{
  int processIdentifier;

  printf("Digite o identificador do processo:\n");
  scanf("%d", &processIdentifier);

  ProcessDescriptor *processDescriptor = findProcessDescriptorById(processIdentifier);
  if (processDescriptor == NULL)
  {
    printf("Processo não encontrado.\n");
    return;
  }

  printf("Tamanho do processo: %d Bytes\n\n", processDescriptor->numberOfPages * pageSizeInBytes);
  printf("Tabela de páginas (página: quadro)\n");

  for (int pageIndex = 0; pageIndex < processDescriptor->numberOfPages; pageIndex++)
  {
    printf("Página %d: %d\n", pageIndex, processDescriptor->pageMap[pageIndex]);
  }

  char c;
  printf("\n\n Escreva alguma letra e aperte enter para voltar a tela inicial.\n");
  scanf(" %c", &c);
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
    printf("\n\n");

    switch (promptAnswer)
    {
    case 1:
      showCurrentPhysicalMemoryState();
      break;
    case 2:
    {
      createNewProcess();
      break;
    }
    case 3:
      showProcessMemoryState();
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
