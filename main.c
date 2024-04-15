/**
 * @file main.c
 * @author Laksh Aggarwal (agglaksh378@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-14
 */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "Helper.h"

#ifdef linux
#define PATH_SEPARATOR "/"
#define PATH_SEPARATORC '/'
#elif __APPLE__
#define PATH_SEPARATOR "/"
#define PATH_SEPARATORC '/'
#else
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATORC '\\'
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

#define CHUNK_SIZE 1024 * 16

#define error(msg)                         \
    {                                      \
        printf(BOLDRED "%s\n" RESET, msg); \
        exit(1);                           \
    }

#define errorc(msg) \
    {               \
        fclose(f);  \
        error(msg); \
    }

FILE *get_config_file(char *filename);

void set_config(char *config_path2);

string *get_fe_tc(FILE *f, int *count, string *var_val, int vars, int ow, void *tbf);

int assign_var(string var, string val, string **vrvl, int *varsp, int ignore_overwrite);

string get_val(string var, string *vrvl, int vars);

int get_val_index(string var, string *vrvl, int vars);

int main(int argc, char **argv)
{
    if (argc < 2)
        error("Invalid Usage. Use -h for help.");
    if (strcmp(argv[1], "-s") == 0)
    {
        char *new;
        if (argc == 3)
            new = argv[2];
        else if (argc == 2)
        {
            new = malloc(sizeof(char) * (FILENAME_MAX + 1));
            if (!new)
                error("Memory error");

            printf(BOLDGREEN "Please enter the path to the configuration file: " RESET);
            fgets(new, FILENAME_MAX, stdin);
            new[strlen(new) - 1] = 0;
        }
        else
            error("Invalid Usage. Use -h for help.");
        set_config(new);
    }

    char *filename = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            if (i == argc - 1)
                error("Invalid Usage for -c.");
            filename = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-s") == 0)
            error("Invalid usage for -s\n");
    }

    FILE *f = get_config_file(filename);

    char create_dir = 0;

    string *fl_ct;
    string *var_val = malloc(sizeof(string) * 2);
    if (!var_val)
        errorc("Memory error");

    char *name = NULL;
    char **fltypes = malloc(sizeof(char *));
    int count = 0;

    if (fltypes == NULL)
    {
        free(var_val);
        errorc("Memory Error.");
    }

    int fls, vars = 0;

#define free_var_val                         \
    {                                        \
        free(fltypes);                       \
        for (int __i = 0; __i < vars; __i++) \
        {                                    \
            free(var_val[__i * 2]);          \
            free(var_val[__i * 2 + 1]);      \
        }                                    \
        free(var_val);                       \
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
            create_dir = 1;
        else if (strcmp(argv[i], "-v") == 0)
        {
            if (i > argc - 3)
            {
                free_var_val;
                errorc("Invalid Usage for -v.");
            }
            else
            {
                string var = new_string_from_char(argv[i + 1]);
                if (!var)
                {
                    free_var_val;
                    errorc("Invalid Usage for -v.");
                }
                trim(var);
                string val = new_string_from_char(argv[i + 2]);
                if (!val)
                {
                    free(var);
                    free_var_val;
                    errorc("Invalid Usage for -v.");
                }
                if (!assign_var(var, val, &var_val, &vars, 0))
                {
                    free(var);
                    free(val);
                    free_var_val;
                    errorc("Memory error");
                }
                i += 2;
            }
        }
        else if (name == NULL)
            name = argv[i];
        else
        {
            char **temp = realloc(fltypes, sizeof(char *) * (count + 1));
            if (temp == NULL)
            {
                free(fltypes);
                free_var_val;
                errorc("Memory error");
            }
            fltypes = temp;
            fltypes[count++] = argv[i];
        }
    }

#define free_error              \
    {                           \
        free_var_val;           \
        if (var)                \
            free(var);          \
        if (val)                \
            free(val);          \
        errorc("Memory error"); \
    }
    int ow = vars;
    {
        string val = new_string_from_char(name);
        string var = new_string_from_char("NAME");
        if (!var || !val || !assign_var(var, val, &var_val, &vars, ow))
            free_error;

        val = new_string_from_char(name);
        for (int i = 0; val->s[i]; i++)
            if (val->s[i] >= 'a' && val->s[i] <= 'z')
                val->s[i] -= 'a' - 'A';
        var = new_string_from_char("NAMECAP");
        if (!var || !val || !assign_var(var, val, &var_val, &vars, ow))
            free_error;
        char temp[1024];

        time_t t = time(0);
        struct tm t2;

        localtime_r(&t, &t2);
        sprintf(temp, "%02d%02d%04d", t2.tm_mday, t2.tm_mon, t2.tm_year + 1900);
        val = new_string_from_char(temp);
        var = new_string_from_char("DDMMYY");
        if (!var || !val || !assign_var(var, val, &var_val, &vars, ow))
            free_error;
        sprintf(temp, "%04d", t2.tm_year + 1900);
        val = new_string_from_char(temp);
        var = new_string_from_char("YEAR");
        if (!var || !val || !assign_var(var, val, &var_val, &vars, ow))
            free_error;
    }

#undef free_var_val
#undef free_error
    fl_ct = get_fe_tc(f, &fls, var_val, vars, ow, fltypes);

#define free_error(msg)               \
    {                                 \
        if (file)                     \
            free(file);               \
        if (var)                      \
            free(var);                \
        for (int i = 0; i < fls; i++) \
        {                             \
            free(fl_ct[i * 2]);       \
            free(fl_ct[i * 2 + 1]);   \
        }                             \
        free(fl_ct);                  \
        free(fltypes);                \
        error(msg);                   \
    }

    string file = new_string(), var = new_string();
    if (!file || !var)
        free_error("Memory error");

    if (create_dir)
    {
        if (mkdir(&(name[0]), 0700) != 0)
            free_error("Failed to create directory with the same name.");
        printf(BOLDGREEN "Created directory: " BOLDWHITE "%s" RESET "\n", name);
        if (!string_append_char(&file, name) || !append_char(&file, PATH_SEPARATORC))
            free_error("Memory Error");
    }

    if (!string_append_char(&file, name))
        free_error("Memory error");

    unsigned long len = file->len;
    for (int i = 0; i < count; i++)
    {
        file->len = len;
        file->s[len] = 0;
        empty_s(var);

        if (!string_append_char(&file, fltypes[i]) || !string_append_char(&var, fltypes[i]))
            free_error("Memory error");
        f = fopen(file->s, "w");
        if (!f)
            free_error("Could not open the file.");
        string val = get_val(var, fl_ct, fls);
        printf(BOLDGREEN "Opened file" BOLDWHITE " %s" RESET "\n", file->s);
        if (val)
        {
            unsigned long done = 0;
            while (done != val->len)
                done += fwrite(val->s + done, sizeof(char), val->len - done, f);
            printf(BOLDGREEN "Writing template data to file" BOLDWHITE " %s" RESET "\n", file->s);
        }
        fclose(f);
    }

    free(var);
    free(file);
    for (int i = 0; i < fls; i++)
    {
        free(fl_ct[i * 2]);
        free(fl_ct[i * 2 + 1]);
    }
    free(fl_ct);
    free(fltypes);
#undef free_error
}

string *get_fe_tc(FILE *f, int *count, string *var_val, int vars, int ow, void *tbf)
{
#define errorfree(msg)                     \
    {                                      \
        printf(BOLDRED "%s\n" RESET, msg); \
        goto freeall;                      \
    }

#define merrorfree() errorfree("Memory error");

    string var, val, temp;
    string *fl_ct;
    int fls = 0;
    int ignore_overwrite = ow;

    if (0)
    {
    freeall:
        if (var)
            free(var);
        if (val)
            free(val);
        if (var_val)
        {
            for (int i = 0; i < vars; i++)
            {
                free(var_val[i * 2]);
                free(var_val[i * 2 + 1]);
            }
            free(var_val);
        }
        if (fl_ct)
        {
            for (int i = 0; i < fls; i++)
            {
                free(fl_ct[i * 2]);
                free(fl_ct[i * 2 + 1]);
            }
            free(fl_ct);
        }
        if (temp)
            free(temp);
        fclose(f);
        free(tbf);
        exit(1);
    }

    enum state
    {
        VR, // reading variable name
        VL, // reading value of var
        T,  // reading text content
        FL  // reading file extension
    };
    enum state state = VR;

    char buff[CHUNK_SIZE];

    var = new_string();
    if (!var)
        merrorfree();

    temp = new_string();
    if (!temp)
        merrorfree();

    val = new_string();
    if (!val)
        merrorfree();

    fl_ct = malloc(sizeof(string) * 2);
    if (!fl_ct)
        merrorfree();

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
                                string tc = get_val(var, fl_ct, fls);
                                if (tc)
                                {
                                    printf(BOLDRED "Template for file %s has already been defined.\n" RESET, var->s);
                                    errorfree("Duplicate file extension.");
                                }
                                if (!assign_var(var, val, &fl_ct, &fls, 0))
                                    merrorfree();
                                var = NULL;
                                val = NULL;

                                var = new_string();
                                if (!var)
                                    merrorfree();
                                val = new_string();
                                if (!val)
                                    merrorfree();
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
                    if (!assign_var(var, val, &var_val, &vars, ignore_overwrite))
                        merrorfree();
                    var = NULL;
                    val = NULL;

                    var = new_string();
                    if (!var)
                        merrorfree();
                    val = new_string();
                    if (!val)
                        merrorfree();
                    new_line = 1;
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
                        read_v = 0;
                        string value = get_val(temp, var_val, vars);

                        if (!value)
                        {
                            printf(BOLDRED "Variable \"%s\" has not been assigned\n" RESET, temp->s);
                            errorfree("Variable error");
                        }
                        if (!string_append(&val, value))
                            merrorfree();

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
    free(temp);
    free(val);
    for (int i = 0; i < vars; i++)
    {
        free(var_val[i * 2]);
        free(var_val[i * 2 + 1]);
    }
    free(var_val);
    *count = fls;
    return fl_ct;
#undef merrorfree
#undef errorfree
}

string get_val(string var, string *vrvl, int vars)
{
    for (int i = 0; i < vars; i++)
    {
        if (vrvl[i * 2]->len == var->len && strcmp(var->s, vrvl[i * 2]->s) == 0)
        {

            return vrvl[i * 2 + 1];
        }
    }
    return NULL;
}

int get_val_index(string var, string *vrvl, int vars)
{
    for (int i = 0; i < vars; i++)
    {
        if (vrvl[i * 2]->len == var->len && strcmp(var->s, vrvl[i * 2]->s) == 0)
        {

            return i;
        }
    }
    return vars;
}

int assign_var(string var, string val, string **vrvlp, int *varsp, int ignore_overwrite)
{
    string *vrvl = *vrvlp;
    for (int i = 0; i < *varsp; i++)
    {
        if (vrvl[i * 2]->len == var->len && strcmp(var->s, vrvl[i * 2]->s) == 0)
        {
            if (i >= ignore_overwrite)
            {
                free(vrvl[i * 2 + 1]);
                free(vrvl[i * 2]);
                vrvl[i * 2] = var;
                vrvl[i * 2 + 1] = val;
            }
            else
            {
                free(var);
                free(val);
            }
            return 1;
        }
    }
    int vars = *varsp;
    vrvl = realloc(vrvl, sizeof(string) * (vars + 1) * 2);
    if (!vrvl)
        return 0;

    vrvl[vars * 2] = var;
    vrvl[vars * 2 + 1] = val;
    *vrvlp = vrvl;
    *varsp = vars + 1;
    return 1;
}

void set_config(char *config_path2)
{
    struct passwd *w = getpwuid(getuid());
    if (w == NULL)
        error("Could not get user home directory.");
    char *home_dir = w->pw_dir;

    char config_path[FILENAME_MAX + 1];

    // copy home directory path
    strncpy(config_path, home_dir, FILENAME_MAX);

    // append the config path
    if (strlen(config_path) + strlen(CONFIG_FILE_) > FILENAME_MAX)
        error("Something is horribly wrong.");
    strcat(config_path, CONFIG_FILE_);

    // attempt to open the config file
    FILE *f;
    memset(config_path2, 0, sizeof(config_path2));
    f = fopen(config_path, "w");
    if (f == NULL)
        error("Failed to open configuration file.");

    if (access(config_path2, F_OK) != 0)
    {
        fclose(f);
        remove(config_path);
        printf("%s\n", config_path2);
        error("The file does not exist.");
    }

    fwrite(config_path2, 1, sizeof(config_path2), f);
    printf(BOLDGREEN "Successfully set config to %s\n", config_path2);
    fclose(f);
    exit(0);
}

FILE *get_config_file(char *filename)
{
    FILE *f;

    char config_path[FILENAME_MAX + 1];
    char config_path2[FILENAME_MAX + 2];

    if (filename == NULL)
    {
        // Get Home directory of user
        struct passwd *w = getpwuid(getuid());
        if (w == NULL)
            error("Could not get user home directory.");
        char *home_dir = w->pw_dir;

        // copy home directory path
        strncpy(config_path, home_dir, FILENAME_MAX);

        // append the config path
        if (strlen(config_path) + strlen(CONFIG_FILE_) > FILENAME_MAX)
            error("Something is horribly wrong.");
        strcat(config_path, CONFIG_FILE_);

        // attempt to open the config file
        memset(config_path2, 0, sizeof(config_path2));
        if (access(config_path, F_OK) == 0)
        {
            f = fopen(config_path, "r");

            if (f == NULL)
                error("Failed to open configuration file.");
            // Read the config path
            fread(config_path2, sizeof(char), sizeof(config_path2), f);

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
    }
    else
    {
        strncpy(config_path2, filename, FILENAME_MAX);
        if (access(config_path2, F_OK) != 0)
        {
            fclose(f);
            remove(config_path);
            printf("%s\n", config_path2);
            error("The file does not exist.");
        }
    }
    // open the config if it exists
    f = fopen(config_path2, "r");
    if (f == NULL)
        error("Failed to open set config file.");
    return f;
}