#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define MAX_PATH_NAME 4096
void usage(char* p)
{
    printf("Usage: %s -p Pathname -d Depth -e extention", p);
    exit(1);
}
void workDirectory(char* Pathname, FILE* file, char* extention, int given_depth)
{
    DIR* dir;
    struct dirent* dir_file;
    struct stat file_stat;
    char curr_pathname[MAX_PATH_NAME];
    char pwd[MAX_PATH_NAME];
    if (getcwd(curr_pathname, MAX_PATH_NAME) == NULL)
        ERR("getcwd failed");
    int i;
    if ((i = chdir(Pathname)) == -1)
        ERR("chdir failed");

    if ((dir = opendir(".")) == NULL)
        ERR("opendir failed");
    if (getcwd(pwd, MAX_PATH_NAME) == NULL)
        ERR("getcwd failed");
    // check if output file is given
    if (file == NULL)
    {
        printf("path: %s\n", Pathname);
    }
    else
    {
        fprintf(file, "path: %s\n", Pathname);
    }

    while ((dir_file = readdir(dir)) != NULL)
    {
        char fullpath[MAX_PATH_NAME];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", Pathname, dir_file->d_name);
        printf("DEBUG: Trying to stat '%s'\n", fullpath);
        if (stat(fullpath, &file_stat) == -1)
            ERR("stat failed");
        if (strcmp(dir_file->d_name, ".") == 0 || strcmp(dir_file->d_name, "..") == 0)
            continue;
        if (dir_file->d_type == DT_REG && strstr(dir_file->d_name, extention) != NULL)
        {
            if (file == NULL)
            {
                printf("%s\t%ld\n", dir_file->d_name, file_stat.st_size);
            }
            else
            {
                fprintf(file, "%s\t%ld\n", dir_file->d_name, file_stat.st_size);
            }
        }
        if (dir_file->d_type == DT_DIR && 1 < given_depth)
        {
            char newPathName[MAX_PATH_NAME];
            snprintf(newPathName, MAX_PATH_NAME, "%s/%s", Pathname, dir_file->d_name);
            workDirectory(newPathName, file, extention, given_depth - 1);
            if (chdir("..") == -1)
                ERR("chdir back failed");
        }
    }
    closedir(dir);
    if ((i = chdir(curr_pathname)) == -1)
        ERR("chdir failed");
}

int main(int argc, char** argv)
{
    char* extention = NULL;
    int opt;
    int given_depth = 1;
    FILE* output_file = NULL;
    char* filename = NULL;
    char* path = NULL;

    while ((opt = getopt(argc, argv, "p:d:e:o:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                given_depth = atoi(optarg);
                break;
            case 'e':
                extention = optarg;
                break;
            case 'o':
                filename = optarg;
                break;
            case 'p':
                path = optarg;
                break;
        }
    }
    if (path == NULL)
    {
        usage(argv[0]);
    }
    if (extention == NULL)
    {
        usage(argv[0]);
    }
    if (filename != NULL)
    {
        if ((output_file = fopen(filename, "w+")) == NULL)
            ERR("fopen failed");
    }

    workDirectory(path, output_file, extention, given_depth);
    return EXIT_SUCCESS;
}
