#include "web.h"

#include <stdlib.h>

#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "util.h"

static size_t add_page_content(char* contents, size_t size, size_t nmemb,
                               void* my_data)
{
	struct page* page_p = my_data;
	struct page page = *page_p;
	int32_t offset = page.contents_size;
	size_t size_to_write = size * nmemb;
	page.contents_size += size_to_write;
	page.contents = realloc(page.contents, page.contents_size + 1);
	if (page.contents == NULL)
	{
		page.contents_size = 0;
		*page_p = page;
		return 0;
	}

	memcpy(page.contents + offset, contents, size_to_write);
	page.contents[page.contents_size] = 0;

	*page_p = page;
	return size_to_write;
}

static struct page make_query(const char* url, CURLcode* const error,
                              char* error_buffer)
{
	assert(error);

	CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (code != CURLE_OK)
	{
		abort();
	}

	CURL* handle = curl_easy_init();
	if (handle == NULL)
	{
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_USERAGENT, "leg/1.0");
	if (code != CURLE_OK)
	{
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, add_page_content);
	if (code != CURLE_OK)
	{
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_URL, url);
	if (code != CURLE_OK)
	{
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
	assert(code == CURLE_OK);
	struct page page =
	{
		.contents = malloc(1),
		.contents_size = 0
	};
	if (page.contents == NULL)
	{
		goto cleanup_1;
	}
	page.contents[0] = 0;
	code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &page);
	if (code != CURLE_OK)
	{
		goto cleanup_1;
	}
	*error = curl_easy_perform(handle);

	curl_easy_cleanup(handle);
	curl_global_cleanup();
	return page;

cleanup_1:
	if (page.contents != NULL)
	{
		free(page.contents);
	}
	if (handle != NULL)
	{
		curl_easy_cleanup(handle);
	}
	curl_global_cleanup();
	abort();
}

void legislation_free(struct legislation* legislation_p)
{
	struct legislation legislation = *legislation_p;
	strd_free(&legislation.type);
	strd_free(&legislation.year);
	strd_free(&legislation.number);
	*legislation_p = legislation;
}

bool parse_url(struct legislation* result, const char* url)
{
	char url_buffer[strlen(url) + 1];
	strcpy(url_buffer, url);
	char* domain = strstr(url_buffer, "legislation.gov.uk/");
	if (domain == NULL)
	{
		return false;
	}
	strtok(domain, "/");

	struct legislation legislation;
	char* type = strtok(NULL, "/");
	if (type == NULL)
	{
		return false;
	}
	char* year = strtok(NULL, "/");
	if (year == NULL)
	{
		return false;
	}
	char* number = strtok(NULL, "/");
	if (number == NULL)
	{
		return false;
	}

	legislation.type = strd_mallocn(strlen(type) + 10);
	strd_append(&legislation.type, type);

	legislation.year = strd_mallocn(strlen(year) + 10);
	strd_append(&legislation.year, year);

	legislation.number = strd_mallocn(strlen(number) + 10);
	strd_append(&legislation.number, number);

	*result = legislation;

	return true;
}

struct string_d get_api_url(struct legislation legislation)
{
	struct string_d normalized = strd_mallocn(strd_length(legislation.type) +
	                             strd_length(legislation.year) + strd_length(
	                                 legislation.number) + 128);
	strd_append(&normalized, "http://www.legislation.gov.uk/");
	strd_append(&normalized, strd_content(legislation.type));
	strd_append(&normalized, "/");
	strd_append(&normalized, strd_content(legislation.year));
	strd_append(&normalized, "/");
	strd_append(&normalized, strd_content(legislation.number));
	strd_append(&normalized, "/data.xml");

	return normalized;
}

struct page get_web_page(const char* url, CURLcode* error, char* error_buffer)
{
	sqlite3* db_conn;
	int return_code = sqlite3_open("cache.db", &db_conn);
	if (return_code != SQLITE_OK)
	{
		abort();
	}

	const char* sql = "select content from web_pages where url = ?;";
	sqlite3_stmt *statement;
	return_code = sqlite3_prepare_v2(db_conn, sql, -1, &statement, NULL);
	if (return_code != SQLITE_OK)
	{
		abort();
	}
	return_code = sqlite3_bind_text(statement, 1, url, -1, SQLITE_STATIC);
	if (return_code != SQLITE_OK)
	{
		abort();
	}
	return_code = sqlite3_step(statement);
	struct page page = {0};
	if (return_code == SQLITE_ROW)
	{
		assert(sqlite3_column_type(statement, 0) == SQLITE_TEXT);
		const char* cached_content = (const char*) sqlite3_column_text(statement, 0);
		page.contents_size = sqlite3_column_bytes(statement, 0);
		page.contents = malloc_a(page.contents_size + 1, sizeof(unsigned char));
		strncpy(page.contents, cached_content, page.contents_size);
		page.contents[page.contents_size] = '\0';

		assert(sqlite3_step(statement) == SQLITE_DONE);
	}
	else if (return_code != SQLITE_DONE)
	{
		abort();
	}

	return_code = sqlite3_finalize(statement);
	if (return_code != SQLITE_OK)
	{
		abort();
	}

	if (page.contents == NULL)
	{
		page = make_query(url, error, error_buffer);

		return_code = sqlite3_prepare_v2(db_conn,
		                                 "insert into web_pages (url, content) values (?, ?);", -1, &statement, NULL);
		if (return_code != SQLITE_OK)
		{
			abort();
		}
		return_code = sqlite3_bind_text(statement, 1, url, -1, SQLITE_STATIC);
		if (return_code != SQLITE_OK)
		{
			abort();
		}
		return_code = sqlite3_bind_text(statement, 2, page.contents, -1, SQLITE_STATIC);
		if (return_code != SQLITE_OK)
		{
			abort();
		}
		return_code = sqlite3_step(statement);
		assert(return_code == SQLITE_DONE);
	}

	return_code = sqlite3_close_v2(db_conn);
	if (return_code != SQLITE_OK)
	{
		abort();
	}

	*error = CURLE_OK;

	return page;
}

