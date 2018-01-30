#include "sort_file.h"
#include "bf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_RECORD_COUNT ((BF_BLOCK_SIZE - sizeof(int)) / sizeof(Record))

SR_ErrorCode SR_Init()
{
  return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName)
{
  BF_Block *BlockMetaData; // Block gia ton anagnwristiko arithmo 210
  BF_Block *dataBlock;     // Arxiko Block, posa records einai grammena sto arxeio
  int fileDesc, error;
  char *BlockMetaDataCont;         // metavlhth gia to periexomeno tou BlockMetaData
  char *dataBlockCont;             // metavlhth gia to periexomeno tou dataBlock
  error = BF_CreateFile(fileName); // Dimiourgoume to arxeio
  if (error)
  {
    printf("\n error on line 24 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  error = BF_OpenFile(fileName, &fileDesc); // Anoigoume to arxeio pou dimiourghsame
  if (error)
  {
    printf("\n error on line 30 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  BF_Block_Init(&BlockMetaData); // Arxikopoioume
  error = BF_AllocateBlock(fileDesc, BlockMetaData);
  if (error)
  {
    printf("\n error on line 37 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  BlockMetaDataCont = BF_Block_GetData(BlockMetaData); // Getting block Content
  *(int *)BlockMetaDataCont = 210;                     // arithmos 210 gia anagnwristiko heap file
  BF_Block_SetDirty(BlockMetaData);                    // kanoume dirty to block wste meta na graftei sto disko
  error = BF_UnpinBlock(BlockMetaData);                // kanoume unpin wste na eleutherwsoume ton buffer
  if (error)
  {
    printf("\n error on line 46 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  BF_Block_Init(&dataBlock); // Arxikopoioume to dataBlock
  error = BF_AllocateBlock(fileDesc, dataBlock);
  if (error)
  {
    printf("\n error on line 53 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  dataBlockCont = BF_Block_GetData(dataBlock);
  *(int *)dataBlockCont = 0; // Arxikopoiw ton arithmo twn egrafwn se 0
  BF_Block_SetDirty(dataBlock);
  error = BF_UnpinBlock(dataBlock);
  if (error)
  {
    printf("\n error on line 62 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }
  error = BF_CloseFile(fileDesc); //kleinoume to arxeio
  if (error)
  {
    printf("\n error on line 68 \n");
    BF_PrintError(error);
    return SR_ERROR;
  }

  return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc)
{
  BF_Block *BlockMetaData;
  BF_Block_Init(&BlockMetaData); // Arxikopoihsh tou block
  char *BlockMetaDataCont;
  int error;
  error = BF_OpenFile(fileName, fileDesc); // anoigoume to arxeio me onoma filename
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  error = BF_GetBlock(*fileDesc, 0, BlockMetaData);
  if (error)
  {
    printf("\n %d \n", *fileDesc);
    BF_PrintError(error);
    return SR_ERROR;
  }
  BlockMetaDataCont = BF_Block_GetData(BlockMetaData); //kratame to periexomeno tou BlockMetaData sth metavlhth BlockMetaDataCont
  if (*(int *)BlockMetaDataCont != 210)                // An den einai heap file
  {
    BF_PrintError(BF_INVALID_FILE_ERROR);
    return SR_ERROR;
  }
  error = BF_UnpinBlock(BlockMetaData);
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  return SR_OK;
}

SR_ErrorCode SR_CloseFile(int fileDesc)
{
  int error;
  error = BF_CloseFile(fileDesc); //dinoume sth metavlhth error thn timh pou epistrefei h sunarthsh
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  return SR_OK;
}

SR_ErrorCode SR_InsertEntry(int fileDesc, Record record)
{
  int NumOfBlocks;        //metavlhth sthn opoia kataxwrw ton arithmo twn blocks
  BF_Block *currentBlock; // to current block
  char *currentBlockData; // metavlhth gia ta dedomena tou current block
  int *recordCount;
  int error;

  BF_Block_Init(&currentBlock);
  error = BF_GetBlockCounter(fileDesc, &NumOfBlocks);
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  error = BF_GetBlock(fileDesc, NumOfBlocks - 1, currentBlock); // kataxwrhse to block sto currentBlock
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  currentBlockData = BF_Block_GetData(currentBlock); //kataxwrhsh twn dedomenwn tou currentBlock
  recordCount = (int *)currentBlockData;
  if (*(recordCount) == MAX_RECORD_COUNT) // an to block einai gemato tote ginontai ta parakatw
  {
    BF_Block *Block;                     //ftiaxnw kainourio block
    BF_Block_Init(&Block);               // kai to arxikopoiw
    error = BF_UnpinBlock(currentBlock); // kanw unpin to palio block
    if (error)
    {
      BF_PrintError(error);
      return SR_ERROR;
    }
    currentBlock = Block;
    error = BF_AllocateBlock(fileDesc, currentBlock);
    if (error)
    {
      BF_PrintError(error);
      return SR_ERROR;
    }
    currentBlockData = BF_Block_GetData(currentBlock);
    *(int *)currentBlockData = 0; // arxika exw 0 records
    recordCount = (int *)currentBlockData;
  }
  memcpy(currentBlockData + sizeof(int) + (*recordCount) * sizeof(Record), &record, sizeof(Record)); // grafoume to neo record
  (*recordCount)++;                                                                                  // auksanoume kata ena th metavlhth
  BF_Block_SetDirty(currentBlock);
  error = BF_UnpinBlock(currentBlock);
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }
  return SR_OK;
}
SR_ErrorCode SR_PrintAllEntries(int fileDesc)
{
  int errorCode, blockCount, recordCount, i, j;
  Record currentRecord;
  BF_Block *currentBlock, *lastBlock;
  BF_Block_Init(&currentBlock);
  BF_Block_Init(&lastBlock); //Initializing Blocks
  char *blockData;

  errorCode = BF_GetBlockCounter(fileDesc, &blockCount); // Getting Block count
  if (errorCode)
  {
    BF_PrintError(errorCode); // If getting fails
    return SR_ERROR;
  }
  for (i = 1; i < blockCount; i++) // For each block
  {
    errorCode = BF_GetBlock(fileDesc, i, currentBlock);
    if (errorCode)
    {
      BF_PrintError(errorCode); // If closing fails
      return SR_ERROR;
    }
    blockData = BF_Block_GetData(currentBlock);
    recordCount = *(int *)blockData;
    for (j = 0; j < recordCount; j++) //For each Entry (Record)
    {
      memcpy(&currentRecord, blockData + sizeof(int) + j * sizeof(Record), sizeof(Record));                      // Copy data to temporary record
      printf("\n %d %s %s %s", currentRecord.id, currentRecord.name, currentRecord.surname, currentRecord.city); //Print it
    }
    errorCode = BF_UnpinBlock(currentBlock); // Trying to unpin
    if (errorCode)
    {
      BF_PrintError(errorCode); // if unpin fails
      return SR_ERROR;
    }
  }
  return SR_OK;
}

SR_ErrorCode SR_SortedFile(
    const char *input_filename,
    const char *output_filename,
    int fieldNo,
    int bufferSize)
{
  // Initial error check
  if (bufferSize < 3 || bufferSize > BF_BUFFER_SIZE)
  {
    BF_PrintError(BF_FULL_MEMORY_ERROR);
    return SR_ERROR;
  }

  // Declarations
  int i, j, k, inputFileDesc, outputFileDesc, tempFileDesc1, tempFileDesc2, errorCode, fileBlockCount;

  //============================================================================================================================
  // Initializing open files files

  // input - output
  errorCode = SR_OpenFile(input_filename, &inputFileDesc);
  if (errorCode)
  {
    printf("\n error on line 205 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }
  errorCode = SR_CreateFile(output_filename);
  if (errorCode)
  {
    printf("\n error on line 214 \n");
    BF_PrintError(BF_FILE_ALREADY_EXISTS);
    return SR_ERROR;
  }
  errorCode = SR_OpenFile(output_filename, &outputFileDesc);
  if (errorCode)
  {
    printf("\n error on line 212 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }

  // temp 1-2
  char *tempFileName1 = "temp1.db";
  errorCode = SR_CreateFile(tempFileName1);
  if (errorCode)
  {
    printf("\n error on line 220 \n");
    BF_PrintError(BF_FILE_ALREADY_EXISTS);
    return SR_ERROR;
  }
  errorCode = SR_OpenFile(tempFileName1, &tempFileDesc1);
  if (errorCode)
  {
    printf("\n error on line 227 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }
  char *tempFileName2 = "temp2.db";
  errorCode = SR_CreateFile(tempFileName2);
  if (errorCode)
  {
    printf("\n error on line 235 \n");
    BF_PrintError(BF_FILE_ALREADY_EXISTS);
    return SR_ERROR;
  }
  errorCode = SR_OpenFile(tempFileName2, &tempFileDesc2);
  if (errorCode)
  {
    printf("\n error on line 242 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }

  //============================================================================================================================
  // Geting the count of inputFile
  errorCode = BF_GetBlockCounter(inputFileDesc, &fileBlockCount);
  if (errorCode)
  {
    printf("\n error on line 237 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }

  int pieces = ((fileBlockCount - 1) / bufferSize) + 1;
  int containBlocks = bufferSize;

  BF_Block **buffer = (BF_Block **)malloc(bufferSize * sizeof(BF_Block *));

  // //============================================================================================================================
  // Filling the buffer and quicksoring it
  int counter = 1, recordCount;
  char *blockData;
  for (i = 0; i < pieces; i++)
  {
    if (i == pieces - 1)
    {
      //containBlocks = (fileBlockCount - 1) - (pieces * bufferSize);
      containBlocks = fileBlockCount - counter - 1;
      if (containBlocks < 0)
      {
        continue;
      }
    }
    if (i == pieces - 2)
    {
      //containBlocks = (fileBlockCount - 1) - (pieces * bufferSize);
      containBlocks = fileBlockCount - counter;
    }
    errorCode = fillBuffer(buffer, &counter, containBlocks, inputFileDesc);
    if (errorCode)
    {
      printf("\n error on line 335 \n");
      BF_PrintError(errorCode);
      return SR_ERROR;
    }
    if (i == pieces - 1)
    {
      blockData = BF_Block_GetData(buffer[containBlocks - 1]);
      recordCount = *(int *)blockData;
      quickSort(buffer, 0, (recordCount * containBlocks) - 1, fieldNo);
    }
    else
    {
      quickSort(buffer, 0, (MAX_RECORD_COUNT * containBlocks) - 1, fieldNo);
    }

    // Saving the sorted file to temp file
    writeToFile(buffer, containBlocks, tempFileDesc1, -1);
    emptyBuffer(buffer, containBlocks);
  }
  //SR_CloseFile(tempFileDesc1);
  //SR_CloseFile(tempFileDesc2);
  //deleteFile(tempFileName1);
  //deleteFile(tempFileName2);
  //SR_PrintAllEntries(tempFileDesc1);

  // //============================================================================================================================
  // //// "Merge Sort" section

  BF_Block *outputBlock;
  BF_Block_Init(&outputBlock);
  errorCode = BF_GetBlock(outputFileDesc, 1, outputBlock);
  if (errorCode)
  {
    printf("\n error on line 396 \n");
    BF_PrintError(errorCode);
    return SR_ERROR;
  }
  buffer[bufferSize - 1] = outputBlock;

  char *outputBlockData = BF_Block_GetData(buffer[bufferSize - 1]); // Getting block context
  *(int *)outputBlockData = 0;

  int availableBufferSize = bufferSize - 1;
  int maxBlocksToLoad = availableBufferSize / 2;
  if (maxBlocksToLoad == 0)
  {
    maxBlocksToLoad = 1;
  }
  int readFileDesc = tempFileDesc1;
  int writeFileDesc = tempFileDesc2;
  while (pieces != 0)
  {
    errorCode = BF_GetBlockCounter(readFileDesc, &fileBlockCount);
    if (errorCode)
    {
      BF_PrintError(errorCode);
      return SR_ERROR;
    }
    int blocksPerPiece = ((fileBlockCount - 1) / pieces) + 1;
    for (i = 0; i < pieces - 2; i = i + 2)
    {
      fillBufferByPiece(buffer, availableBufferSize, readFileDesc, maxBlocksToLoad, (i * blocksPerPiece), ((i + 1) * blocksPerPiece), 0, 0);
      int count1 = 0;
      int blockCount1 = 0;
      int count2 = 0;
      int blockCount2 = 0;
      int halfBufferRecords = (availableBufferSize / 2) * MAX_RECORD_COUNT;
      Record *record1;
      Record *record2;
      while ((count1 < halfBufferRecords || count2 < halfBufferRecords) && (blockCount1 < blocksPerPiece || blockCount2 < blocksPerPiece))
      {
        record1 = getRecordByIndex(buffer, count1);
        record2 = getRecordByIndex(buffer, halfBufferRecords + count2 + 1);
        if (count1 == halfBufferRecords && blockCount1 < blocksPerPiece)
        {
          blockCount1++;
          emptyBuffer(buffer, availableBufferSize);
          fillBufferByPiece(buffer, availableBufferSize, readFileDesc, maxBlocksToLoad, (i * blocksPerPiece), ((i + 1) * blocksPerPiece), blockCount1, 0);
          count1 = 0;
        }
        else if (count2 == halfBufferRecords && blockCount2 < blocksPerPiece)
        {
          blockCount2++;
          emptyBuffer(buffer, availableBufferSize);
          fillBufferByPiece(buffer, availableBufferSize, readFileDesc, maxBlocksToLoad, (i * blocksPerPiece), ((i + 1) * blocksPerPiece), 0, blockCount2);
          count2 = 0;
        }
        else if (count1 == halfBufferRecords)
        {
          writeToOutputBlock(record2, buffer, bufferSize, writeFileDesc);
          count2++;
        }
        else if (count2 == halfBufferRecords)
        {
          writeToOutputBlock(record1, buffer, bufferSize, writeFileDesc);
          count1++;
        }
        else
        {
          if (record1 == NULL)
          {
            printf(" ======================================== \n");
          }
          if (compere(record1, record2, fieldNo) == -1 || compere(record1, record2, fieldNo) == 0)
          {
            writeToOutputBlock(record1, buffer, bufferSize, writeFileDesc);
            count1++;
          }
          else
          {
            writeToOutputBlock(record2, buffer, bufferSize, writeFileDesc);
            count2++;
          }
        }
      }
      emptyBuffer(buffer, availableBufferSize);
    }
    if (writeFileDesc == tempFileDesc2)
    {
      writeFileDesc = tempFileDesc1;
      readFileDesc = tempFileDesc2;
    }
    else
    {
      writeFileDesc = tempFileDesc2;
      readFileDesc = tempFileDesc1;
    }

    pieces = (pieces / 2) + 1;
  }

  // //============================================================================================================================
  // // Ending
  free(buffer);
  BF_UnpinBlock(outputBlock);
  SR_CloseFile(tempFileDesc2);
  SR_CloseFile(outputFileDesc);
  SR_CloseFile(inputFileDesc);
  return SR_OK;
}

//------------------------------------------------------  Helpers Part -----------------------------------------------------------------------

int writeToOutputBlock(Record *record, BF_Block **buffer, int bufferSize, int fileDesc)
{
  char *blockData;
  blockData = BF_Block_GetData(buffer[bufferSize - 1]);
  int *recordCount = (int *)blockData;
  if (*recordCount == MAX_RECORD_COUNT)
  {
    writeToFile(buffer, bufferSize, fileDesc, bufferSize - 1);
  }
  memcpy(blockData + sizeof(int) + ((*recordCount) * sizeof(Record)), record, sizeof(Record));
  (*recordCount)++;
  return SR_OK;
}

int fillBuffer(BF_Block **buffer, int *counter, int bufferSize, int fileDesc)
{

  BF_Block *currentBlock;
  int i, errorCode, recordCount;
  char *blockData;
  for (i = 0; i < bufferSize; i++)
  {
    BF_Block_Init(&currentBlock);
    errorCode = BF_GetBlock(fileDesc, *counter, currentBlock);

    // blockData = BF_Block_GetData(currentBlock);
    // recordCount = *(int *)blockData;
    // printf("\n %d \n", recordCount);

    if (errorCode)
    {
      BF_PrintError(errorCode);
      return SR_ERROR;
    }
    buffer[i] = currentBlock;
    (*counter)++;
  }
  return SR_OK;
}

int fillBufferByPiece(BF_Block **buffer,
                      int availableBufferSize,
                      int fileDesc,
                      int maxBlocksToLoad,
                      int firstPieceStart,
                      int secondPieceStart,
                      int offset1,
                      int offset2)
{
  BF_Block *currentBlock;
  int i, j, errorCode, recordCount, useStart = firstPieceStart;
  char *blockData;
  int counter = 0;
  int offset = offset1;
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < maxBlocksToLoad; j++)
    {
      if (i != 0)
      {
        if (offset2 != 0)
        {
          offset = offset2;
        }
        useStart = secondPieceStart;
      }
      BF_Block_Init(&currentBlock);
      errorCode = BF_GetBlock(fileDesc, (useStart + j) + 1 + offset, currentBlock);
      if (errorCode)
      {
        printf("GOT HERE ASDHJKASHLDKJHASJKLDHLGAKJSDLGUJKASGDLKJHASGLUKHJDAGISJS\n");
        BF_PrintError(errorCode);
        return SR_ERROR;
      }
      buffer[counter] = currentBlock;
      counter++;
    }
  }
  return SR_OK;
}

int emptyBuffer(BF_Block **buffer, int reps)
{
  int errorCode, i;
  for (i = 0; i < reps; i++)
  {
    errorCode = BF_UnpinBlock(buffer[i]);
    if (errorCode)
    {
      printf("\n===============427======================\n");
      BF_PrintError(errorCode);
      return SR_ERROR;
    }
  }
  return SR_OK;
}

void writeToFile(BF_Block **buffer, int bufferSize, int fileDesc, int index) // If index is -1 then write the whole buffer
{
  int i, k;
  Record record;
  int recordCount;
  char *currentBlockData;

  if (index == -1)
  {
    for (i = 0; i < bufferSize; i++)
    {
      currentBlockData = BF_Block_GetData(buffer[i]);
      recordCount = *(int *)currentBlockData;
      for (k = 0; k < recordCount; k++)
      {
        memcpy(&record, currentBlockData + sizeof(int) + (sizeof(Record) * k), sizeof(Record));
        if (record.id == 0 && strlen(record.city) <= 1)
        {
          continue;
        }
        SR_InsertEntry(fileDesc, record);
      }
    }
  }
  else
  {
    currentBlockData = BF_Block_GetData(buffer[index]);
    recordCount = *(int *)currentBlockData;
    for (k = 0; k < recordCount; k++)
    {
      memcpy(&record, currentBlockData + sizeof(int) + (sizeof(Record) * k), sizeof(Record));
      if (record.id == 0 && strlen(record.city) <= 1)
      {
        continue;
      }
      SR_InsertEntry(fileDesc, record);
    }
    *(int *)currentBlockData = 0;
  }
}

SR_ErrorCode clearFile(char *filename, int *fileDesc)
{
  int error;
  error = BF_CloseFile(*fileDesc); //dinoume sth metavlhth error thn timh pou epistrefei h sunarthsh
  if (error)
  {
    BF_PrintError(error);
    return SR_ERROR;
  }

  char command[100];
  sprintf(command, "rm %s", filename); //isws xreiastei to relative path
  error = system(command);
  if (error)
  {
    BF_PrintError(BF_INVALID_FILE_ERROR);
    return SR_ERROR;
  }

  error = SR_CreateFile(filename);
  if (error)
  {
    return SR_ERROR;
  }
  error = SR_OpenFile(filename, fileDesc);
  if (error)
  {
    return SR_ERROR;
  }
  return SR_OK;
}

SR_ErrorCode deleteFile(char *filename)
{
  int error;
  char command[100];
  sprintf(command, "rm %s", filename); //isws xreiastei to relative path
  error = system(command);
  if (error)
  {
    BF_PrintError(BF_INVALID_FILE_ERROR);
    return SR_ERROR;
  }
  return SR_OK;
}

int compere(Record *record1, Record *record2, int fieldNo)
{
  int i;
  switch (fieldNo)
  {
    {
    case 0:
      if (record1->id < record2->id)
        return -1;
      else if (record1->id == record2->id)
        return 0;
      else if (record1->id > record2->id)
        return 1;

      break;
    case 1:
      i = strcmp(record1->name, record2->name);
      if (i < 0)
        return -1;
      else if (i == 0)
        return 0;
      else if (i > 0)
        return 1;

      break;
    case 2:
      i = strcmp(record1->surname, record2->surname);
      if (i < 0)
        return -1;
      else if (i == 0)
        return 0;
      else if (i > 0)
        return 1;

      break;
    case 3:
      i = strcmp(record1->city, record2->city);
      if (i > 0)
        return -1;
      else if (i == 0)
        return 0;
      else if (i > 0)
        return 1;
      break;

    default:

      break;
    }
  }
}

// A utility function to swap two elements
void swap(Record *a, Record *b)
{
  Record temp;
  memcpy(&temp, a, sizeof(Record));
  memcpy(a, b, sizeof(Record));
  memcpy(b, &temp, sizeof(Record));

  // Record t = *a;
  // *a = *b;
  // *b = t;
}

/* This function takes last element as pivot, places
	 the pivot element at its correct position in sorted
	 array, and places all smaller (smaller than pivot)
	 to left of pivot and all greater elements to right
	 of pivot */
int partition(BF_Block **buffer, int l, int r, int fieldNo)
{
  int p, i, j, t;
  Record *pivot = getRecordByIndex(buffer, l);
  i = l;
  j = r + 1;
  Record *record1;
  Record *record2;
  Record *record3;
  while (1)
  {
    do
    {
      ++i;
      record1 = getRecordByIndex(buffer, i);
    } while ((compere(record1, pivot, fieldNo) == -1 || compere(record1, pivot, fieldNo) == 0) && i <= r);
    do
    {
      --j;
      record2 = getRecordByIndex(buffer, j);
    } while (compere(record2, pivot, fieldNo) == 1);
    if (i >= j)
      break;
    swap(record1, record2);
  }
  record3 = getRecordByIndex(buffer, l);
  swap(record3, record2);
  return j;
}

void quickSort(BF_Block **buffer, int l, int r, int fieldNo)
{

  int j;

  if (l < r)
  {
    j = partition(buffer, l, r, fieldNo);
    quickSort(buffer, l, j - 1, fieldNo);
    quickSort(buffer, j + 1, r, fieldNo);
  }
}

Record *getRecordByIndex(BF_Block **buffer, int index) // Tested , works fine
{
  Record *record;          //h metavlhth pou epistrefei h sunarthsh
  int BlockNumberOfRecord; //o arithmos tou block pou vrisketai h egrafh
  int EntryNumberOfRecord; //h thesh ths egrafhs
  BF_Block *Block;
  char *blockData;

  if (index == 0)
  {
    BlockNumberOfRecord = 0;
    EntryNumberOfRecord = 0;
  }
  else if (index == 1)
  {
    BlockNumberOfRecord = 0;
    EntryNumberOfRecord = 1;
  }
  else
  {
    BlockNumberOfRecord = ((index - 1) / MAX_RECORD_COUNT);
    EntryNumberOfRecord = ((index - 1) % MAX_RECORD_COUNT);
  }
  if (BF_BLOCK_SIZE < BlockNumberOfRecord) //An o arithmos twn block tou pinaka einai mikroteros apo ton arithmo tou block pou einai h egrafh pou psaxnoume
  {
    return 0;
  }
  Block = buffer[BlockNumberOfRecord]; //kataxwroume sth metavlhth block, to block me noumero BlockNumberOfRecord

  blockData = BF_Block_GetData(Block); //deikths sta dedomena tou Block
  blockData = blockData + sizeof(int) + (EntryNumberOfRecord * sizeof(Record));
  record = (Record *)blockData;
  return record;
}