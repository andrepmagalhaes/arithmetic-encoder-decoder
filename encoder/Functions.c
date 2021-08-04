#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

//https://reusablesec.blogspot.com/2009/05/character-frequency-analysis-info.html

typedef struct Alphabet {
    char* letters;
    uint16_t* values;
    uint64_t size;
    uint16_t initial_length;
} Alphabet;

typedef struct File {
    char* text;
    uint64_t size;
} File;

typedef struct Enc {
    uint64_t base;
    uint16_t length;
    char* bitStream;
    uint64_t bitStream_pos;
    uint64_t file_pos;
    uint64_t t;
} Enc; //encoder struct

Alphabet* alphabet;
File* file;
Enc* enc;

void fileReader(char* path)
{
    char* file_data;
    uint64_t file_size;
    FILE* fp;

    if((fp = fopen(path, "r")) == NULL)
    {
        return;
    }

    file = (File*) malloc(sizeof(File));

    fseek(fp, 0, SEEK_END);
    file->size = ftell(fp);
    rewind(fp);
    file->text = (char*) malloc((sizeof(char)) * file->size);
    fread(file->text, sizeof(char), file->size, fp);

    fclose(fp);
    //file = file_data;
}

void alphabetReader(char *path)
{
    FILE* fp;

    if((fp = fopen(path, "r")) == NULL)
    {
        return;
    }

    char* line_buffer = (char*) malloc(sizeof(char));
    uint64_t char_buffer;
    uint64_t counter = 0;
    uint64_t position = 0;

    alphabet = (Alphabet*) malloc(sizeof(Alphabet));
    alphabet->letters = (char*) malloc((sizeof(char)));
    alphabet->values = (uint16_t*) malloc((sizeof(uint16_t)));
    alphabet->initial_length = 0;
    
    while ((char_buffer = fgetc(fp)) != EOF)
    {
        if(char_buffer == '\n')
        {
            alphabet->letters = realloc(alphabet->letters, (sizeof(char) * (position + 1)));
            alphabet->values = realloc(alphabet->values, (sizeof(uint16_t) * (position + 1)));
            alphabet->size = position + 1;
            char* line_buffer_aux = (char*) malloc(sizeof(char));
            char* ptr_aux;
            uint64_t i = 0;
            uint64_t j = 0;

            for (i = 0; i < counter; i++)
            {
                if(((line_buffer[i] == ',') && (line_buffer[i+1] == ',')))
                {
                    i++;
                    break;
                }
                if((line_buffer[i] == ','))
                {
                    break;
                }
            }

            alphabet->letters = realloc(alphabet->letters, sizeof(char) * (position + 1));
            alphabet->letters[position] = line_buffer[j];

            j = 0;

            for (i++ ; i < counter ; i++) 
            {
                //printf("%c ", line_buffer[i]);
                line_buffer_aux = realloc(line_buffer_aux, sizeof(char) * (j + 1));
                line_buffer_aux[j] = line_buffer[i];
                j++;
            }
            alphabet->values[position] = (strtod(line_buffer_aux, &ptr_aux)) * UINT16_MAX;

            counter = 0;
            position++;
            free(line_buffer_aux);
            free(line_buffer);
            line_buffer = malloc(sizeof(char));

        }
        else
        {

            line_buffer = realloc(line_buffer, sizeof(char) * (counter + 1));
            line_buffer[counter] = char_buffer;
            counter++;

        }
    }
    free(line_buffer);
    fclose(fp);

    for(uint64_t i = 0; i < alphabet->size ; i++)
    {
        printf("%c | %i\n", alphabet->letters[i], alphabet->values[i]);
        alphabet->initial_length += alphabet->values[i];
    }
    // printf("sum: %i\n", alphabet->initial_length);
    printf("\n%s | %lu", file->text, file->size);
    
}

void fileWriter(char* path)
{
    FILE *fp;

    if((fp = fopen(path, "w")) == NULL)
    {
        return;
    }

    for(int i = 0 ; i < enc->bitStream_pos ; i++)
    {
        fputc(enc->bitStream[i], fp);
    }

    //fputs(enc->bitStream, fp);

    fclose(fp);
}

// =================================== ENC ===================================

uint64_t findSymbol(char symbol)
{
    uint64_t diff;
    uint64_t a;
    uint64_t b;
    for(uint64_t i = 0 ; i < alphabet->size ; i++)
    {
        //printf("%lu ", i);
        //printf("\n%c %i | %c %i \n", symbol, symbol, alphabet->letters[i], alphabet->letters[i]);
        a = (uint64_t) symbol;
        b = (uint64_t) alphabet->letters[i];
        if((a-b) == 0)
        {
            return i;
        }
    }
}

uint16_t cumulativeProbability(uint64_t symbolPos)
{
    uint16_t cumulativeProbability = 0;

    for(uint64_t i = 0; i < symbolPos ; i++)
    {
        cumulativeProbability = cumulativeProbability + alphabet->values[i];
    }
    return cumulativeProbability;
}

void renormalization()
{

    while(enc->length <= 32767) //UINT16_MAX / 2
    {
        //enc->bitStream_pos++;
        //enc->length = (enc->length << 1);
        enc->length *= 2;
        if(enc->base >= 32767) 
        {
            enc->bitStream = realloc(enc->bitStream, sizeof(char) * (enc->bitStream_pos + 1));
            enc->bitStream[enc->bitStream_pos] = '1';
            enc->bitStream_pos++;
            enc->base = (enc->base - 32767) * 2;
            //enc->base = ((enc->base - 32767) << 1); // TODO: test with 32767 (0111111111111111) and 32768 (1000000000000000)
        }
        else
        {
            enc->bitStream = realloc(enc->bitStream, sizeof(char) * (enc->bitStream_pos + 1));
            enc->bitStream[enc->bitStream_pos] = '0';
            enc->bitStream_pos++;
            //enc->base = (enc->base << 1);
            enc->base *= 2;
        }
    }
}

void propagate_carry()
{
    uint64_t n = enc->bitStream_pos;
    while(enc->bitStream[n] == '1')
    {
        enc->bitStream[n] = '0';
        n--;
    }
    enc->bitStream[n] = '1';
}

void interval_update() //TODO fix interval update
{
    //printf("[base]: %lu | [length]: %u -> ", enc->base, enc->length);
    uint64_t symbolPos = findSymbol(file->text[enc->file_pos]);
    //printf(" | %lu ,", symbolPos);
    long double base = (long double) enc->base / UINT16_MAX;
    long double cumulativeProb = (long double) cumulativeProbability(symbolPos) / UINT16_MAX;
    long double length = (long double) enc->length / UINT16_MAX;
    long double symbolProb = (long double) alphabet->values[symbolPos] / UINT16_MAX;
    //printf("%u", cumulativeProbability(symbolPos));
    
    //printf("\n\t[base]: %Lf | [cumulativeProb]: %Lf | [length]: %Lf | [symbolProb]: %Lf -> ", base, cumulativeProb, length, symbolProb);

    base = base + cumulativeProb * length;
    length = length * symbolProb;
    base = base * UINT16_MAX;
    length = length * UINT16_MAX;

    //printf("[base]: %Lf | [length]: %Lf \n", base, length);

    enc->base = (uint64_t) base;
    enc->length = (uint16_t) length;
    
    // enc->base = enc->base + cumulativeProbability(symbolPos) * enc->length;
    // enc->length = enc->length * alphabet->values[symbolPos];

}

void bitStream_selector()
{
    //enc->t++;
    if(enc->base <= 32767) //check half
    {
        enc->bitStream = realloc(enc->bitStream, sizeof(char) * (enc->bitStream_pos + 1));
        enc->bitStream[enc->bitStream_pos] = '1';
        enc->bitStream_pos++;
    }
    else
    {
        enc->bitStream = realloc(enc->bitStream, sizeof(char) * (enc->bitStream_pos + 1));
        enc->bitStream[enc->bitStream_pos] = '0';
        enc->bitStream_pos++;
    }
}

void code_value_selection() //TODO: finish implementing code value selection
{
    enc->bitStream_pos++;

    if(enc->base <= 32767)
    {
        enc->bitStream[enc->bitStream_pos] = '1';
    }
    else
    {
        enc->bitStream[enc->bitStream_pos] = '0';
        enc->bitStream_pos--;
        propagate_carry();
    }
}

//EOF = *
void printAux(char* name)
{
    // printf("\n");
    // printf("[%s] -> [base]: %lu | [length]: %u | [bitstream]: ", name, enc->base, enc->length);
    // for(uint64_t i = 0 ; i < enc->bitStream_pos; i++)
    // {
    //     printf("%c", enc->bitStream[i]);
    // }
    // printf(";\n");
}

void encoder()
{
    enc = (Enc*) malloc(sizeof(Enc));
    enc->base = 0;
    enc->length = UINT16_MAX;
    enc->t = 0;
    enc->file_pos = 0;
    enc->bitStream_pos = 0;
    enc->bitStream = (char*) malloc(sizeof(char));

    uint64_t charCmp = (uint64_t) file->text[enc->file_pos];

    while( charCmp != 42 )
    {
        //printf("%c\n", charCmp);
        printAux("b4 interval_update");
        interval_update();
        printAux("after interval_update");
        if(enc->base >= UINT16_MAX)
        {
            enc->base = enc->base - UINT16_MAX;
            propagate_carry();
        }
        printAux("after propagate carry");
        if(enc->length <= 32767)
        {
            renormalization();
        }
        printAux("after renorm");
        bitStream_selector();
        printAux("after bitstream select");
        enc->file_pos++;
        charCmp = (uint64_t) file->text[enc->file_pos];
    }

    // printf("\n");
    // for(enc->file_pos; enc->file_pos < file->size ; enc->file_pos++)//TODO: change for loop to while(file->text[x] != '*')
    // {
    //     printf("\n4\n");
    //     charCmp = (uint64_t) file->text[enc->file_pos];
    //     printf("%c[%i]", file->text[enc->file_pos], file->text[enc->file_pos]);
    //     if(charCmp == 42)
    //     {
    //         break;
    //     }
    //     interval_update();
    //     if(enc->base >= UINT16_MAX)
    //     {
    //         enc->base = enc->base - UINT16_MAX;
    //         propagate_carry();
    //     }
    //     if(enc->length <= 32767)
    //     {
    //         renormalization();
    //     }
    //     bitStream_selector();
    // }

    // for(int i = 0 ; i < 115 ; i++)
    // {
    //     printf("%c", file->text[enc->file_pos]);
    //     if(file->text[enc->file_pos] != '')
    //     {
    //         break;
    //     }
    //     interval_update();
    //     if(enc->base >= UINT16_MAX)
    //     {
    //         enc->base = enc->base - UINT16_MAX;
    //         propagate_carry();
    //     }
    //     if(enc->length <= 32767)
    //     {
    //         renormalization();
    //     }
    //     bitStream_selector();
    //     aa++;
    // }

    code_value_selection();

}


