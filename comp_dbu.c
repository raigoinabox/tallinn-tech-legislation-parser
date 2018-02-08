/*
 * comp_dbu.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "comp_dbu.h"

#include <assert.h>
#include <string.h>

#include "doing_business.h"
#include "legislation.h"
#include "maps.h"
#include "safe_string.h"
#include "sections.h"
#include "util.h"
#include "vectors.h"
#include "misc.h"

struct leg_complex {
	struct leg_id legislation;
	const char* dbu_category;
	int32_t complexity;
};

VECTOR_DECLARE(static, struct leg_complex, leg_complex_list)
VECTOR_DEFINE(static, struct leg_complex, leg_complex_list)

MAP_DECLARE(, const char*, int32_t, cat_comps)
MAP_DEFINE(, const char*, int32_t, cat_comps)

struct dbu_category_complexity {
	int32_t complexity_total;
	int32_t legislation_count;
};

MAP_DECLARE(static, const char*, struct dbu_category_complexity, cat_compl_list)
MAP_DEFINE(static, const char*, struct dbu_category_complexity, cat_compl_list)

static bool get_legislation_complexity(int32_t* result, struct leg_id legislation)
{
	assert(result != NULL);

	struct sections sections;
	if (!get_sections_from_legislation(&sections, legislation))
	{
		fprintf_a(stderr, "get_sections_from_legislation failed. \n");
		return false;
	}
	remove_foreign_sections(sections);

	int32_t sections_count = sections_length(sections);
	if (sections_count == 0)  {
		sections_free_deep(&sections);
		return false;
	}
	int32_t references_total = 0;
	for (int i = 0; i < sections_count; i++)
	{
		struct section section = sections_get(sections, i);
		references_total += get_references_count(section);
	}
	sections_free_deep(&sections);

	*result = (references_total * 100) / sections_count;
	return true;
}

static struct leg_complex_list get_norm_leg_complexities(
		struct law_category_list law_categories)
{
	struct leg_complex_list leg_complex_list = leg_complex_list_init();
	for (int i = 0; i < law_category_list_length(law_categories); i++)
			{
		struct dbu_law_category law_category = law_category_list_get(
				law_categories, i);

		for (int j = 0; j < law_list_length(law_category.laws); j++)
				{
			struct leg_id leg_id = law_list_get(law_category.laws, j);
			int32_t complexity;
			bool success = get_legislation_complexity(&complexity, leg_id);
			if (success) {
				for (int j = 0;
						j < string_list_length(law_category.dbu_categories);
						j++)
						{
					const char* dbu_category = string_list_get(
							law_category.dbu_categories, j);
					struct leg_complex leg_complex = {
							.legislation = leg_id,
							.dbu_category = dbu_category,
							.complexity = complexity
					};
					leg_complex_list_append(&leg_complex_list, leg_complex);
				}
			}
		}
	}

	int32_t max_complex = INT32_MIN;
	int32_t min_complex = INT32_MAX;
	for (int i = 0; i < leg_complex_list_length(leg_complex_list); i++)
			{
		struct leg_complex leg_complex = leg_complex_list_get(leg_complex_list,
				i);
		if (leg_complex.complexity < min_complex) {
			min_complex = leg_complex.complexity;
		}
		if (max_complex < leg_complex.complexity) {
			max_complex = leg_complex.complexity;
		}
	}

	for (int i = 0; i < leg_complex_list_length(leg_complex_list); i++)
			{
		struct leg_complex leg_complex = leg_complex_list_get(leg_complex_list,
				i);
		leg_complex.complexity = ((leg_complex.complexity - min_complex) * 100)
				/ (max_complex - min_complex);
		leg_complex_list_set(&leg_complex_list, i, leg_complex);
	}

	return leg_complex_list;
}

bool comp_dbu(int argc, const char* argv[], int32_t offset) {
	(void) argc;
	(void) argv;
	(void) offset;

	dbu_init();
	const struct law_category_list law_categories = get_law_categories();
	struct leg_complex_list leg_complex_list = get_norm_leg_complexities(
			law_categories);
	struct cat_compl_list cat_compl_list = cat_compl_list_init(strcmp);
	for (int i = 0; i < leg_complex_list_length(leg_complex_list); i++)
	{
		struct leg_complex leg_complex = leg_complex_list_get(leg_complex_list, i);
		struct dbu_category_complexity complexity;
		if (cat_compl_list_get(&complexity, cat_compl_list, leg_complex.dbu_category))
		{
			complexity.legislation_count++;
			complexity.complexity_total += leg_complex.complexity;
		} else {
			complexity.legislation_count = 1;
			complexity.complexity_total = leg_complex.complexity;
		}
		cat_compl_list_set(&cat_compl_list, leg_complex.dbu_category, complexity);
	}

	struct cat_compl_list_iterator iterator = cat_compl_list_iterator(cat_compl_list);
	while (cat_compl_list_iterator_has_next(iterator)) {
		cat_compl_list_iterator_next(&iterator);
		struct dbu_category_complexity complexity =
				cat_compl_list_iterator_get_value(iterator);
		complexity.complexity_total = complexity.complexity_total
				/ complexity.legislation_count;
		complexity.legislation_count = 1;
		MAP_ITERATOR_SET(&iterator, complexity);
	}

	iterator = cat_compl_list_iterator(cat_compl_list);
	while (cat_compl_list_iterator_has_next(iterator)) {
		cat_compl_list_iterator_next(&iterator);
		printf_a("Hello %s %d %d\n", cat_compl_list_iterator_get_key(iterator),
				cat_compl_list_iterator_get_value(iterator).complexity_total,
				cat_compl_list_iterator_get_value(iterator).legislation_count);
	}

	leg_complex_list_free(&leg_complex_list);
	cat_compl_list_free(&cat_compl_list);

	return false;
}
