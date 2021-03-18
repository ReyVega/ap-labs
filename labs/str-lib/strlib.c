

int mystrlen(char *str)
{
    int len = 0;
    while (*str != '\0')
    {
        str++;
        len++;
    }
    return len;
}

char *mystradd(char *origin, char *addition)
{
    char *result = origin + mystrlen(origin);

    while (*addition != '\0')
    {
        *result++ = *addition++;
    }

    *result = '\0';
    return origin;
}

int mystrfind(char *origin, char *substr)
{
    int len = 0,
        len2 = 0,
        verification = mystrlen(substr),
        tmp = 0;

    while (origin[len] != '\0') {
        tmp = len;
        while (origin[len] == substr[len2] && origin[len] != '\0') {
            len++;
            len2++;
        }
        if (len2 == verification) {
            return tmp;
        }
        len++;
        len2 = 0;
    }
    return -1;
}
