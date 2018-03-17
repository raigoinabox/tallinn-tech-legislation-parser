#include "printing.h"

#include <cgraph.h>
#include <gvc.h>
#include <gvcext.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vectors.h"

static char* get_node_id(char* section_number)
{
    char* node_id = malloc_a(strlen(section_number) + 20, sizeof(char));
    int error = snprintf(node_id, 100, "section_%s", section_number);
    if (error < 0)
    {
        abort();
    }
    return node_id;
}

static int fprintf_a(FILE* file, const char* template, ...)
{
    va_list args;
    va_start(args, template);
    int bytes_printed = vfprintf(file, template, args);
    va_end(args);
    if (bytes_printed < 0)
    {
        perror("vprintf");
        abort();
    }
    return bytes_printed;
}

static void print_csv_format(FILE* file, struct section_vec connections)
{
    for (int32_t i = 0; i < vec_length(connections); i++)
    {
        struct section section = vec_elem(connections, i);
        for (int32_t j = 0; j < vec_length(section.references); j++)
        {
            char* reference = vec_elem(section.references, j);
            fprintf_a(file, "%s;%s\n", section.id, reference);
        }
    }
}

int printeln_a()
{
    return println_a(str_c(""));
}

int println_a(struct string string)
{
    int bytes_printed = printf("%s\n", str_content(string));
    if (bytes_printed < 0)
    {
        perror("printf");
        abort();
    }
    return bytes_printed;
}

int printf_a(const char* template, ...)
{
    va_list args;
    va_start(args, template);
    int bytes_printed = vprintf(template, args);
    va_end(args);
    if (bytes_printed < 0)
    {
        perror("vprintf");
        abort();
    }
    return bytes_printed;
}

int printf_ea(const char* template, ...)
{
    va_list args;
    va_start(args, template);
    int bytes_printed = vfprintf(stderr, template, args);
    va_end(args);
    if (bytes_printed < 0)
    {
        perror("vfprintf");
        abort();
    }
    return bytes_printed;
}

void print_graph(FILE* file, struct section_vec connections,
                         const char* format)
{
    if (strcmp(format, "csv") == 0)
    {
        print_csv_format(file, connections);
    }
    else
    {
        Agraph_t *graph = agopen("G", Agstrictdirected, NULL);
        for (int32_t i = 0; i < vec_length(connections); i++)
        {
            struct section section = vec_elem(connections, i);
            char* section_node_id = get_node_id(section.id);
            Agnode_t *node = agnode(graph, section_node_id, 1);
            free(section_node_id);
            agsafeset(node, "label", section.id, "");
            for (int32_t ref_i = 0;
                    ref_i < vec_length(section.references);
                    ref_i++)
            {
                char* ref_node_id = get_node_id(get_reference(section, ref_i));
                Agnode_t *ref_node = agnode(graph, ref_node_id, 1);
                if (ref_node == NULL)
                {
                    abort();
                }
                free(ref_node_id);
                agedge(graph, node, ref_node, NULL, 1);
            }
        }

        GVC_t* gvc = gvContext();
        gvLayout(gvc, graph, "dot");
        gvRender(gvc, graph, format, file);

        gvFreeLayout(gvc, graph);
        agclose(graph);
        gvFreeContext(gvc);
    }
}
