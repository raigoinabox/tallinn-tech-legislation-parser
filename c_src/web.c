#include "web.h"

#include <assert.h>
#include <curl/easy.h>
#include <curl/typecheck-gcc.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "database.h"
#include "printing.h"
#include "util.h"

static size_t add_page_content(char* contents, size_t size, size_t nmemb,
                               void* my_data)
{
    size_t size_to_write = size * nmemb;
    str_appendn(my_data, contents, size_to_write);
    return size_to_write;
}

static bool make_query(
		struct str_builder* result, const char* url, struct error* error)
{
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    bool success = true;
    CURL* handle = { 0 };
    if (code != CURLE_OK)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }

    handle = curl_easy_init();
    if (handle == NULL)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_USERAGENT, "leg/1.0");
    if (code != CURLE_OK)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, add_page_content);
    if (code != CURLE_OK)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }
	code = curl_easy_setopt(handle, CURLOPT_URL, url);
    if (code != CURLE_OK)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }
    char error_buffer[CURL_ERROR_SIZE];
    code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
    assert(code == CURLE_OK);
    struct str_builder page = str_init();
    code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &page);
    if (code != CURLE_OK)
    {
		register_error(error, "Unknown error");
        success = false;
        goto exit;
    }
    code = curl_easy_perform(handle);
    if (code != CURLE_OK)
    {
        success = false;
		register_error(error, str_copy(error_buffer));
    }

exit:
    if (success)
    {
        *result = page;
    }
    else
    {
        str_builder_destroy(&page);
    }
    curl_easy_cleanup(handle);
    curl_global_cleanup();
    return success;
}

bool parse_leg_url(struct legal_act_id* result, const char* url)
{
    char url_buffer[strlen(url) + 1];
    strcpy(url_buffer, url);
    char* domain = strstr(url_buffer, "legislation.gov.uk/");
    if (domain == NULL)
    {
        return false;
    }
    strtok(domain, "/");

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

	struct legal_act_id legislation = { 0 };
	legislation.type = str_copy(type);
	legislation.year = str_copy(year);
	legislation.number = str_copy(number);

    *result = legislation;
    return true;
}

char* get_leg_api_url(struct legal_act_id legislation)
{
    struct str_builder url = str_init();
    str_appendc(&url, "http://www.legislation.gov.uk/");
	str_appendc(&url, legislation.type);
    str_appendc(&url, "/");
	str_appendc(&url, legislation.year);
    str_appendc(&url, "/");
	str_appendc(&url, legislation.number);
	if (!str_empty(legislation.version_date))
    {
        str_appendc(&url, "/");
		str_appendc(&url, legislation.version_date);
    }
    str_appendc(&url, "/data.xml");

	return str_content(&url);
}

static bool insert_cache(
		struct db_conn db_conn, const char* url, const char* content,
		struct error* error)
{
    struct db_params params = db_params_init();
	db_params_append(&params, url);
	db_params_append(&params, content);
	bool success = db_exec_params(
			db_conn, "insert into web_cache (url, content) values ($1, $2);",
			params, error);
	register_frame(error);
	return success;
}

bool get_web_page(struct str_builder* result, const char* url, struct error* error)
{
    struct db_conn db_conn = db_open_conn();

    struct db_params param_vec = db_params_init();
	db_params_append(&param_vec, url);
    const char* sql = "select content from web_cache where url = $1;";
    struct db_result db_result = {0};
	if (!db_query_params(&db_result, db_conn, sql, param_vec, error))
	{
		print_error(*error);
		abort();
	}
    db_params_destroy(&param_vec);

    struct str_builder page = { 0 };
	bool success = true;
	bool is_cached = false;
    if (db_nrows(db_result) > 0)
    {
        const char* cached_content = db_get_value(db_result, 0, 0);

        page = str_init();
        str_appendc(&page, cached_content);
		is_cached = true;
	}
	else
	{
		success = make_query(&page, url, error);
    }

    db_close_result(db_result);
	if (success && !is_cached)
	{
		success = insert_cache(db_conn, url, str_content(&page), error);
	}

    if (success) {
    	*result = page;
	}

	register_frame(error);
    db_close_conn(db_conn);
    return success;
}

