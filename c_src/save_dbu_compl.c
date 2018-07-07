/*
 * comp_dbu.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "save_dbu_compl.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <igraph.h>
#include <sqlite3.h>

#include "command_line.h"
#include "database.h"
#include "doing_business.h"
#include "legislation.h"
#include "maps.h"
#include "misc.h"
#include "printing.h"
#include "results_dao.h"
#include "sections.h"
#include "strings.h"
#include "vectors.h"

struct leg_complex
{
    struct leg_id legislation;
    const char* dbu_category;
    // double
    struct vector complexities;
};

vec_struct(leg_complex_list, struct leg_complex);

struct dbu_category_complexity
{
    int32_t legislation_count;
    // double
    struct vector complexity_totals;
};

static struct leg_complex init_leg_complex(struct leg_id legislation,
        const char* dbu_category)
{
    struct leg_complex result;
    result.legislation = legislation;
    result.dbu_category = dbu_category;
    vec_init(&result.complexities, sizeof(double));
    return result;
}

static void destroy_leg_complex(struct leg_complex* leg_complex)
{
    vec_destroy(&leg_complex->complexities);
}

static void destroy_leg_complex_list(struct leg_complex_list* vector)
{
    for (int i = 0; i < vec_length_old(*vector); i++)
    {
        struct leg_complex leg_complex = vec_elem_old(*vector, i);
        destroy_leg_complex(&leg_complex);
    }
    vec_free(*vector);
}

static struct dbu_category_complexity init_dbu_cat_complexity()
{
    struct dbu_category_complexity dbu_complexity;
    dbu_complexity.legislation_count = 1;
    vec_init(&dbu_complexity.complexity_totals, sizeof(double));
    return dbu_complexity;
}

static void destroy_dbu_cat_complexity(struct dbu_category_complexity*
                                       complexity)
{
    //but why destroy cat??
    vec_destroy(&complexity->complexity_totals);
}

static void destroy_cat_complex_list(struct map* cat_compl_list)
{
    struct map_iterator iter = map_iterator(*cat_compl_list);
    struct map_entry entry;
    while (map_iterator_next(&entry, &iter))
    {
        struct dbu_category_complexity* complex = entry.value;
        destroy_dbu_cat_complexity(complex);
    }
    map_free(cat_compl_list);
}

static bool get_average_vertex_degree(double* result, const igraph_t* graph)
{
    igraph_integer_t vertex_count = igraph_vcount(graph);
    if (vertex_count <= 0)
    {
        return false;
    }

    double edge_count = igraph_ecount(graph);
    *result = edge_count / vertex_count;
    return true;
}

static bool get_sections_graph(igraph_t* graph, struct section_vec section_vec)
{
    bool success = true;
    igraph_vector_t edges = { 0 };
    if (igraph_vector_init(&edges, 0) != IGRAPH_SUCCESS)
    {
        success = false;
        goto exit;
    }
    for (vec_size i = 0; i < vec_length_old(section_vec); i++)
    {
        struct section section = vec_elem_old(section_vec, i);
        for (vec_size j = 0; j < vec_length_old(section.references); j++)
        {
            struct string reference = vec_elem_old(section.references, j);
            vec_size ref_index = -1;
            for (vec_size k = 0; k < vec_length_old(section_vec); k++)
            {
                struct section test_section = vec_elem_old(section_vec, k);
                if (str_is_equal(reference, test_section.id))
                {
                    ref_index = k;
                    break;
                }
            }
            if (ref_index >= 0)
            {
                if (igraph_vector_push_back(&edges, i) != IGRAPH_SUCCESS)
                {
                    success = false;
                    goto exit;
                }
                if (igraph_vector_push_back(&edges, ref_index) != IGRAPH_SUCCESS)
                {
                    success = false;
                    goto exit;
                }
            }
        }
    }
    if (igraph_empty(graph, vec_length_old(section_vec),
                     IGRAPH_DIRECTED) != IGRAPH_SUCCESS)
    {
        success = false;
        goto exit;
    }
    if (igraph_add_edges(graph, &edges, 0) != IGRAPH_SUCCESS)
    {
        igraph_destroy(graph);
        success = false;
        goto exit;
    }

exit:
    assert(success);
    igraph_vector_destroy(&edges);
    return success;
}

static bool get_legislation_sections(struct section_vec* result,
                                     struct leg_id legislation)
{
    if (!get_sections_from_legislation(result, legislation))
    {
        return false;
    }
    remove_foreign_sections(*result, false);
    return true;
}

static bool get_legislation_graph(igraph_t* graph, struct leg_id legislation)
{
    struct section_vec sections;
    if (!get_legislation_sections(&sections, legislation))
    {
        return false;
    }
    if (!get_sections_graph(graph, sections))
    {
        sections_free_deep(&sections);
        return false;
    }

    sections_free_deep(&sections);
    return true;
}

static struct leg_complex_list get_leg_complexities(
    struct law_category_list law_categories, struct string date)
{
    struct leg_complex_list leg_complex_list;
    vec_init_old(leg_complex_list);
    for (int32_t i = 0; i < vec_length_old(law_categories); i++)
    {
        struct dbu_law_category law_category = vec_elem_old(law_categories, i);

        for (int32_t j = 0; j < vec_length_old(law_category.laws); j++)
        {
            struct leg_id leg_id = vec_elem_old(law_category.laws, j);
            leg_id.version_date = date;
            igraph_t graph = { 0 };
            if (!get_legislation_graph(&graph, leg_id))
            {
                continue;
            }

            double avg_vertex_degree;
            if (!get_average_vertex_degree(&avg_vertex_degree, &graph))
            {
                goto loop_end;
            }
            double avg_path_length;
            if (igraph_ecount(&graph) <= 0)
            {
                avg_path_length = 0;
            }
            else if (igraph_average_path_length(&graph, &avg_path_length, true, true)
                     != IGRAPH_SUCCESS)
            {
                goto loop_end;
            }
            igraph_integer_t graph_diameter_i;
            if (igraph_diameter(&graph, &graph_diameter_i, NULL, NULL, NULL, true, true)
                    != IGRAPH_SUCCESS)
            {
                goto loop_end;
            }
            double graph_diameter = graph_diameter_i;
            double transitivity;
            if (igraph_transitivity_undirected(
                    &graph,
                    &transitivity,
                    IGRAPH_TRANSITIVITY_ZERO) != IGRAPH_SUCCESS) {
                goto loop_end;
            }

            for (int32_t j = 0; j < vec_length_old(law_category.dbu_categories);
                    j++)
            {
                const char* dbu_category = vec_elem_old(
                                               law_category.dbu_categories, j);
                struct leg_complex leg_complex = init_leg_complex(leg_id, dbu_category);
                vec_append(&leg_complex.complexities, &avg_vertex_degree);
                vec_append(&leg_complex.complexities, &avg_path_length);
                vec_append(&leg_complex.complexities, &graph_diameter);
                vec_append(&leg_complex.complexities, &transitivity);
                vec_append_old(leg_complex_list, leg_complex);
            }
loop_end:
            igraph_destroy(&graph);
        }
    }
    return leg_complex_list;
}

static int void_strcmp(void* void1, void* void2)
{
    const char** string1 = void1;
    const char** string2 = void2;
    return strcmp(*string1, *string2);
}

// return map(const char*, struct dbu_category_complexity)
static struct map get_category_complexities_from_leg(
    struct leg_complex_list leg_complex_list)
{
    struct map cat_compl_list = map_init(sizeof(const char*),
                                         sizeof(struct dbu_category_complexity), void_strcmp);
    for (int32_t i = 0; i < vec_length_old(leg_complex_list); i++)
    {
        struct leg_complex leg_complex = vec_elem_old(leg_complex_list,
                                         i);
        struct dbu_category_complexity dbu_complexity;
        if (map_get(&dbu_complexity, cat_compl_list,
                    &leg_complex.dbu_category))
        {
            dbu_complexity.legislation_count++;
            for (int i = 0; i < vec_length(leg_complex.complexities); i++)
            {
                double* complexity = vec_elem(leg_complex.complexities, i);
                double* complex_total = vec_elem(dbu_complexity.complexity_totals, i);
                *complex_total += *complexity;
            }
        }
        else
        {
            dbu_complexity = init_dbu_cat_complexity();
            for (int i = 0; i < vec_length(leg_complex.complexities); i++)
            {
                double* complexity = vec_elem(leg_complex.complexities, i);
                vec_append(&dbu_complexity.complexity_totals, complexity);
            }
        }
        map_set(&cat_compl_list, &leg_complex.dbu_category, &dbu_complexity);
    }

    struct map_iterator iterator = map_iterator(cat_compl_list);
    struct map_entry entry;
    while (map_iterator_next(&entry, &iterator))
    {
        struct dbu_category_complexity* complexity_p = entry.value;
        struct dbu_category_complexity complexity = *complexity_p;
        for (int i = 0; i < vec_length(complexity.complexity_totals); i++)
        {
            double* total_complex = vec_elem(complexity.complexity_totals, i);
            *total_complex /= complexity.legislation_count;
        }
        complexity.legislation_count = 1;
        *complexity_p = complexity;
    }

    return cat_compl_list;
}

// return map(const char*, struct dbu_category_complexity)
static struct map get_category_complexities(
    struct law_category_list law_categories, struct string date)
{
    struct leg_complex_list leg_complex_list = get_leg_complexities(
                law_categories, date);
    struct map cat_compl_list = get_category_complexities_from_leg(
                                    leg_complex_list);
    destroy_leg_complex_list(&leg_complex_list);
    return cat_compl_list;
}

// takes in map(const char*, struct dbu_category_complexity)
static void print_category_complexities(
    struct map cat_compl_list, struct string date)
{
    struct map_iterator iterator = map_iterator(cat_compl_list);
    struct map_entry entry;
    while (map_iterator_next(&entry, &iterator))
    {
        const char** key_p = entry.key;
        printf_a("%s %s", str_content(date), *key_p);
        struct dbu_category_complexity* complexity_p = entry.value;
        struct dbu_category_complexity complexity = *complexity_p;
        for (int i = 0; i < vec_length(complexity.complexity_totals); i++)
        {
            double* complex_total = vec_elem(complexity.complexity_totals, i);
            printf(" %g", *complex_total);
        }
        printf("\n");
    }
}

// takes in map(*, struct dbu_category_complexity)
static void insert_cat_cmpxs(sqlite3* db_conn, int32_t year,
                             struct map cat_compl_list)
{
    struct map_iterator iterator = map_iterator(cat_compl_list);
    begin_transaction(db_conn);
    struct map_entry entry;
    while (map_iterator_next(&entry, &iterator))
    {
        struct dbu_category_complexity* complexity = entry.value;
        for (int i = 0; i < vec_length(complexity->complexity_totals); i++)
        {
            double* complex_total = vec_elem(complexity->complexity_totals, i);
            const char** dbu_category = entry.key;
            struct complexity_result_dto result;
            result.country = str_c("GB");
            result.year = year;
            result.dbu_category = str_c(*dbu_category);
            result.complexity = *complex_total;
            result.algorithm = i + 1;
            insert_result(db_conn, result);
        }
    }
    commit_transaction(db_conn);
}

static struct arp_option_vec get_options()
{
    struct arp_option_vec options;
    vec_init_old(options);
    struct arp_option option = { .short_form = 'h', .long_form = "help",
        .help_text = "Print this help message.", .argument_name = NULL
    };
    vec_append_old(options, option);
    return options;
}

bool save_dbu_compl(const char* prog, const char* command,
                    struct arp_parser parser)
{

    struct arp_option_vec options = get_options();
    parser = arp_get_parser_from_parser(parser, options);

    bool print_help = false;
    bool is_success = arp_next(&parser);
    while (is_success && arp_has(parser))
    {
        if (arp_has_option(parser))
        {
            switch (arp_get_option_key(parser))
            {
            case 'h':
                print_help = true;
                break;
            default:
                return false;
            }
        }
        else
        {
            printf_ea("%s: this command takes no arguments.\n", prog);
            return false;
        }

        is_success = arp_next(&parser);
    }
    if (!is_success)
    {
        vec_free(options);
        return false;
    }

    if (print_help)
    {
        if (!col_print_command_help(prog, command, options, ""))
        {
            vec_free(options);
            return false;
        }
    }
    else
    {
        dbu_init();

        sqlite3* db_conn = db_open_conn();
        delete_results(db_conn);
        for (int i = 2004; i <= 2018; i++)
        {
            struct string date = str_init();
            str_appendf(&date, "%d-%02d-%02d", i, 1, 1);

            const struct law_category_list law_categories =
                get_english_law_categories();
            struct map cat_compl_list = get_category_complexities(
                                            law_categories, date);
            insert_cat_cmpxs(db_conn, i, cat_compl_list);

            print_category_complexities(cat_compl_list, date);
            destroy_cat_complex_list(&cat_compl_list);
            str_free(&date);
        }
        db_close_conn(db_conn);
    }

    vec_free(options);
    return true;
}
