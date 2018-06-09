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
    double complexity1;
    double complexity2;
};

vec_struct(leg_complex_list, struct leg_complex);

struct dbu_category_complexity
{
    double complexity1_total;
    double complexity2_total;
    int32_t legislation_count;
};

MAP_DECLARE(static, const char*, struct dbu_category_complexity, cat_compl_list)
MAP_DEFINE(static, const char*, struct dbu_category_complexity, cat_compl_list)

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
    for (vec_size i = 0; i < vec_length(section_vec); i++)
    {
        struct section section = vec_elem(section_vec, i);
        for (vec_size j = 0; j < vec_length(section.references); j++)
        {
            struct string reference = vec_elem(section.references, j);
            vec_size ref_index = -1;
            for (vec_size k = 0; k < vec_length(section_vec); k++)
            {
                struct section test_section = vec_elem(section_vec, k);
                if (str_equal(reference, test_section.id))
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
    if (igraph_empty(graph, vec_length(section_vec), IGRAPH_DIRECTED) != IGRAPH_SUCCESS)
    {
        success = false;
        goto exit;
    }
    if (igraph_add_edges(graph, &edges, 0) != IGRAPH_SUCCESS)
    {
        success = false;
        goto exit;
    }

exit:
    assert(success);
    igraph_vector_destroy(&edges);
    return success;
}

static bool get_legislation_sections(struct section_vec* result, struct leg_id legislation)
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
    vec_init(leg_complex_list);
    for (int32_t i = 0; i < vec_length(law_categories); i++)
    {
        struct dbu_law_category law_category = vec_elem(law_categories, i);

        for (int32_t j = 0; j < vec_length(law_category.laws); j++)
        {
            struct leg_id leg_id = vec_elem(law_category.laws, j);
            leg_id.version_date = date;
            igraph_t graph;
            if (!get_legislation_graph(&graph, leg_id))
            {
                continue;
            }

            double avg_vertex_degree;
            if (!get_average_vertex_degree(&avg_vertex_degree, &graph)) {
                continue;
            }
            igraph_real_t avg_path_length;
            if (igraph_ecount(&graph) <= 0)
            {
                avg_path_length = 0;
            } else if (igraph_average_path_length(&graph, &avg_path_length, true, true)
                    != IGRAPH_SUCCESS)
            {
                continue;
            }
            igraph_destroy(&graph);

            for (int32_t j = 0; j < vec_length(law_category.dbu_categories);
                    j++)
            {
                const char* dbu_category = vec_elem(
                                               law_category.dbu_categories, j);
                struct leg_complex leg_complex;
                leg_complex.dbu_category = dbu_category;
                leg_complex.complexity1 = avg_vertex_degree;
                leg_complex.complexity2 = avg_path_length;
                vec_append(leg_complex_list, leg_complex);
            }
        }
    }
    return leg_complex_list;
}

static struct cat_compl_list get_category_complexities_from_leg(
    struct leg_complex_list leg_complex_list)
{
    struct cat_compl_list cat_compl_list = cat_compl_list_init(strcmp);
    for (int32_t i = 0; i < vec_length(leg_complex_list); i++)
    {
        struct leg_complex leg_complex = vec_elem(leg_complex_list,
                                         i);
        struct dbu_category_complexity complexity;

            if (cat_compl_list_get(&complexity, cat_compl_list,
                                   leg_complex.dbu_category))
        {
            complexity.legislation_count++;
            complexity.complexity1_total += leg_complex.complexity1;
            complexity.complexity2_total += leg_complex.complexity2;
        }
        else
        {
            complexity.legislation_count = 1;
            complexity.complexity1_total = leg_complex.complexity1;
            complexity.complexity2_total = leg_complex.complexity2;
        }
        cat_compl_list_set(&cat_compl_list, leg_complex.dbu_category,
                           complexity);
    }

    struct cat_compl_list_iterator iterator = cat_compl_list_iterator(
                cat_compl_list);
    while (cat_compl_list_iterator_has_next(iterator))
    {
        cat_compl_list_iterator_next(&iterator);
        struct dbu_category_complexity complexity =
            cat_compl_list_iterator_get_value(iterator);
        complexity.complexity1_total = complexity.complexity1_total
                                      / complexity.legislation_count;
        complexity.complexity2_total = complexity.complexity2_total
                                              / complexity.legislation_count;
        complexity.legislation_count = 1;
        MAP_ITERATOR_SET(&iterator, complexity);
    }

    return cat_compl_list;
}

static struct cat_compl_list get_category_complexities(
    struct law_category_list law_categories, struct string date)
{
    struct leg_complex_list leg_complex_list = get_leg_complexities(
                law_categories, date);
    struct cat_compl_list cat_compl_list = get_category_complexities_from_leg(
            leg_complex_list);
    vec_free(leg_complex_list);
    return cat_compl_list;
}

static void print_category_complexities(
        struct cat_compl_list cat_compl_list, struct string date)
{
    struct cat_compl_list_iterator iterator = cat_compl_list_iterator(
                cat_compl_list);

    while (cat_compl_list_iterator_has_next(iterator))
    {
        cat_compl_list_iterator_next(&iterator);
        struct dbu_category_complexity complexity =
                cat_compl_list_iterator_get_value(iterator);
        printf_a("%s %s %g\n", str_content(date),
                cat_compl_list_iterator_get_key(iterator),
                complexity.complexity1_total);
    }
}

static void insert_cat_cmpxs(sqlite3* db_conn, int32_t year,
                             struct cat_compl_list cat_compl_list)
{
    struct cat_compl_list_iterator iterator = cat_compl_list_iterator(
                cat_compl_list);
    begin_transaction(db_conn);
    while (cat_compl_list_iterator_has_next(iterator))
    {
        cat_compl_list_iterator_next(&iterator);
        struct dbu_category_complexity complexity =
                cat_compl_list_iterator_get_value(iterator);
        struct complexity_result_dto result;
        result.country = str_c("GB");
        result.year = year;
        result.dbu_category = str_c(cat_compl_list_iterator_get_key(iterator));
        result.complexity = complexity.complexity1_total;
        result.algorithm = 0;
        insert_result(db_conn, result);

        result.complexity = complexity.complexity2_total;
        result.algorithm = 1;
        insert_result(db_conn, result);
    }
    commit_transaction(db_conn);
}

static struct arp_option_vec get_options()
{
    struct arp_option_vec options;
    vec_init(options);
    struct arp_option option = { .short_form = 'h', .long_form = "help",
        .help_text = "Print this help message.", .argument_name = NULL
    };
    vec_append(options, option);
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
            struct cat_compl_list cat_compl_list = get_category_complexities(
                    law_categories, date);
            insert_cat_cmpxs(db_conn, i, cat_compl_list);

            print_category_complexities(cat_compl_list, date);
            MAP_FREE(cat_compl_list);
            str_free(&date);
        }
        db_close_conn(db_conn);
    }

    vec_free(options);
    return true;
}
