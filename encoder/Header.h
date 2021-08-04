#include <stdio.h>
#include <stdlib.h>
#include "Functions.c"

// typedef struct Alphabet {
//     char* letters;
//     double* values;
// } Alphabet;

void fileReader(char* path);
void alphabetReader(char* path);
void fileWriter(char* path);
void renormalization();
uint16_t cumulativeProbability(uint64_t symbolPos);
void encoder();