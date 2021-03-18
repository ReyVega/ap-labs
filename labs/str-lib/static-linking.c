#include <stdio.h>
#include <string.h>

extern int mystrlen(char *);
char *mystradd(char *, char *);
int mystrfind(char *, char *);

int main(int argc, char *argv[]) {
    
    if (argc == 4 && strcmp(argv[1], "-add") == 0) {
        printf("Initial Lenght      : %d\n", mystrlen(argv[2]));
        char *new_string = mystradd(argv[2],argv[3]);
        printf("New String          : %s\n", new_string);
        printf("New length          : %d\n", mystrlen(new_string));
        return 0;
    }

    if (argc == 4 && strcmp(argv[1], "-find") == 0) {
        int find = mystrfind(argv[2], argv[3]);
        if (find > -1) {
            printf("['%s'] string was found at [%d] position\n", argv[3], find);
        } else {
            printf("No ocurrences found");
        }
        return 0;
    }
    printf("Error: missing arguments or bad inputs\n");
    return 0;
}
