#include "web.h"

#include <assert.h>
#include <curl/easy.h>
#include <curl/typecheck-gcc.h>
#include <sqlite3.h>
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

static bool make_query(struct string* result, const char* url,
        char* error_buffer)
{
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    bool success = true;
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }

    CURL* handle = curl_easy_init();
    if (handle == NULL)
    {
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_USERAGENT, "leg/1.0");
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, add_page_content);
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_URL, url);
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }
    code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
    assert(code == CURLE_OK);
    struct string page = str_init();
    code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &page);
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }
    code = curl_easy_perform(handle);
    if (code != CURLE_OK)
    {
        success = false;
        goto exit;
    }

exit:
    if (success)
    {
        *result = page;
    }
    else
    {
        str_free(&page);
    }
    curl_easy_cleanup(handle);
    curl_global_cleanup();
    return success;
}

bool parse_leg_url(struct leg_id* result, const char* url)
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

    struct string leg_type = str_init_ds(strlen(type) + 10);
    str_appends(&leg_type, type);
    struct string leg_year = str_init_ds(strlen(year) + 10);
    str_appends(&leg_year, year);
    struct string leg_number = str_init_ds(strlen(number) + 10);
    str_appends(&leg_number, number);

    struct leg_id legislation;
    legislation.type = leg_type;
    legislation.year = leg_year;
    legislation.number = leg_number;

    *result = legislation;
    return true;
}

struct string get_leg_api_url(struct leg_id legislation)
{
    struct string url = str_init();
    str_appends(&url, "http://www.legislation.gov.uk/");
    str_append(&url, legislation.type);
    str_appends(&url, "/");
    str_append(&url, legislation.year);
    str_appends(&url, "/");
    str_append(&url, legislation.number);
    if (str_length(legislation.version_date) == 10)
    {
        str_appends(&url, "/");
        str_append(&url, legislation.version_date);
    }
    str_appends(&url, "/data.xml");

    return url;
}

bool get_web_page(
        struct string* result, const char* url, char* error_buffer)
{
    sqlite3* db_conn = db_open_conn();

    const char* sql = "select content from web_cache where url = ?;";
    sqlite3_stmt *statement = db_prepare_stmt(db_conn, sql);
    db_bind_text(statement, 1, url);
    bool is_row;
    if (!db_step(&is_row, statement))
    {
        printf_ea(sqlite3_errmsg(db_conn));
        abort();
    }

    struct string page = { 0 };
    if (is_row)
    {
        assert(sqlite3_column_type(statement, 0) == SQLITE_TEXT);
        const char* cached_content = (const char*) sqlite3_column_text(
                                         statement, 0);

        page = str_init();
        int contents_size = sqlite3_column_bytes(statement, 0);
        str_appendn(&page, cached_content, contents_size);

        if (!db_step(&is_row, statement))
        {
            printf_ea(sqlite3_errmsg(db_conn));
            abort();
        }
        assert(!is_row);
    }

    db_close_stmt(statement);

    bool success = true;
    if (str_content(page) == NULL)
    {
        if (!make_query(&page, url, error_buffer)) {
            success = false;
            goto exit;
        }

        statement = db_prepare_stmt(db_conn,
                                    "insert into web_cache (url, content) values (?, ?);");
        db_bind_text(statement, 1, url);
        db_bind_text(statement, 2, str_content(page));
        if (!db_step(&is_row, statement))
        {
            printf_ea(sqlite3_errmsg(db_conn));
            abort();
        }
        assert(!is_row);
        db_close_stmt(statement);
    }

    *result = page;
exit:
    db_close_conn(db_conn);
    *result = page;
    return success;
}

