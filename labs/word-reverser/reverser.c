#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int i = 0;
    char c;
    char* word = malloc(100 * sizeof(char));

    while ((c = getchar()) != EOF)
    {
        if (c == '\n')
        {
            for (int j = i; j >= 0; j--)
            {
                printf("%c", word[j]);
            }
            i = 0;
            memset(word, 0, sizeof word);
            printf("\n");
        }
        else
        {
            word[i++] = c;
        }
    }
    free(word);
    return 0;
}
