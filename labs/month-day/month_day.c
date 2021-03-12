#include <stdio.h>
#include <stdlib.h>

static int plus = 0;

static char daytab[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/* month_day function's prototype*/
void month_day(int year, int yearday, int *pmonth, int *pday);

char *month_name(int n) {

   static char *name[] = {
       "Illegal month",
       "January", "February", "March",
       "April", "May", "June",
       "July", "August", "September",
       "October", "November", "December"
   };

   return (n < 1 || n > 12) ? name[0] : name[n];
}

void month_day(int year, int yearday, int *pmonth, int *pday) {
    int leap = year%4 == 0 && year%100 != 0 || year%400 == 0;
    *pday = yearday;

    do {
        for (int i = 1; i < 13; i++) {
            *pmonth = i;

            if (*pday <= 31) {           
                return;
            }
            *pday -= daytab[leap][i - 1];
        }
        year++;
        leap = year%4 == 0 && year%100 != 0 || year%400 == 0;
        plus++;
    } while(*pday > 31);
}


int main(int argc, char **argv) {
    int year = atoi(argv[1]),
        yearday = atoi(argv[2]),
        pmonth = 0,
        pday = 0;

    month_day(year, yearday, &pmonth, &pday);
    printf("%s %d, %d\n", month_name(pmonth), pday, year + plus);
    return 0;
}
