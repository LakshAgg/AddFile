/**
 * @file Helper.c
 * @author Laksh Aggarwal (aggarwallaksh54@gmail.com)
 * @version 0.1
 *
 */
#include <string.h>
#include "Helper.h"
#include <stdlib.h>

string new_string()
{
    string rv = malloc(sizeof(*rv) + 1);
    if (rv)
    {
        rv->alloc = 1;
        rv->len = 0;
        rv->s = (char *)(rv + 1);
        rv->s[0] = 0;
    }

    return rv;
}

string new_string_from_char(char *s)
{
    unsigned long len = strlen(s);
    string rv = malloc(sizeof(*rv) + 1 + len);
    if (rv)
    {
        rv->alloc = 1 + len;
        rv->len = len;
        rv->s = (char *)(rv + 1);
        strcpy(rv->s, s);
    }

    return rv;
}

void empty_s(string s)
{
    s->len = 0;
    s->s[0] = 0;
}

char append_char(string *sp, char c)
{
    string s = *sp;

    if (s->len + 2 >= s->alloc)
    {
        s = realloc(s, sizeof(*s) + (sizeof(char) * s->alloc * 2));
        if (!s)
            return 0;
        s->s = (char *)(s + 1);
        s->s[s->len++] = c;
        s->alloc *= 2;
        *sp = s;
    }
    else
        s->s[s->len++] = c;
    s->s[s->len] = 0;
    return 1;
}

void trim(string s)
{
    unsigned long move_from = s->len;
    for (unsigned long i = 0; i < s->len; i++)
    {
        if (s->s[i] != ' ')
        {
            move_from = i;
            break;
        }
    }
    if (move_from == s->len)
    {
        empty_s(s);
        return;
    }

    unsigned long end = move_from;
    for (unsigned long i = s->len - 1; i >= move_from; i--)
        if (s->s[i] != ' ')
        {
            end = i;
            break;
        }

    memmove(s->s, s->s + move_from, end - move_from + 1);
    s->len = end - move_from + 1;
    s->s[s->len] = 0;
}

char string_append(string *sp, string s2)
{
    string s = *sp;

    if (s->len + s2->len + 1 >= s->alloc)
    {
        unsigned long size = s->len + s2->len + 1;
        size = size > s->alloc * 2 ? size : s->alloc * 2;

        s = realloc(s, sizeof(*s) + (sizeof(char) * size));
        if (!s)
            return 0;
        s->s = (char *)(s + 1);
        s->alloc = size;
        *sp = s;
    }
    memcpy(s->s + s->len, s2->s, s2->len + 1);
    s->len += s2->len;
    return 1;
}

char string_append_char(string *sp, char *s2)
{
    string s = *sp;
    unsigned long len = strlen(s2);

    if (s->len + len + 1 >= s->alloc)
    {
        unsigned long size = s->len + len + 1;
        size = size > s->alloc * 2 ? size : s->alloc * 2;

        s = realloc(s, sizeof(*s) + (sizeof(char) * size));
        if (!s)
            return 0;
        s->s = (char *)(s + 1);
        s->alloc = size;
        *sp = s;
    }
    memcpy(s->s + s->len, s2, len + 1);
    s->len += len;
    return 1;
}