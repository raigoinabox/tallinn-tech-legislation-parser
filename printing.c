#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <gvc.h>

#include "sections.h"
#include "util.h"

// static void print_dot_from_connections(struct sections connections)
// {
// 	printf("strict digraph G {\n");
// 	for (int32_t i = 0; i < connections.elem_count; i++) {
// 		struct section section = connections.list[i];
// 		printf("\tsection_%s [label=\"%s\"];\n", section.id, section.id);
// 	}
// 	printf("\n");
// 	for (int32_t i = 0; i < connections.elem_count; i++) {
// 		struct section section = connections.list[i];
// 		for (int32_t ref_i = 0; ref_i < section.references.elem_count; ref_i++) {
// 			printf("\tsection_%s -> section_%s;\n", section.id, get_reference(section, ref_i));
// 		}
// 	}
// 	printf("}\n");
// }


static char* get_node_id(char* section_number)
{
	char* node_id = safe_malloc(strlen(section_number) + 20, sizeof(char));
	int error = snprintf(node_id, 100, "section_%s", section_number);
	if (error < 0)
	{
		abort();
	}
	return node_id;
}

static void print_from_sections(struct sections connections, const char* format)
{
	Agraph_t *graph = agopen("G", Agstrictdirected, NULL);
	for (int32_t i = 0; i < connections.elem_count; i++)
	{
		struct section section = connections.list[i];
		char* section_node_id = get_node_id(section.id);
		Agnode_t *node = agnode(graph, section_node_id, 1);
		free(section_node_id);
		agsafeset(node, "label", section.id, "");
		for (int32_t ref_i = 0; ref_i < section.references.elem_count; ref_i++)
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
	gvRender(gvc, graph, format, stdout);

	gvFreeLayout(gvc, graph);
	agclose(graph);
	gvFreeContext(gvc);
}

void print_pdf_from_sections(struct sections connections)
{
	print_from_sections(connections, "pdf");
}

void print_dot_from_sections(struct sections connections)
{
	print_from_sections(connections, "canon");
}
