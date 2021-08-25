#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define PRECISION UINT16_MAX

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

typedef struct Dec {
    uint64_t base;
    uint16_t length;
    uint64_t codeValue;
    char* decodedString;
    uint64_t decodedString_pos;
    uint64_t file_pos;
} Dec; //decoder struct

Alphabet* alphabet;
File* file;
Dec* dec;

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
            alphabet->values[position] = (strtod(line_buffer_aux, &ptr_aux)) * PRECISION;

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
    printf("\n%s | %lu\n", file->text, file->size);
    
}

void fileWriter(char* path)
{
    FILE *fp;

    if((fp = fopen(path, "w")) == NULL)
    {
        return;
    }

    for(int i = 0 ; i < dec->decodedString_pos ; i++)
    {
        fputc(dec->decodedString[i], fp);
    }

    //fputs(enc->bitStream, fp);

    fclose(fp);
}

// =================================== DEC ===================================

void printAux(char* name)
{
    printf("\n");
    printf("[%s] -> [base]: %f | [length]: %f | [codeValue]: %f | [decodedString]: ", name, (float) dec->base / PRECISION, (float) dec->length / PRECISION, (float) dec->codeValue / PRECISION);
   for(uint64_t i = 0 ; i < dec->decodedString_pos; i++)
    {
        printf("%c", dec->decodedString[i]);
    }
    printf(";\n");
}

long double fracBinaryConverter(uint64_t start, uint64_t end)
{
    long double binaryConverted = 0;
    uint64_t power = start + 1;

    for(uint64_t i = start ; i < end ; i++)
    {  
        if(file->text[i] == '1')
        {
            binaryConverted += 1 * (1/(pow(2, power)));
        }
        //printf("\n[%c|%lu|%Lf]\n", file->text[i], power, binaryConverted);
        power++;
    }

    return binaryConverted * PRECISION;
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

void intervalSelection()
{
    uint64_t symbolPos = alphabet->size;

    long double base = (long double) dec->base / PRECISION;
    long double length = (long double) dec->length / PRECISION;
    //long double cumulativeProb = (long double) cumulativeProbability(symbolPos) / PRECISION;
    long double fileValue = 0.0;

    long double x = base + (length * ((long double) cumulativeProbability(symbolPos) / PRECISION));
    long double y = base + length;

    //printf("\ncumProb: %.25Lf\n", cumulativeProb);
    //printf("\n%.25Lf | %.25Lf\n", x, dec->codeValue);

    while(x > (dec->codeValue / PRECISION))
    {
        //printf("\n%.25Lf | %.25Lf\n", x, dec->codeValue);
        symbolPos--;
        y = x;
        x = base + length * ((long double) cumulativeProbability(symbolPos) / PRECISION);
    }

    x = x * PRECISION;
    y = y * PRECISION;

    dec->base = (uint64_t) x;
    dec->length = (uint16_t) y - dec->base;

    dec->decodedString = realloc(dec->decodedString, sizeof(char) * (dec->decodedString_pos + 1));
    dec->decodedString[dec->decodedString_pos] = alphabet->letters[symbolPos];
    dec->decodedString_pos++;

}

void decoderRenorm()
{
    while(dec->length <= PRECISION/2)
    {
        if(dec->base >= PRECISION/2)
        {
            dec->base = 2*(dec->base - PRECISION/2);
            dec->codeValue = 2*((dec->codeValue) - PRECISION/2);
        }
        else
        {
            dec->base = dec->base * 2;
            dec->codeValue = dec->codeValue * 2;
        }
        dec->file_pos++;

        if(file->text[dec->file_pos] == '1')
        {
            dec->codeValue = dec->codeValue + ((pow(2,((int) dec->file_pos * -1)) * 1) * PRECISION);
            //printf("%Lf", dec->codeValue);
        }
        
        dec->length = 2*dec->length;
    }
}

void decode()
{
    dec = (Dec*) malloc(sizeof(Dec));
    dec->base = 0;
    dec->length = PRECISION;
    dec->decodedString = (char*) malloc(sizeof(char));
    dec->decodedString_pos = 0;
    dec->file_pos = 0;

    dec->codeValue = fracBinaryConverter(dec->file_pos, dec->file_pos + 16);
    dec->file_pos += 16;

    printf("filePos: %lu | fileSize: %lu\n", dec->file_pos, file->size);
    printAux("aaa");

    do //num of bits in bitstream
    {
        //v = 16 pos of bitstream?
        //bitstream = 0.bitstream
        //need to convert v to decimal where as v = 0.16pos of bitsream
        //on alg 7: 
        //s = last symbol of alphabet
        //x = intermediate base | y = intermediate length
        //while intermediate base > v(16 bits read from bitsream converted to decimal||integer(both base and v have to be on the same representation method. watch out for v conversion to integer where as v is the lower part of a fractional binary number (0.v)))



        // if(dec->file_pos > file->size)
        // {
        //     dec->codeValue = fracBinaryConverter(dec->file_pos, (dec->file_pos + (file->size - dec->file_pos)));
        //     dec->file_pos += file->size - dec->file_pos;
        // }
        // else
        // {
        //     dec->codeValue = fracBinaryConverter(dec->file_pos, dec->file_pos + 16);
        //     dec->file_pos += 16;
        // }

        // if (pos+16 > file->size)
        // {
        //     printf("%lu|%lu %.510Lf\n",pos, file->size - pos, fracBinaryConverter(pos, (pos + (file->size - pos))));
        //     pos += file->size - pos;
        // }
        // else
        // {
        //     printf("%lu|%lu %.510Lf\n",pos, pos+16, fracBinaryConverter(pos, pos + 16));
        //     pos += 16;
        // }

        printAux("pre-intervalSelection");

        intervalSelection();

        printAux("post-intervalSelection");

        // printf("\n");
        // for(uint64_t i = 0 ; i < dec->decodedString_pos; i++)
        // {
        //     printf("%c", dec->decodedString[i]);
        // }
        // printf("\n");

        // printf("\n%.25Lf\n", dec->codeValue);



        if(dec->base >= PRECISION)
        {
            dec->base -= PRECISION;
            dec->codeValue -= PRECISION;
        }

        if(dec->length <= PRECISION/2)
        {
            decoderRenorm();
        }
        
    }while (dec->file_pos < file->size);
}