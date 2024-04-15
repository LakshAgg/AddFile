/**
 * @file main.c
 * @author Laksh Aggarwal (agglaksh378@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-14
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "Helper.h"

#ifdef linux
#define PATH_SEPARATOR "/"
#elif __APPLE__
#define PATH_SEPARATOR "/"
#else
#define PATH_SEPARATOR "\\"
#endif

#define CONFIG_FILE ".addfile.config"
#define CONFIG_FILE_ PATH_SEPARATOR ".config" PATH_SEPARATOR CONFIG_FILE

#define S_ "\033["
#define Red "31"
#define Green "32"
#define Blue "34"
#define Bold "1;"
#define White "37"
#define Invert "7;"
#define Reset "0"
#define _E "m"

#define BOLDWHITE S_ Bold White _E
#define BOLDRED S_ Bold Red _E
#define BOLDGREEN S_ Bold Green _E
#define RESET S_ Reset _E

#define error(msg)                         \
    {                                      \
        printf(BOLDRED "%s\n" RESET, msg); \
        return 1;                          \
    }

int main()
{
    // Get Home directory of user
    struct passwd *w = getpwuid(getuid());
    if (w == NULL)
        error("Could not get user home directory.");
    char *home_dir = w->pw_dir;

    char config_path[FILENAME_MAX + 1];
    char config_path2[FILENAME_MAX + 2];

    // copy home directory path
    strncpy(config_path, home_dir, FILENAME_MAX);

    // append the config path
    if (strlen(config_path) + strlen(CONFIG_FILE_) > FILENAME_MAX)
        error("Something is horribly wrong.");
    strcat(config_path, CONFIG_FILE_);

    // attempt to open the config file
    FILE *f;
    memset(config_path2, 0, sizeof(config_path2));
    if (access(config_path, F_OK) == 0)
    {
        f = fopen(config_path, "r");

        if (f == NULL)
            error("Failed to open configuration file.");
        // Read the config path
        fread(config_path2, sizeof(char), sizeof(config_path2), f);

        printf("%s\n", config_path2);
        if (access(config_path2, F_OK) != 0)
        {
            fclose(f);
            remove(config_path);
            error("The file does not exist.");
        }
    }
    else
    {
        f = fopen(config_path, "w");
        if (f == NULL)
            error("Failed to open configuration file.");

        printf(BOLDGREEN "Please enter the path to the configuration file: " RESET);
        fgets(config_path2, FILENAME_MAX, stdin);
        config_path2[strlen(config_path2) - 1] = 0;

        if (access(config_path2, F_OK) != 0)
        {
            fclose(f);
            remove(config_path);
            printf("%s\n", config_path2);
            error("The file does not exist.");
        }

        fwrite(config_path2, 1, sizeof(config_path2), f);
    }

    fclose(f);

    // open the config if it exists
    f = fopen(config_path2, "r");
    if (f == NULL)
        error("Failed to open set config file.");

    enum state
    {
        VR, // reading variable name
        VL, // reading value of var
        T,  // reading text content
        FL  // reading file extension
    };
    enum state state = VR;

#define CHUNK_SIZE 1024 * 16
    char buff[CHUNK_SIZE];

#define errorc(msg) \
    {               \
        fclose(f);  \
        error(msg); \
    }

    string var = new_string();
    if (!var)
        errorc("Memory error");

    string temp = new_string();
    if (!temp)
    {
        free(var);
        errorc("Memory error");
    }

    string val = new_string();
    if (!val)
    {
        free(var);
        free(temp);
        errorc("Memory error");
    }

    string *var_val = malloc(sizeof(string) * 2);
    if (!var_val)
    {
        free(var);
        free(val);
        free(temp);
        error("Memory error");
    }
    int vars = 0;

#define errorfree(msg)                 \
    {                                  \
        free(var);                     \
        free(val);                     \
        for (int i = 0; i < vars; i++) \
        {                              \
            free(var_val[i * 2]);      \
            free(var_val[i * 2 + 1]);  \
        }                              \
        free(var_val);                 \
        if (temp)                      \
            free(temp);                \
        errorc(msg);                   \
    }
#define merrorfree() errorfree("Memory error");

    int read;
    while ((read = fread(buff, sizeof(char), CHUNK_SIZE, f)) != 0)
    {
        char new_line = 1, skip = 0, dash = 0, escape = 0, read_v = 0;
        for (int i = 0; i < read; i++)
        {
            char c = buff[i];
            if (skip)
            {
                if (c == '\n')
                {
                    skip = 0;
                    new_line = 1;
                }
                continue;
            }

            if (new_line)
            {
                if (state == VR)
                {
                    if (c == '#')
                    {
                        skip = 1;
                        continue;
                    }
                    else if (c == '-')
                    {
                        if (dash)
                        {
                            new_line = 0;
                            state = FL;
                            dash = 0;
                        }
                        else
                            dash++;
                    }
                    else
                    {
                        if (dash)
                        {
                            if (!append_char(&var, '-'))
                                merrorfree();
                        }
                        else if (c == '\n')
                        {
                            new_line = 1;
                            continue;
                        }
                        if (!append_char(&var, c))
                            merrorfree();
                        dash = 0;
                        new_line = 0;
                    }
                }
                else if (state == T)
                {
                    if (!escape && c == '\\' && !dash)
                    {
                        escape = 1;
                        continue;
                    }
                    if (c == '-')
                    {
                        if (dash)
                        {
                            if (escape)
                            {
                                if (!append_char(&val, '-') || !append_char(&val, '-'))
                                    merrorfree();
                                escape = 0;
                                new_line = 0;
                            }
                            else
                            {
                                printf("\"%s %s\"\n", var->s, val->s);
                                empty_s(var);
                                empty_s(val);
                                new_line = 0;
                                state = VR;
                            }
                            dash = 0;
                        }
                        else
                            dash++;
                    }
                    else
                    {
                        if (dash)
                        {
                            if (escape && !append_char(&val, '\\'))
                                merrorfree();
                            if (!append_char(&val, '-'))
                                merrorfree();
                            escape = 0;
                            if (c == '$')
                                read_v = 1;
                            else if (c == '\\')
                                escape = 1;
                            else if (!append_char(&val, c))
                                merrorfree();
                            if (c == '\n')
                            {
                                dash = 0;
                                continue;
                            }
                        }
                        else
                        {
                            if (c == '$')
                            {
                                if (!escape)
                                {
                                    read_v = 1;
                                    new_line = 0;
                                    continue;
                                }
                                else if (!append_char(&val, c))
                                    merrorfree();
                                escape = 0;
                            }
                            else
                            {
                                if (escape && !append_char(&val, '\\'))
                                    merrorfree();
                                if (c == '\n')
                                {
                                    escape = 0;
                                    continue;
                                }
                                else if (!append_char(&val, c))
                                    merrorfree();
                                escape = 0;
                            }
                        }

                        dash = 0;
                        new_line = 0;
                    }
                }
                continue;
            }
            switch (state)
            {
            case VR:
            {
                if (c == ' ')
                {
                    empty_s(var);
                    skip = 1;
                }
                else if (c == '=')
                    state = VL;
                else if (c == '\n')
                    new_line = 1;
                else if (!append_char(&var, c))
                    merrorfree();
            }
            break;
            case FL:
            {
                if (c == '\n')
                {
                    trim(var);
                    state = T;
                }
                else if (!append_char(&var, c))
                    merrorfree();
            }
            break;
            case VL:
            {
                if (c == '\n')
                {
                    void *temp = realloc(var_val, sizeof(string) * (vars + 1) * 2);
                    if (!temp)
                        merrorfree();
                    var_val = temp;
                    var_val[vars * 2] = var;
                    var_val[vars * 2 + 1] = val;

                    string nvar = new_string();
                    if (!nvar)
                        merrorfree();

                    string nval = new_string();
                    if (!nval)
                    {
                        free(nvar);
                        merrorfree();
                    }
                    vars++;
                    new_line = 1;
                    var = nvar;
                    val = nval;
                    state = VR;
                }
                else if (!append_char(&val, c))
                    merrorfree();
            }
            break;
            case T:
            {
                if (read_v)
                {
                    if (c == '\n')
                        new_line = 1;

                    if (c == ' ' || c == '\n')
                    {
                        char found = 0;
                        read_v = 0;
                        for (int _i = 0; _i < vars; _i++)
                        {
                            if (var_val[_i * 2]->len == temp->len && strcmp(var_val[_i * 2]->s, temp->s) == 0)
                            {
                                if (!string_append(&val, var_val[_i * 2 + 1]))
                                    merrorfree();
                                found = 1;
                            }
                        }

                        if (!found)
                        {
                            printf(BOLDRED "Variable \"%s\" has not been assigned\n" RESET, temp->s);
                            errorfree("Variable error");
                        }
                        if (c == '\n' && !append_char(&val, c))
                            merrorfree();
                        empty_s(temp);
                    }
                    else if (!append_char(&temp, c))
                        merrorfree();
                    continue;
                }

                if (c == '\\')
                {
                    if (escape && !append_char(&val, '\\'))
                        merrorfree();
                    escape = 1;
                    continue;
                }

                if (escape)
                {
                    if (c != '$')
                        if (!append_char(&val, '\\'))
                            merrorfree();
                    escape = 0;
                }
                else if (c == '$')
                {
                    read_v = 1;
                    continue;
                }

                if (c == '\n')
                {
                    if (escape && !append_char(&val, '\\'))
                        merrorfree();
                    new_line = 1;
                }
                if (!append_char(&val, c))
                    merrorfree();
            }
            break;
            default:
                break;
            }
        }
    }

    fclose(f);
    free(var);
    free(val);
}