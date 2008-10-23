/*
 * adler32.c (c) 2006 Bjorn Lindeijer
 * License: GPL, v2 or later
 *
 * Calculates Adler-32 checksums for all files passed as argument.
 *
 *  Usage: adler32 [file]...
 */

#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

/**
 * Calculates the Adler-32 checksum for the given file.
 */
unsigned long fadler32(FILE *file)
{
    // Obtain file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Calculate Adler-32 checksum
    char *buffer = (char*) malloc(fileSize);
    fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, (Bytef*) buffer, fileSize);
    free(buffer);

    return adler;
}

/**
 * Prints out usage and exists.
 */
void print_usage()
{
    printf("Usage: adler32 [file]...\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int i;                            /**< Loops through arguments. */

    if (argc == 1)
    {
        print_usage();
    }

    for (i = 1; i < argc; ++i)
    {
        FILE *file = fopen(argv[i], "r");

        if (!file)
        {
            printf("Error while opening '%s' for reading!\n", argv[i]);
            exit(1);
        }

        unsigned long adler = fadler32(file);
        printf("%s %lx\n", argv[i], adler);
        fclose(file);
    }

    return 0;
}
