#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// Struct used within the implementation
struct Package
{
    char name[100],
        installDate[100],
        lastUpdate[100],
        removalDate[100];
    int updates;
};

void analizeLog(char *logFile, char *report);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage:./pacman-analizer.o -input file.txt -report file.txt\n");
        return 1;
    }
    // Validating number of arguments
    else if (argc != 5)
    {
        printf("Error: invalid arguments\n");
        return 1;
    }

    // Validating that plug calls are correct
    if (strcmp(argv[1], "-input") != 0 || strcmp(argv[3], "-report") != 0)
    {
        printf("Error: invalid plugs call\n");
        return 1;
    }

    analizeLog(argv[2], argv[4]);
    return 0;
}

void analizeLog(char *logFile, char *report)
{
    printf("Generating Report from: [%s] log file\n", logFile);

    // Implement your solution here.
    // Variables for the output
    struct Package pk[3000];

    int installedPackages = 0,
        removedPackages = 0,
        upgradedPackages = 0,
        currentInstalled = 0;

    char oldestPackage[100] = "",
         newestPackage[100] = "",
         packagesNotUpgraded[5000] = "";

    int alpm_scriptletCount = 0,
        alpmCount = 0,
        pacmanCount = 0,
        indexPackage = 0;

    // Read file
    FILE *fptr;

    // Validated if file was found or if cannot be opened
    if ((fptr = fopen(logFile, "r")) == NULL)
    {
        printf("Error: file not found or cannot open\n");
        return;
    }

    // Variables for reading lines
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Read file
    while ((read = getline(&line, &len, fptr)) != -1)
    {
        // Validate if its a possible entry
        if (line[0] != '[')
        {
            continue;
        }

        // Variables used to traverse the line and obtain keywords
        int index = 0;
        int i = 1;

        // Getting date
        char date[50];
        while (line[i] != ']')
        {
            date[index++] = line[i];
            i++;
        }
        date[index] = '\0';

        // Getting type count
        char type[50];
        index = 0;
        i += 3;
        while (line[i] != ']')
        {
            type[index++] = line[i];
            i++;
        }
        type[index] = '\0';

        // Getting action
        char action[50];
        index = 0;
        i += 2;
        while (line[i] != ' ')
        {
            action[index++] = line[i];
            i++;
        }
        action[index] = '\0';

        // Count types viewed within the file
        if (strcmp(type, "PACMAN") == 0)
        {
            pacmanCount++;
        }
        else if (strcmp(type, "ALPM") == 0)
        {
            alpmCount++;
        }
        else if (strcmp(type, "ALPM-SCRIPTLET") == 0)
        {
            alpm_scriptletCount++;
        }

        // Getting package name if action is valid; otherwise ignore all the process and continue to the next line
        char pName[50];
        if (strcmp(action, "installed") == 0 || strcmp(action, "upgraded") == 0 || strcmp(action, "removed") == 0 || strcmp(action, "reinstalled") == 0)
        {
            index = 0;
            i += 1;
            while (line[i] != ' ')
            {
                pName[index++] = line[i];
                i++;
            }
            pName[index] = '\0';
        }
        else
        {
            continue;
        }

        // Managing packages
        // Install new package; initialize new struct within the array of packages
        if (strcmp(action, "installed") == 0)
        {
            strcpy(pk[indexPackage].name, pName);
            strcpy(pk[indexPackage].installDate, date);
            strcpy(pk[indexPackage].lastUpdate, "-");
            pk[indexPackage].updates = 0;
            strcpy(pk[indexPackage].removalDate, "-");
            indexPackage++;
        }
        // Upgraded packages; look for the package, increase update variable and change latest update date
        else if (strcmp(action, "upgraded") == 0)
        {
            for (int i = 0; i < indexPackage; i++)
            {
                if (strcmp(pk[i].name, pName) == 0)
                {
                    strcpy(pk[i].lastUpdate, date);
                    pk[i].updates++;
                    break;
                }
            }
        }
        // Reinstalled packages; look for the package and reset all his values to default, 
        // of course package will have now a newest date of installation
        else if (strcmp(action, "reinstalled") == 0)
        {
            bool flag = true;
            for (int i = 0; i < indexPackage; i++)
            {
                if (strcmp(pk[i].name, pName) == 0)
                {
                    strcpy(pk[i].installDate, date);
                    strcpy(pk[i].lastUpdate, "-");
                    pk[i].updates = 0;
                    strcpy(pk[i].removalDate, "-");
                    flag = false;
                    break;
                }
            }
            // If package was not found I considered it as a new installation
            // Note: this happens a lot in pacman2.txt file
            // Packages reinstalled without having history that they have been
            // previously installed 
            if (flag)
            {
                strcpy(pk[indexPackage].name, pName);
                strcpy(pk[indexPackage].installDate, date);
                strcpy(pk[indexPackage].lastUpdate, "-");
                pk[indexPackage].updates = 0;
                strcpy(pk[indexPackage].removalDate, "-");
                indexPackage++;
            }
        }
        // Removed packages; look for the package and add the removal date
        else if (strcmp(action, "removed") == 0)
        {
            for (int i = 0; i < indexPackage; i++)
            {
                if (strcmp(pk[i].name, pName) == 0)
                {
                    strcpy(pk[i].removalDate, date);
                    break;
                }
            }
        }
    }
    // Close file
    fclose(fptr);

    // Oldest package: look for the first package which doesn't have a removal date
    for (int i = 0; i < indexPackage; i++)
    {
        if (strcmp(pk[i].removalDate, "-") == 0)
        {
            strcpy(oldestPackage, pk[i].name);
            break;
        }
    }

    // Newest package; look for the last package which doesn't hace a removal date
    for (int i = indexPackage; i >= 0; i--)
    {
        if (strcmp(pk[i].removalDate, "-") == 0)
        {
            strcpy(newestPackage, pk[i].name);
            break;
        }
    }

    // Managing package count, packages upgraded, current installed, removed, etc.
    int k = 0;
    for (int i = 0; i < indexPackage; i++)
    {
        // For each element we know there is a package, so we counted it
        installedPackages++;

        // If package has a removal date, counted as a removed package and continue with
        // the next package, otherwise we counted as current installed
        if (strcmp(pk[i].removalDate, "-") != 0)
        {
            removedPackages++;
            continue;
        }
        else
        {
            currentInstalled++;
        }

        // If package has updates we counted as an upgraded package, of course I'm not considering
        // removed packages that's why the "if" before
        if (pk[i].updates > 0)
        {
            upgradedPackages++;
        }
        // Managing string for not upgraded packages
        else
        {
            strcat(packagesNotUpgraded, strcat(pk[i].name, ", "));
            k++;
            if (k == 20)
            {
                strcat(packagesNotUpgraded, "\n-                              : ");
                k = 0;
            }
        }
    }

    // Write file (System call: "open")
    int fptr2 = open(report, O_CREAT | O_WRONLY, 0600);

    // Validate that the file could be created
    if (fptr2 < 0)
    {
        printf("Error: technical error ocurred when creating the file");
        return;
    }

    // Managing string that has the statistics of the packages
    char msg[6000];
    snprintf(msg, 6000, "Pacman Packages Report\n"
                        "----------------------\n"
                        "- Installed packages : %d \n"
                        "- Removed packages   : %d \n"
                        "- Upgraded packages  : %d \n"
                        "- Current installed  : %d \n"
                        "-------------\n"
                        "General Stats\n"
                        "-------------\n"
                        "- Oldest package               : %s \n"
                        "- Newest package               : %s \n"
                        "- Package with no upgrades     : %s \n"
                        "- [ALPM-SCRIPTLET] type count  : %d \n"
                        "- [ALPM] count                 : %d \n"
                        "- [PACMAN] count               : %d \n"
                        "----------------\n"
                        "List of packages\n"
                        "----------------\n",
             installedPackages,
             removedPackages,
             upgradedPackages,
             currentInstalled,
             oldestPackage,
             newestPackage,
             packagesNotUpgraded,
             alpm_scriptletCount,
             alpmCount,
             pacmanCount);

    // Write statistics within the file (System call: "write")
    write(fptr2, msg, strlen(msg));

    // Managing all packages to be printed within the output file
    for (int i = 0; i < indexPackage; i++)
    {
        char msgP[1000];
        snprintf(msgP, 1000, "- Package Name         : %s \n"
                             "   - Install date      : %s \n"
                             "   - Last update date  : %s \n"
                             "   - How many updates  : %d \n"
                             "   - Removal date      : %s \n",
                 pk[i].name,
                 pk[i].installDate,
                 pk[i].lastUpdate,
                 pk[i].updates,
                 pk[i].removalDate);
        write(fptr2, msgP, strlen(msgP));
    }
    // Close output file (System call: "close")
    close(fptr2);

    printf("Report is generated at: [%s]\n", report);
}