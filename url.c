#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include <utils/builtins.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

PG_MODULE_MAGIC;

typedef struct _url {
   char* scheme;
   char* host;
   char* path;
   char* query;
   char* port;
   char* protocol;
}  URL;

typedef struct varlena url_db;

/*static void parse_url(const char* url_str, URL *url)
{
	url->host = "";
}*/

static void is_valid_url(const char* url_str){
	regex_t regex;
	int value_comp;
	int value_match;
	value_comp = regcomp( &regex, "((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)", REG_EXTENDED);
	if (value_comp != 0) {
        elog(ERROR, "Error while compiling the regex");
    }
	value_match = regexec(&regex, url_str, 0, NULL, 0);
	if (value_match == REG_NOMATCH){
		elog(ERROR, "Please provide a valid URL");
	}
}

Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS) 
{
	char *str_url;
    url_db *var_url_db;
	str_url = PG_GETARG_CSTRING(0);
	is_valid_url(str_url);
	var_url_db = (url_db *) cstring_to_text(str_url);
	PG_RETURN_POINTER(var_url_db);
}

PG_FUNCTION_INFO_V1(url_out);
Datum url_out(PG_FUNCTION_ARGS) 
{	
	Datum url_db = PG_GETARG_DATUM(0);
	PG_RETURN_CSTRING(TextDatumGetCString(url_db));
}


