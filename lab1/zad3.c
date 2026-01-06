#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define MAX_DIRECTORIES 256

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

char* parse_String(char* p)
{
    char v[256];
    char f[256];
    int i;

    if ((i = sscanf(p, "%[^=]==%s", f, v)) != 2)
        ERR("parse failed");
    char* output = malloc(strlen(f) + strlen(v) + 3);
    if (!output)
        ERR("malloc fail");
    strcat(output, f);
    strcat(output, " ");
    strcat(output, v);
    return output;
}
void install_packet(char* env_name, char* package)
{
    char* full_package = malloc(sizeof(char) * strlen(package) + 1);
    strcpy(full_package, package);
    char* name = strchr(full_package, '=');
    if (!name)
    {
        ERR("name null");
    }
    name[0] = '\0';
    printf("%s\n", name);
    struct stat ls;
    FILE* requirements;
    FILE* pack;
    int i;
    if (lstat(env_name, &ls) == 0)
    {
        if ((i = chdir(env_name) == -1))
            ERR("chdir failed");
        if ((requirements = fopen("requirements", "a+")) == NULL)
            ERR("fopen failed");
        if ((pack = fopen(full_package, "w+")) == NULL)
            ERR("fopen failed");
        if ((i = fputs(parse_String(package), requirements)) < 0)
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
    free(full_package);
}

void remove_packet(char* env_name, char* package)
{
    struct stat ls;
    FILE* file;
    int i;
    char  buf2[MAX_DIRECTORIES];
    /*if((i = chdir("lab1"))==-1)
        ERR("no env named lab1");*/
    printf("%s\n", getcwd(buf2,MAX_DIRECTORIES));
    if ((i = chdir(env_name)) == -1)
        ERR("no env named like that there");
    if ((file = fopen(package, "r")) == NULL)
        ERR("no package like that there");
    if (lstat(package, &ls) == -1)
        ERR("lstat fail");
    if(unlink(package) != 0)
        ERR("deleting file failed");
    char* buf = calloc(ls.st_size , sizeof(char));
    char f[30];
    char v[30];
    while((i = fscanf(file, "%s %s", f,v ))==2){
        if(strcmp(package,f)!= 0){
            strcat(buf,f);
            strcat(buf, " ");
            strcat(buf, v);
            strcat(buf, "\n");
        }
    }
    if(fclose(file) != 0)
        ERR("fclose failed");
    FILE* file2;
    if((file2 = fopen("requirements", "w")) == NULL)
        ERR("fopen failed but  in other way");
    fprintf(file2,"%s", buf);
    free(buf);
    if(fclose(file2) != 0)
        ERR("fclose failed");
    if((i = chdir("..")) == -1)
        ERR("COMEback fail");
}

int main(int argc, char** argv)
{
    int opt;
    char* packagename = NULL;
    char* env_names[MAX_DIRECTORIES];
    int env_counter = 0;
    char* package_to_delete;
    bool create = false;
    while ((opt = getopt(argc, argv, "cv:i:r:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                if (env_counter >= MAX_DIRECTORIES)
                    ERR("wybombiaj niunia");
                env_names[env_counter++] = optarg;
                break;
            case 'c':
                create = true;
                break;
            case 'i':
                packagename = optarg;
                break;
            case 'r':
                package_to_delete = optarg;
                break;
        }
    }
    if (create == true && env_names[0] != NULL)
        for (int i = 0; i < env_counter; i++)
        {
            create_environment(env_names[i]);
        }
    if (packagename != NULL)
        for (int i = 0; i < env_counter; i++)
            install_packet(env_names[i], packagename);
    if(package_to_delete != NULL)
        for (int i = 0; i< env_counter; i++)
            remove_packet(env_names[i], package_to_delete);
    return EXIT_SUCCESS;
}
