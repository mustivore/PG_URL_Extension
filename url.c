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
   char* authority;
   char* user_info; 
}  URL;

typedef struct varlena url_db;

static void retrieve_protocol(const char* url_str, URL *url)
{
	char *token;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str); 
	token = strtok(url_to_stroke, "://");
	url->protocol = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url->protocol, token);
	free(url_to_stroke);
}

static void retrieve_authority(const char* url_str, URL *url)
{
	char *token;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str);
	token = strtok(url_to_stroke, "/");
	url->authority = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url->authority, token);
	free(url_to_stroke);
}

static void retrieve_userinfo(const char* url_str, URL *url)
{
	char *token;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	char *e;
	strcpy(url_to_stroke, url_str);
	e = strchr(url_to_stroke,'@');
    if (e == NULL) {
		url->user_info = malloc(sizeof(char)*(strlen(url_str)+1));
		strcpy(url->user_info, "\0");
	}else{
		token = strtok(url_to_stroke, "@");
		url->user_info = malloc(sizeof(char)*(strlen(url_str)+1));
		strcpy(url->user_info, token);
	}
}

static void retrieve_host(const char* url_str, URL *url)
{
	char *e;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str);
	e = strchr(url_to_stroke,'@');
	if (e != NULL) {
		url_to_stroke = url_to_stroke + (strlen(url->user_info) + 1);
	}
	url->host = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url->host, url_to_stroke);
}

static void parse_url(char* url_str, URL *url)
{
	retrieve_protocol(url_str,url);
	url_str = url_str + (strlen(url->protocol) + 3);
	retrieve_authority(url_str,url);
	retrieve_userinfo(url->authority,url);
	retrieve_host(url->authority,url);
}

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
Datum get_protocol(PG_FUNCTION_ARGS);
Datum get_default_port(PG_FUNCTION_ARGS);
Datum get_authority(PG_FUNCTION_ARGS);
Datum get_host(PG_FUNCTION_ARGS);

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
	char *url_str = TextDatumGetCString(url_db);
	PG_RETURN_CSTRING(url_str);
}

PG_FUNCTION_INFO_V1(get_protocol);
Datum get_protocol(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_CSTRING(cstring_to_text(url->protocol));
}

PG_FUNCTION_INFO_V1(get_default_port);
Datum get_default_port(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	if(strcmp(url->protocol,"https") == 0){
		PG_RETURN_INT32(443);
	} else {
		PG_RETURN_INT32(80);
	}
}

PG_FUNCTION_INFO_V1(get_authority);
Datum get_authority(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_CSTRING(cstring_to_text(url->authority));
}

PG_FUNCTION_INFO_V1(get_user_info);
Datum get_user_info(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	if(strlen(url->user_info) == 0){
		PG_RETURN_NULL();
	} else {
		PG_RETURN_CSTRING(cstring_to_text(url->user_info));
	}
}

PG_FUNCTION_INFO_V1(get_host);
Datum get_host(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_CSTRING(cstring_to_text(url->host));
}