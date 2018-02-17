#include "web.h"

#include <stdlib.h>

#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "util.h"
#include "database.h"

static size_t add_page_content(char* contents, size_t size, size_t nmemb,
		void* my_data) {
	struct page* page_p = my_data;
	struct page page = *page_p;
	int32_t offset = page.contents_size;
	size_t size_to_write = size * nmemb;
	page.contents_size += size_to_write;
	page.contents = realloc(page.contents, page.contents_size + 1);
	if (page.contents == NULL) {
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
		char* error_buffer) {
	assert(error);

	CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (code != CURLE_OK) {
		abort();
	}

	CURL* handle = curl_easy_init();
	if (handle == NULL) {
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_USERAGENT, "leg/1.0");
	if (code != CURLE_OK) {
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, add_page_content);
	if (code != CURLE_OK) {
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_URL, url);
	if (code != CURLE_OK) {
		goto cleanup_1;
	}
	code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
	assert(code == CURLE_OK);
	struct page page = { .contents = malloc(1), .contents_size = 0 };
	if (page.contents == NULL) {
		goto cleanup_1;
	}
	page.contents[0] = 0;
	code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &page);
	if (code != CURLE_OK) {
		goto cleanup_1;
	}
	*error = curl_easy_perform(handle);

	curl_easy_cleanup(handle);
	curl_global_cleanup();
	return page;

	cleanup_1: if (page.contents != NULL) {
		free(page.contents);
	}
	if (handle != NULL) {
		curl_easy_cleanup(handle);
	}
	curl_global_cleanup();
	abort();
}

void legislation_free(struct leg_id* legislation_p) {
	struct leg_id legislation = *legislation_p;
	str_free(&legislation.type);
	str_free(&legislation.year);
	str_free(&legislation.number);
	*legislation_p = legislation;
}

bool parse_url(struct leg_id* result, const char* url) {
	char url_buffer[strlen(url) + 1];
	strcpy(url_buffer, url);
	char* domain = strstr(url_buffer, "legislation.gov.uk/");
	if (domain == NULL) {
		return false;
	}
	strtok(domain, "/");

	struct leg_id legislation;
	char* type = strtok(NULL, "/");
	if (type == NULL) {
		return false;
	}
	char* year = strtok(NULL, "/");
	if (year == NULL) {
		return false;
	}
	char* number = strtok(NULL, "/");
	if (number == NULL) {
		return false;
	}

	legislation.type = str_init_ds(strlen(type) + 10);
	str_append(&legislation.type, type);

	legislation.year = str_init_ds(strlen(year) + 10);
	str_append(&legislation.year, year);

	legislation.number = str_init_ds(strlen(number) + 10);
	str_append(&legislation.number, number);

	*result = legislation;

	return true;
}

struct string get_api_url(struct leg_id legislation) {
	struct string normalized = str_init_ds(
			str_length(legislation.type) + str_length(legislation.year)
					+ str_length(legislation.number) + 128);
	str_append(&normalized, "http://www.legislation.gov.uk/");
	str_appends(&normalized, legislation.type);
	str_append(&normalized, "/");
	str_appends(&normalized, legislation.year);
	str_append(&normalized, "/");
	str_appends(&normalized, legislation.number);
	if (str_length(legislation.version_date) == 10) {
		str_append(&normalized, "/");
		str_appends(&normalized, legislation.version_date);
	}
	str_append(&normalized, "/data.xml");

	return normalized;
}

struct page get_web_page(const char* url, CURLcode* error, char* error_buffer) {
	sqlite3* db_conn = db_open_conn();

	const char* sql = "select content from web_cache where url = ?;";
	sqlite3_stmt *statement = db_prepare_stmt(db_conn, sql);
	db_bind_text(statement, 1, url);
	bool is_row;
	db_step(&is_row, statement);
	struct page page = { 0 };
	if (is_row) {
		assert(sqlite3_column_type(statement, 0) == SQLITE_TEXT);
		const char* cached_content = (const char*) sqlite3_column_text(
				statement, 0);
		page.contents_size = sqlite3_column_bytes(statement, 0);
		page.contents = malloc_a(page.contents_size + 1, sizeof(unsigned char));
		strncpy(page.contents, cached_content, page.contents_size);
		page.contents[page.contents_size] = '\0';

		db_step(&is_row, statement);
		assert(!is_row);
	}

	db_close_stmt(statement);

	if (page.contents == NULL) {
		page = make_query(url, error, error_buffer);

		statement = db_prepare_stmt(db_conn,
				"insert into web_cache (url, content) values (?, ?);");
		db_bind_text(statement, 1, url);
		db_bind_text(statement, 2, page.contents);
		db_step(&is_row, statement);
		assert(!is_row);
		db_close_stmt(statement);
	}

	db_close_conn(db_conn);
	*error = CURLE_OK;

	return page;
}
