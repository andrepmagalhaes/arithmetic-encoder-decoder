#include "Header.h"
//file input | alphabet | file output
int main(int argc, char **argv)
{

    fileReader(argv[1]);
    alphabetReader(argv[2]);

    encoder();

     printf("\n\n");

    for(uint64_t i = 0 ; i < enc->bitStream_pos; i++)
    {
        printf("%c", enc->bitStream[i]);
    }

    printf(" | %lu", enc->bitStream_pos);

    printf("\n");

    fileWriter(argv[3]);

    return 0;
}