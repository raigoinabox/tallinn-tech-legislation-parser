#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include <libxml/xmlstring.h>
#include <pcre.h>

#include "text_parser.h"

static bool
str_is_prefix_ci(const char* text, const char* prefix)
{
	while (*prefix != 0)
	{
		if (*text == 0 || tolower(*text) != tolower(*prefix))
		{
			return false;
		}

		text++;
		prefix++;
	}

	return true;
}

static const char* parse_prefix(const char* char_p)
{
	const char prefix[] = "section";
	if (!str_is_prefix_ci(char_p, prefix))
	{
		return NULL;
	}
	char_p += strlen(prefix);

	if (tolower(*char_p) == 's')
	{
		char_p++;
	}

	return char_p;
}

static const char* parse_spaces(const char* char_p)
{
	if (*char_p != ' ')
	{
		return NULL;
	}
	while (*char_p == ' ')
	{
		char_p++;
	}

	return char_p;
}

// return value:
// 0 means no error
// 1 means the section reference was missing
// 2 means that the text is something the parser doesn't recognize
static int32_t parse_section_reference(char** result_p, const char** char_pp)
{
	const char* char_p = *char_pp;
	bool reference_exists;
	char* result = NULL;
	if (isdigit(*char_p))
	{
		reference_exists = true;

		char ref_buffer[10];
		int32_t ref_buf_i = 0;
		while (isdigit(*char_p))
		{
			if (ref_buf_i >= 10 - 1)
			{
				abort();
			}
			ref_buffer[ref_buf_i] = *char_p;

			ref_buf_i++;
			char_p++;
		}
		while (*char_p >= 'A' && *char_p <= 'Z')
		{
			assert(ref_buf_i < 10 - 1);
			ref_buffer[ref_buf_i] = *char_p;

			ref_buf_i++;
			char_p++;
		}
		ref_buffer[ref_buf_i] = 0;

		result = malloc((ref_buf_i + 2) * sizeof(char const));
		strcpy(result, ref_buffer);
	}
	else if (*char_p == '(')
	{
		reference_exists = false;
	}
	else
	{
		goto error;
	}

	while (*char_p == '(')
	{
		char_p++;
		if (isdigit(*char_p))
		{
			while (isdigit(*char_p))
			{
				char_p++;
			}
			int32_t failsafe = 0;
			while (*char_p >= 'A' && *char_p <= 'Z')
			{
				assert(failsafe < 100);
				char_p++;
			}
			if (*char_p != ')')
			{
				goto error;
			}
		}
		else if (*(char_p + 1) == ')')
		{
			char_p++;
		}
		else
		{
			goto error;
		}
		char_p++;
	}

	if (result != NULL)
	{
		*result_p = result;
	}

	*char_pp = char_p;

	if (reference_exists)
	{
		return 0;
	}
	else
	{
		return 1;
	}
error:
	if (result != NULL)
	{
		free(result);
	}
	return 2;
}

// doesn't handle cases:
// section 119A(4) of the Enterprise Act 2002 (c. 40) (which applies to functions
static struct section_references get_references_from_match(const char* text)
{
	struct section_references result = init_references();

	const char* char_p = text;

	char_p = parse_prefix(char_p);
	if (char_p == NULL)
	{
		goto parse_end;
	}

	char_p = parse_spaces(char_p);
	if (char_p == NULL)
	{
		goto parse_end;
	}

	char* reference;
	int32_t error = parse_section_reference(&reference, &char_p);
	if (error > 0)
	{
		goto parse_end;
	}
	add_reference(&result, reference);

	bool had_space;
	while (true)
	{
		had_space = false;
		if (*char_p == ' ')
		{
			had_space = true;
			while (*char_p == ' ')
			{
				char_p++;
			}
		}

		bool is_from_to = false;
		if (*char_p == ',')
		{
			char_p++;
			while (*char_p == ' ')
			{
				char_p++;
			}
		}
		else if (had_space)
		{
			if (str_is_prefix_ci(char_p, "and"))
			{
				char_p += strlen("and");
			}
			else if (str_is_prefix_ci(char_p, "or"))
			{
				char_p += strlen("or");
			}
			else if (str_is_prefix_ci(char_p, "to"))
			{
				char_p += strlen("to");
				is_from_to = true;
			}
			else
			{
				break;
			}

			char_p = parse_spaces(char_p);
			if (char_p == NULL)
			{
				goto parse_end;
			}
		}
		else
		{
			goto parse_end;
		}

		char const* const previous_reference = reference;
		error = parse_section_reference(&reference, &char_p);
		if (error > 1)
		{
			goto parse_end;
		}
		else if (error < 1)
		{
			// what to with section 199A to 210?
			if (is_from_to)
			{
				for (long int i = strtol(previous_reference, NULL, 10) + 1;
				        i < strtol(reference, NULL, 10); i++)
				{
					char* i_string = malloc(10);
					if (i_string == NULL)
					{
						abort();
					}
					int chars = snprintf(i_string, 10, "%ld", i);
					if (chars <= 0)
					{
						abort();
					}

					add_reference(&result, i_string);
				}
			}
			add_reference(&result, reference);
		}
	}

	if (had_space)
	{
		char const of_suffix[] = "of ";
		if (str_is_prefix_ci(char_p, of_suffix))
		{
			char_p += strlen(of_suffix);
			if (!str_is_prefix_ci(char_p, "this act"))
			{
				goto ignore_references;
			}
		}
	}

parse_end:
	return result;

ignore_references:
	free_references(&result);
	return init_references();
}

static int32_t get_reference_count(struct section_references references)
{
	return references.elem_count;
}

static char* get_reference(struct section_references const references,
                           int32_t index)
{
	return references.list[index];
}

static void free_references_shallow(struct section_references* array_p)
{
	struct section_references array = *array_p;

	free(array.list);
	array.list = NULL;
	array.list_size = 0;
	array.elem_count = 0;

	*array_p = array;
}

struct section_references init_references()
{
	struct section_references result =
	{
		.list = malloc(1 * sizeof(char*)),
		.list_size = 1,
		.elem_count = 0
	};
	if (result.list == NULL)
	{
		abort();
	}

	return result;
}

void free_references(struct section_references* array_p)
{
	struct section_references array = *array_p;

	for (int32_t i = 0; i < get_reference_count(array); i++)
	{
		free(get_reference(array, i));
	}

	free_references_shallow(&array);

	*array_p = array;
}

void add_reference(struct section_references* array_p, char* const element)
{
	struct section_references array = *array_p;

	if (array.list_size <= array.elem_count)
	{
		array.list_size *= 2;
		array.list = realloc(array.list, array.list_size * sizeof(char*));
		if (array.list == NULL)
		{
			abort();
		}
	}
	array.list[array.elem_count] = element;
	array.elem_count++;

	*array_p = array;
}

void remove_reference(struct section_references* array_p, int32_t index)
{
	struct section_references array = *array_p;
	assert(array.elem_count > 0);
	for (; index < array.elem_count - 1; index++)
	{
		array.list[index] = array.list[index + 1];
	}
	array.elem_count -= 1;
	*array_p = array;
}

struct section_references get_references_from_text(const char* text)
{
	struct section_references result = init_references();

	const char* error_message;
	int error_offset;
	pcre* regex = pcre_compile("sections?+ ++\\d++", PCRE_CASELESS, &error_message,
	                           &error_offset, NULL);
	if (regex == NULL)
	{
		abort();
	}

	int ovector_size = 30;
	int ovector[ovector_size];
	int text_len = strlen(text);
	int matches = pcre_exec(regex, NULL, text, text_len, 0, 0, ovector,
	                        ovector_size);
	while (matches > 0)
	{
		// char buffer[80];
		// strncpy(buffer, text + ovector[0], 79);
		// buffer[79] = 0;
		// fprintf(stderr, "%s\n", buffer);
		struct section_references refs_from_match = get_references_from_match(
		            text + ovector[0]);
		for (int i = 0; i < refs_from_match.elem_count; i++)
		{
			char* const reference = refs_from_match.list[i];
			add_reference(&result, reference);
			// fprintf(stderr, "%ld ", reference);
		}
		free_references_shallow(&refs_from_match);
		// fprintf(stderr, "\n");

		matches = pcre_exec(regex, NULL, text, text_len, ovector[1], 0, ovector,
		                    ovector_size);
	}
	if (matches != -1)
	{
		// fprintf(stderr, "match error: %d\n", matches);
		abort();
	}

	pcre_free(regex);
	return result;
}
