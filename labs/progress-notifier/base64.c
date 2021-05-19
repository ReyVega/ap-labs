// Reference used
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include "logger.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE 64
#define EQUALS 65
#define INVALID 66

static const unsigned char d[] = {
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 66, 66, 66, 65, 66, 66, 66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66, 66, 66, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66};

long numberOfCharsTotal = 0L;
long numberOfChars = 0L;

int base64encode(const void *data_buf, size_t dataLength, char *result, size_t resultSize)
{
    const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const uint8_t *data = (const uint8_t *)data_buf;
    size_t resultIndex = 0;
    size_t x;
    uint32_t n = 0;
    int padCount = dataLength % 3;
    uint8_t n0, n1, n2, n3;

    /* increment over the length of the string, three characters at a time */
    for (x = 0; x < dataLength; x += 3)
    {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

        if ((x + 1) < dataLength)
            n += ((uint32_t)data[x + 1]) << 8; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

        if ((x + 2) < dataLength)
            n += data[x + 2];

        /* this 24-bit number gets separated into four 6-bit numbers */
        n0 = (uint8_t)(n >> 18) & 63;
        n1 = (uint8_t)(n >> 12) & 63;
        n2 = (uint8_t)(n >> 6) & 63;
        n3 = (uint8_t)n & 63;

        /*
       * if we have one byte available, then its encoding is spread
       * out over two characters
       */
        if (resultIndex >= resultSize)
            return 1; /* indicate failure: buffer too small */
        result[resultIndex++] = base64chars[n0];
        if (resultIndex >= resultSize)
            return 1; /* indicate failure: buffer too small */
        result[resultIndex++] = base64chars[n1];

        /*
       * if we have only two bytes available, then their encoding is
       * spread out over three chars
       */
        if ((x + 1) < dataLength)
        {
            if (resultIndex >= resultSize)
                return 1; /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n2];
        }

        /*
       * if we have all three bytes available, then their encoding is spread
       * out over four characters
       */
        if ((x + 2) < dataLength)
        {
            if (resultIndex >= resultSize)
                return 1; /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n3];
        }
        numberOfChars += 3;
    }

    /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
    if (padCount > 0)
    {
        for (; padCount < 3; padCount++)
        {
            if (resultIndex >= resultSize)
                return 1; /* indicate failure: buffer too small */
            result[resultIndex++] = '=';
        }
    }
    if (resultIndex >= resultSize)
        return 1; /* indicate failure: buffer too small */
    result[resultIndex] = 0;
    return 0; /* indicate success */
}

int base64decode(char *in, size_t inLen, unsigned char *out, size_t *outLen)
{
    char *end = in + inLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;

    while (in < end)
    {
        unsigned char c = d[*in++];

        switch (c)
        {
        case WHITESPACE:
            continue; /* skip whitespace */
        case INVALID:
            return 1; /* invalid input, return error */
        case EQUALS:  /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4)
            {
                if ((len += 3) > *outLen)
                    return 1; /* buffer overflow */
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0;
                iter = 0;
            }
        }
        numberOfChars++;
    }

    if (iter == 3)
    {
        if ((len += 2) > *outLen)
            return 1; /* buffer overflow */
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2)
    {
        if (++len > *outLen)
            return 1; /* buffer overflow */
        *(out++) = (buf >> 4) & 255;
    }

    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}

void strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.')
    {
        --end;
    }

    if (end > fname)
    {
        *end = '\0';
    }
}

void strip_process(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '-')
    {
        --end;
    }

    if (end > fname)
    {
        *end = '\0';
    }
}

void signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGUSR1)
    {
        float c = ((float)numberOfChars) / numberOfCharsTotal * 100;
        infof("%.2f%\n", c);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);

    char *fileToWrite = malloc(50 * sizeof(char));
    short flag = 0;

    if (argc < 3)
    {
        errorf("Usage: --<encode/decode> <file>.txt");
        return 0;
    }

    strcpy(fileToWrite, argv[2]);

    if (strcmp(argv[1], "--encode") == 0)
    {
        strip_ext(fileToWrite);
        strcat(fileToWrite, "-encoded.txt");
        flag = 1;
    }
    else if (strcmp(argv[1], "--decode") == 0)
    {
        strip_ext(fileToWrite);
        strip_process(fileToWrite);
        strcat(fileToWrite, "-decoded.txt");
        flag = 2;
    }
    else
    {
        errorf("Usage: --<encode/decode> <file>.txt");
        return 0;
    }

    FILE *fptr;
    if ((fptr = fopen64(argv[2], "r")) == NULL)
    {
        errorf("ERROR: file not found or cannot open\n");
        return 0;
    }

    fseeko64(fptr, 0L, SEEK_END);
    numberOfCharsTotal = ftello64(fptr);
    fseeko64(fptr, 0L, SEEK_SET);

    FILE *fptr2;
    if ((fptr2 = fopen64(fileToWrite, "w")) == NULL)
    {
        errorf("ERROR: file couldn't be created\n");
    }

    if (flag == 1)
    {
        size_t resultSize = (((4 * numberOfCharsTotal / 3) + 3) & ~3) + 1;
        char *data_buff = (char *)calloc(numberOfCharsTotal, sizeof(char));
        char *result = (char *)calloc(resultSize, sizeof(char));

        fread(data_buff, 1, numberOfCharsTotal, fptr);
        base64encode(data_buff, numberOfCharsTotal, result, resultSize);
        fprintf(fptr2, "%s", result);

        free(data_buff);
        free(result);
    }
    else if (flag == 2)
    {
        char *in = NULL;
        size_t inLen = 0;
        getline(&in, &inLen, fptr);

        size_t outLen = (3 * (inLen / 4) + 2);
        char *out = (char *)calloc(outLen, sizeof(char));
        base64decode(in, inLen, out, &outLen);

        fprintf(fptr2, "%s", out);
        free(out);
    }

    fclose(fptr);
    fclose(fptr2);
    return 0;
}
