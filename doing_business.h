#pragma once

#include <stdint.h>

#include "legislation.h"
#include "vectors.h"

VECTOR_DECLARE(, struct leg_id, law_list)
VECTOR_DECLARE(, const char*, string_list)

struct dbu_law_category {
	const char* name;
	struct string_list dbu_categories;
	struct law_list laws;
};

VECTOR_DECLARE(, struct dbu_law_category, law_category_list)

struct dbu_year {
	int_fast32_t year;
	double global_dtf;
	double starting_a_business;
	double dealing_with_construction_permits;
	double getting_electricity;
	double registering_property;
	double getting_credit;
	double protecting_minority_investors;
	double paying_taxes;
	double trading_across_borders;
	double enforcing_contracts;
	double resolving_insolvency;
};

static const struct dbu_year doing_business_data[] = {
		{ 2018, 82.22, 94.58, 80.39, 93.29, 74.51, 75, 75, 86.7, 93.76, 68.69,
				80.24 },
		{ 2017, 82.34, 94.58, 80.34, 93.29, 74.11, 75, 75, 86.63, 93.76,
				68.69, 82.04 },
		{ 2016, 82.34, 94.57, 80.29, 93.29, 74.5, 75, 75, 86.25, 93.76,
				68.69, 82.04 },
		{ 2015, 82.57, 91.23, 80.16, 93.28, 74.41, 75, 75, 90.62, 93.76,
				70.19, 82.04 },
		{ 2014, 81.77, 89.85, 86.85, 90.78, 72.43, 75, 75, 90.08, 88.24,
				67.41, 82.04 },
		{ 2013, 85.03, 89.8, 86.96, 81.41, 71.32, 100, 80, 89.9, 88.16,
				67.41, 95.33 },
		{ 2012, 84.99, 89.8, 86.92, 80.82, 71.28, 100, 80, 89.36, 88,
				68.45, 95.33 },
		{ 2011, 84.88, 89.79, 85.79, 81.09, 71.23, 100, 80, 89.38,
				87.78, 68.45, 95.33 },
		{ 2010, 84.09, 90.04, 85.84, 81.1, 71.27, 100, 80, 89.96, 85.68,
				66.36, 90.66 },
		{ 2009, -1, 90.04, 70.54, -1, 69.12, 100, 80, 90.54, 84.78,
				66.23, 90.66 },
		{ 2008, -1, 90.04, 70.53, -1, 69.06, 100, 80, 90.38, 85.24,
				66.23, 91.02 },
		{ 2007, -1, 90.05, 70.43, -1, 68.95, 100, 80, 90.35, 84.99,
				66.23, 91.68 },
		{ 2006, -1, 89.67, 70.4, -1, 68.92, 100, 80, 90.32, 84.82,
				66.94, 91.77 },
		{ 2005, -1, 89.39, -1, -1, 68.8, 100, -1, -1, -1, 68.44, 92.39 },
		{ 2004, -1, 89.38, -1, -1, -1, -1, -1, -1, -1, 70.31, 92.12 },
		{ 0 }
};

void dbu_init();
struct law_category_list get_law_categories();
