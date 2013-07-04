/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS bin2c
 * FILE:            tools/bin2c/bin2c.c
 * PURPOSE:         Converts a binary file into a byte array
 * PROGRAMMER:      Herm�s B�lusca - Ma�to
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE* inFile;
    FILE* outCFile;
    FILE* outHFile;
    unsigned char ch;
    unsigned char cnt;

    /*
     * Validate the arguments.
     */
    if (argc < 5)
    {
        printf("Usage: bin2c infile.bin outfile.c outfile.h array_name [array_attribute [header_for_attribute]]\n");
        return -1;
    }

    /*
     * Open the input and the output files.
     */
    inFile = fopen(argv[1], "rb");
    if (!inFile)
    {
        printf("ERROR: Couldn't open data file '%s'.\n", argv[1]);
        return -1;
    }
    outCFile = fopen(argv[2], "w");
    if (!outCFile)
    {
        fclose(inFile);
        printf("ERROR: Couldn't create output source file '%s'.\n", argv[2]);
        return -1;
    }
    outHFile = fopen(argv[3], "w");
    if (!outHFile)
    {
        fclose(outCFile);
        fclose(inFile);
        printf("ERROR: Couldn't create output header file '%s'.\n", argv[3]);
        return -1;
    }

    /*
     * Generate the header file and close it.
     */
    fprintf(outHFile, "/* This file is autogenerated, do not edit. */\n\n");
    fprintf(outHFile, "#ifndef CHAR\n"
                      "#define CHAR char\n"
                      "#endif\n\n");
    fprintf(outHFile, "extern CHAR %s[];\n", argv[4]);
    fclose(outHFile);

    /*
     * Generate the source file and close it.
     */
    fprintf(outCFile, "/* This file is autogenerated, do not edit. */\n\n");
    if (argc >= 7)
    {
        /* There is a header to be included for defining the array attribute. */
        fprintf(outCFile, "#include \"%s\"\n", argv[6]);
    }
    fprintf(outCFile, "#include \"%s\"\n\n", argv[3]);

    /* Generate the array. */
    if (argc >= 6)
    {
        /* There is an array attribute. */
        fprintf(outCFile, "%s ", argv[5]);
    }
    fprintf(outCFile, "CHAR %s[] =\n{", argv[4]);

    cnt = 0;
    ch  = fgetc(inFile);
    while (!feof(inFile))
    {
        if ((cnt % 16) == 0)
        {
            fprintf(outCFile, "\n   ");
            cnt = 0;
        }
        fprintf(outCFile, " 0x%02x,", (unsigned int)ch);
        ++cnt;
        ch = fgetc(inFile);
    }
    /* Put a final NULL terminator. */
    fprintf(outCFile, "\n    0x00");
    fprintf(outCFile, "\n};\n");
    fclose(outCFile);

    /* Close the input file. */
    fclose(inFile);

    return 0;
}

/* EOF */
