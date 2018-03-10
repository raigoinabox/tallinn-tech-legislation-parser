#include <cgraph.h>
#include <gvc.h>
#include <gvcext.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sections.h"
#include "text_parser.h"
#include "util.h"

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

void print_from_sections(FILE* file, struct sections connections,
                         const char* format)
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
