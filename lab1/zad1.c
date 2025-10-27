#include <dirent.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_DIR_NAME_LENGTH 256
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
void usage(char* p)
{
    printf("Usage:%s -p Pathname -o output", p);
    exit(1);
}

int main(int argc, char** argv)
{
    DIR* dir;
    struct dirent* file;
    int opt;
    bool out = false;
    FILE* output;
    char* filename;
    struct stat file_stat;

    while ((opt = getopt(argc, argv, "p:o::")) != -1)
    {
        switch (opt)
        {
            case 'o':
                out = true;
                filename = optarg;
                break;
            case 'p':
                if (out == false)
                {
                    int i;
                    if ((i = chdir(optarg)) == -1)
                        ERR("getcwd failed");
                    if ((dir = opendir(".")) == NULL)
                        ERR("opendir fail");
                    printf("Sciezka:\n %s\n Lista plikow:\n", filename);
                    while ((file = readdir(dir)) != NULL)
                    {
                        if (stat(file->d_name, &file_stat) == 0)
                        {
                            printf("%s: %ld bytes\n", file->d_name, file_stat.st_size);
                        }
                    }
                }
                else
                {
                    int i;
                    if ((i = chdir(optarg)) == -1)
                        ERR("getcwd failed");
                    if ((dir = opendir(".")) == NULL)
                        ERR("opendir fail");
                    if ((output = fopen(filename, "w+")) == NULL)
                        ERR("fopen failed");
                    fprintf(output, "Sciezka:\n %s\n Lista plikow:\n", filename);
                    while ((file = readdir(dir)) != NULL)
                    {
                        if (stat(file->d_name, &file_stat) == 0)
                        {
                            printf("%s: %ld bytes\n", file->d_name, file_stat.st_size);
                        }
                    }
                }
                break;
            default:
                usage("./main");
        }
    }
    closedir(dir);
    return EXIT_SUCCESS;
}
