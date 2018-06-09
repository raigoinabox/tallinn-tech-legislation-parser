#include "sections.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"

static bool is_section_connected(struct section_vec sections,
                                 struct section section)
{
    if (has_section_references(section))
    {
        return 1;
    }

    for (int32_t section_idx = 0; section_idx < vec_length(sections);
            section_idx++)
    {
        struct section test_section = vec_elem(sections, section_idx);
        for (int32_t ref_idx = 0; ref_idx < get_references_count(test_section);
                ref_idx++)
        {
            if (str_equal(get_reference(test_section, ref_idx), section.id))
            {
                return 1;
            }
        }
    }

    return 0;
}

static void free_section(struct section* section_p)
{
    struct section section = *section_p;

    str_free(&section.id);
    free_references_deep(&section.references);

    *section_p = section;
}

void sections_free_deep(struct section_vec* array_p)
{
    struct section_vec array = *array_p;

    for (int32_t i = 0; i < vec_length(array); i++)
    {
        struct section section = vec_elem(array, i);
        free_section(&section);
    }

    vec_free(array);

    *array_p = array;
}

int32_t get_references_count(struct section section)
{
    return vec_length(section.references);
}

struct string get_reference(struct section section, int32_t index)
{
    return vec_elem(section.references, index);
}

_Bool has_section_references(struct section section)
{
    return vec_length(section.references) > 0;
}

void remove_foreign_sections(struct section_vec sections, bool abort_when_found)
{
    for (int32_t section_idx = 0; section_idx < vec_length(sections);
            section_idx++)
    {
        struct section section = vec_elem(sections, section_idx);
        for (int32_t ref_i = 0; ref_i < get_references_count(section); ref_i++)
        {
            struct string reference = get_reference(section, ref_i);

            bool ref_found = 0;
            for (int32_t i = 0; i < vec_length(sections); i++)
            {
                if (str_equal(vec_elem(sections, i).id, reference) == 0)
                {
                    ref_found = 1;
                    break;
                }
            }
            if (!ref_found)
            {
                assert(!abort_when_found);
                vec_remove(section.references, ref_i);
                ref_i--;
                str_free(&reference);
            }
        }
        vec_set(sections, section_idx, section);
    }
}

void remove_single_sections(struct section_vec* sections_p)
{
    struct section_vec sections = *sections_p;
    for (int32_t test_section_idx = 0;
            test_section_idx < vec_length(sections); test_section_idx++)
    {
        if (!is_section_connected(sections, vec_elem(sections, test_section_idx)))
        {
            struct section section = vec_elem(sections, test_section_idx);
            free_section(&section);
            vec_remove(sections, test_section_idx);
            test_section_idx--;
        }
    }
    *sections_p = sections;
}
