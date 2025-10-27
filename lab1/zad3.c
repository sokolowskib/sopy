#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void create_environment(char* env_name)
{
    struct stat ls;
    FILE* requirements;
    int i;
    if (lstat(env_name, &ls) == -1)
    {
        if (mkdir(env_name, 0700) == -1)
            ERR("mkdir failed");
        if ((i = chdir(env_name) == -1))
            ERR("chdir failed");
        if ((requirements = fopen("requirements", "w")) == NULL)
            ERR("fopen failed");
        printf("udalo sie \n");
        if (chdir("..") == -1)
            ERR("chdir coming back failed");
        fclose(requirements);
    }
    else
    {
        perror("environment already exists, pick other name or chose another operation");
        exit(1);
    }
}
void install_packet(char* env_name, char* package)
{
    struct stat ls;
    FILE* requirements;
    int i;
    if (lstat(env_name, &ls) == 0)
    {
        if ((i = chdir(env_name) == -1))
            ERR("chdir failed");
        if ((requirements = fopen("requirements", "a+")) == NULL)
            ERR("fopen failed");
        if ((i = fputs(package, requirements)) < 0)
            ERR("putting into files failed");
        fprintf(requirements, "\n");
        if (chdir("..") == -1)
            ERR("chdir coming back failed");
        fclose(requirements);
    }
    else
    {
        ERR("env doesnt exist");
    }
}
void remove_packet(char* env_name, char* package) {}

int main(int argc, char** argv)
{
    int opt;
    char* envname = NULL;
    char* packagename = NULL;
    bool create = false;
    while ((opt = getopt(argc, argv, "cv:i:r:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                envname = optarg;
                break;
            case 'c':
                create = true;
                break;
            case 'i':
                packagename = optarg;
                break;
        }
    }
    if (create == true && envname != NULL)
        create_environment(envname);
    if (packagename != NULL)
        install_packet(envname, packagename);

    return EXIT_SUCCESS;
}
