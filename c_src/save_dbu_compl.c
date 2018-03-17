/*
 * comp_dbu.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "save_dbu_compl.h"

#include <assert.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    int32_t complexity;
};

vec_struct(leg_complex_list, struct leg_complex);

MAP_DECLARE(, const char*, int32_t, cat_comps)
MAP_DEFINE(, const char*, int32_t, cat_comps)

struct dbu_category_complexity
{
    int32_t complexity_total;
    int32_t legislation_count;
};

MAP_DECLARE(static, const char*, struct dbu_category_complexity, cat_compl_list)
MAP_DEFINE(static, const char*, struct dbu_category_complexity, cat_compl_list)

static bool get_legislation_complexity(int32_t* result,
                                       struct leg_id legislation)
{
    assert(result != NULL);

    struct section_vec sections;
    if (!get_sections_from_legislation(&sections, legislation))
    {
//		fprintf_a(stderr, "get_sections_from_legislation failed. \n");
        return false;
    }
    remove_foreign_sections(sections);

    int32_t sections_count = vec_length(sections);
    if (sections_count == 0)
    {
        sections_free_deep(&sections);
        return false;
    }
    int32_t references_total = 0;
    for (int32_t i = 0; i < sections_count; i++)
    {
        struct section section = vec_elem(sections, i);
        references_total += get_references_count(section);
    }
    sections_free_deep(&sections);

    *result = (references_total * 100) / sections_count;
    return true;
}

static struct leg_complex_list get_norm_leg_complexities(
    struct law_category_list law_categories, struct string date)
{
    struct leg_complex_list leg_complex_list;
    vec_init(leg_complex_list);
    for (int32_t i = 0; i < vec_length(law_categories); i++)
    {
        struct dbu_law_category law_category = vec_elem(
                law_categories, i);

        for (int32_t j = 0; j < vec_length(law_category.laws); j++)
        {
            struct leg_id leg_id = vec_elem(law_category.laws, j);
            leg_id.version_date = date;
            int32_t complexity;
            bool success = get_legislation_complexity(&complexity, leg_id);
            if (success)
            {
                for (int32_t j = 0;
                        j < vec_length(law_category.dbu_categories);
                        j++)
                {
                    const char* dbu_category = vec_elem(
                                                   law_category.dbu_categories, j);
                    struct leg_complex leg_complex = { .dbu_category =
                            dbu_category, .complexity = complexity
                    };
                    vec_append(leg_complex_list, leg_complex);
                }
            }
        }
    }

    int32_t max_complex = INT32_MIN;
    int32_t min_complex = INT32_MAX;
    for (int32_t i = 0; i < vec_length(leg_complex_list); i++)
    {
        struct leg_complex leg_complex = vec_elem(leg_complex_list,
                                         i);
        if (leg_complex.complexity < min_complex)
        {
            min_complex = leg_complex.complexity;
        }
        if (max_complex < leg_complex.complexity)
        {
            max_complex = leg_complex.complexity;
        }
    }

    for (int32_t i = 0; i < vec_length(leg_complex_list); i++)
    {
        struct leg_complex leg_complex = vec_elem(leg_complex_list,
                                         i);
        leg_complex.complexity = ((leg_complex.complexity - min_complex) * 100)
                                 / (max_complex - min_complex);
        vec_set(leg_complex_list, i, leg_complex);
    }

    return leg_complex_list;
}

static struct cat_compl_list get_norm_category_complexities(
    const struct law_category_list law_categories, struct string date)
{
    struct leg_complex_list leg_complex_list = get_norm_leg_complexities(
                law_categories, date);
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
            complexity.complexity_total += leg_complex.complexity;
        }
        else
        {
            complexity.legislation_count = 1;
            complexity.complexity_total = leg_complex.complexity;
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
        complexity.complexity_total = complexity.complexity_total
                                      / complexity.legislation_count;
        complexity.legislation_count = 1;
        MAP_ITERATOR_SET(&iterator, complexity);
    }

    vec_free(leg_complex_list);

    return cat_compl_list;
}

static void print_norm_category_complexities(
    struct law_category_list law_categories, struct string date)
{
    struct cat_compl_list cat_compl_list = get_norm_category_complexities(
            law_categories, date);

    struct cat_compl_list_iterator iterator = cat_compl_list_iterator(
                cat_compl_list);
    while (cat_compl_list_iterator_has_next(iterator))
    {
        cat_compl_list_iterator_next(&iterator);
        printf_a("%s %s %d\n", str_content(date),
                 cat_compl_list_iterator_get_key(iterator),
                 cat_compl_list_iterator_get_value(iterator).complexity_total);
    }

    cat_compl_list_free(&cat_compl_list);
}

static void insert_cat_cmpxs(sqlite3* db_conn, int32_t year,
                             struct cat_compl_list cat_compl_list)
{
    struct cat_compl_list_iterator iterator = cat_compl_list_iterator(
                cat_compl_list);
    while (cat_compl_list_iterator_has_next(iterator))
    {
        cat_compl_list_iterator_next(&iterator);
        struct complexity_result_dto result =
        {
            .country = str_c("GB"), .year = year, .dbu_category =
            str_c(cat_compl_list_iterator_get_key(iterator)),
            .complexity = cat_compl_list_iterator_get_value(
                iterator).complexity_total
        };
        insert_result(db_conn, result);
    }
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
            struct cat_compl_list cat_compl_list = get_norm_category_complexities(
                    law_categories, date);
            insert_cat_cmpxs(db_conn, i, cat_compl_list);

            print_norm_category_complexities(law_categories, date);
            cat_compl_list_free(&cat_compl_list);
            str_free(&date);
        }
        db_close_conn(db_conn);
    }

    vec_free(options);
    return true;
}
