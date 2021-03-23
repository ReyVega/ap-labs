#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINES 5000
char *lineptr[MAXLINES],
     *aux[500];

int readlines(FILE *input, int nlines);
void writelines(FILE *output, int nlines);
int numcmp(char *, char *);

void merge_sort(int, int, int (*comp)(void *, void *));

int readlines(FILE *input, int nlines)
{
    char *line;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, input) != -1))
    {
        if (nlines == MAXLINES)
        {
            printf("File too big to be sorted");
            exit(0);
        }
        lineptr[nlines] = malloc(strlen(line));
        strcpy(lineptr[nlines++], line);
    }
    return nlines;
}

void writelines(FILE *output, int nlines)
{
    for (int i = 0; i < nlines; i++)
    {
        fprintf(output, "%s",lineptr[i]);
    }
}

int numcmp(char *s1, char *s2)
{
    double v1,
        v2;
    v1 = atof(s1);
    v2 = atof(s2);
    if (v1 < v2)
    {
        return -1;
    }
    else if (v1 > v2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void merge_sort(int l, int r, int (*comp)(void *, void *))
{
    if (r <= l)
    {
        return;
    }

    int m = (l + r) / 2;
    merge_sort(l, m, comp);
    merge_sort(m + 1, r, comp);

    int pointer_left = l,
        pointer_right = m + 1,
        k;

    for (k = l; k <= r; k++)
    {
        if (pointer_left == m + 1)
        {
            aux[k] = malloc(strlen(lineptr[pointer_right]));
            strcpy(aux[k], lineptr[pointer_right]);
            pointer_right++;
        }
        else if (pointer_right == r + 1)
        {
            aux[k] = malloc(strlen(lineptr[pointer_left]));
            strcpy(aux[k], lineptr[pointer_left]);
            pointer_left++;
        }
        else if ((*comp)(lineptr[pointer_left], lineptr[pointer_right]) < 0)
        {
            aux[k] = malloc(strlen(lineptr[pointer_left]));
            strcpy(aux[k], lineptr[pointer_left]);
            pointer_left++;
        }
        else
        {
            aux[k] = malloc(strlen(lineptr[pointer_right]));
            strcpy(aux[k], lineptr[pointer_right]);
            pointer_right++;
        }
    }

    for (k = l; k <= r; k++)
    {
        strcpy(lineptr[k], aux[k]);
    }
}

int main(int argc, char *argv[])
{
    int nlines = 0,
        numeric = 0;
    FILE *input,
        *output;

    if (argc == 1)
    {
        printf("There are no parameters\n");
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-n") == 0)
    {
        printf("Incomplete parameters\n");
        return 0;
    }

    if (strcmp(argv[1], "-n") == 0)
    {
        numeric = 1;
        if ((input = fopen(argv[2], "r")) == NULL)
        {
            printf("Error: file not found or cannot open\n");
            return 0;
        }
    }
    else
    {
        if ((input = fopen(argv[1], "r")) == NULL)
        {
            printf("Error: file not found or cannot open\n");
            return 0;
        }
    }

    nlines = readlines(input, nlines);
    fclose(input);

    merge_sort(0, nlines - 1, (numeric ? (int (*)(void *, void *))numcmp : (int (*)(void *, void *))strcmp));

    char name_output[20] = "sorted_";
    strcat(name_output, (numeric ? argv[2] : argv[1]));

    if ((output = fopen(name_output, "w")) == NULL)
    {
        printf("Error: file cannot be created\n");
        return 0;
    }

    writelines(output, nlines);
    fclose(output);
    printf("Results file can be found at ./%s\n", name_output);
    return 0;
}
