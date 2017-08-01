#include "sections.h"

#include <stdint.h>
#include <stdlib.h>

#include <string.h>

static _Bool is_section_connected(struct sections sections,
                                  struct section section)
{
	if (has_section_references(section))
	{
		return 1;
	}

	for (int32_t section_idx = 0; section_idx < get_sections_count(sections);
	        section_idx++)
	{
		struct section test_section = get_section(sections, section_idx);
		for (int32_t ref_idx = 0; ref_idx < get_references_count(test_section);
		        ref_idx++)
		{
			if (strcmp(get_reference(test_section, ref_idx), section.id) == 0)
			{
				return 1;
			}
		}
	}

	return 0;
}

struct sections init_section_darray()
{
	struct sections result =
	{
		.list = malloc(1 * sizeof(struct section)),
		.list_size = 1,
		.elem_count = 0
	};
	if (result.list == NULL)
	{
		abort();
	}

	return result;
}

void free_section(struct section* section_p)
{
	struct section section = *section_p;

	free(section.id);
	section.id = NULL;
	free_references(&section.references);

	*section_p = section;
}

void free_sections(struct sections* array_p)
{
	struct sections array = *array_p;

	for (int32_t i = 0; i < get_sections_count(array); i++)
	{
		struct section section = get_section(array, i);
		free_section(&section);
	}

	free_sections_shallow(&array);

	*array_p = array;
}

void free_sections_shallow(struct sections* sections_p)
{
	struct sections array = *sections_p;

	free(array.list);
	array.list = NULL;
	array.list_size = 0;
	array.elem_count = 0;

	*sections_p = array;
}

void section_da_add_elem(struct sections* array_p, struct section element)
{
	struct sections array = *array_p;

	if (array.list_size <= array.elem_count)
	{
		array.list_size *= 2;
		array.list = realloc(array.list, array.list_size * sizeof(struct section));
		if (array.list == NULL)
		{
			abort();
		}
	}
	array.list[array.elem_count] = element;
	array.elem_count++;

	*array_p = array;
}

void remove_section(struct sections* array_p, int32_t index)
{
	struct sections array = *array_p;
	for (; index < array.elem_count - 1; index++)
	{
		array.list[index] = array.list[index + 1];
	}
	array.elem_count--;
	*array_p = array;
}

void set_section(struct sections sections, int32_t index,
                 struct section section)
{
	sections.list[index] = section;
}

int32_t get_sections_count(struct sections sections)
{
	return sections.elem_count;
}

struct section get_section(struct sections sections, int32_t index)
{
	return sections.list[index];
}

int32_t get_references_count(struct section section)
{
	return section.references.elem_count;
}

char* get_reference(struct section section, int32_t index)
{
	return section.references.list[index];
}

_Bool has_section_references(struct section section)
{
	return section.references.elem_count > 0;
}

void remove_foreign_sections(struct sections sections)
{
	for (int32_t section_idx = 0; section_idx < sections.elem_count; section_idx++)
	{
		struct section section = sections.list[section_idx];
		for (int32_t ref_i = 0; ref_i < section.references.elem_count; ref_i++)
		{
			char const* const reference = section.references.list[ref_i];

			_Bool ref_found = 0;
			for (int32_t i = 0; i < sections.elem_count; i++)
			{
				if (strcmp(sections.list[i].id, reference) == 0)
				{
					ref_found = 1;
					break;
				}
			}
			if (!ref_found)
			{
				char* reference = get_reference(section, ref_i);
				remove_reference(&section.references, ref_i);
				ref_i--;
				free(reference);
			}
		}
		set_section(sections, section_idx, section);
	}
}

void remove_single_sections(struct sections* sections_p)
{
	struct sections sections = *sections_p;
	for (int32_t test_section_idx = 0;
	        test_section_idx < get_sections_count(sections); test_section_idx++)
	{
		if (!is_section_connected(sections, get_section(sections, test_section_idx)))
		{
			struct section section = get_section(sections, test_section_idx);
			free_section(&section);
			remove_section(&sections, test_section_idx);
			test_section_idx--;
		}
	}
	*sections_p = sections;
}
