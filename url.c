#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include <utils/builtins.h>
#include <string.h>
#include <regex.h>

#define REGEX_URL "((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)"

PG_MODULE_MAGIC;

typedef struct _url {
   char *scheme;
   char *host;
   char *path;
   char *query;
   char *protocol;
   char *authority;
   char *user_info; 
   char *file; 
   char *ref;
   int   port;
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
	token = strtok(url_to_stroke, "?");
	url->authority = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url->authority, token);
	free(url_to_stroke);
}

static void retrieve_port_from_authority(const char* url_str, URL *url)
{
	char *e;
	int port; 
	e = strchr(url_str,':');
	if (e != NULL) {
		e = e + 1;
		port = atoi(e);
		url->port = port;
	}else{
		url->port = -1;
	}
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
	free(url_to_stroke);
}

static void retrieve_host(const char* url_str, URL *url)
{
	char *e;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str);
	url->host = malloc(sizeof(char)*(strlen(url_str)+1));
	e = strchr(url_to_stroke,'@');
	if (e != NULL) {
		e = e + 1;
		strcpy(url->host, e);
	}else{
		strcpy(url->host, url_to_stroke);
	}
	free(url_to_stroke);
}

static void retrieve_file(const char* url_str, URL *url)
{
	char *token;
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str);
	token = strtok(url_to_stroke, "#");
	url->file = malloc(sizeof(char)*(strlen(url_str)+1));
	if(token != NULL){
		strcpy(url->file, token);
	}else{
		strcpy(url->file, "");
	}
	free(url_to_stroke);
}

static void retrieve_path_from_file(const char* url_str, URL *url)
{
	char *url_to_stroke = malloc(sizeof(char)*(strlen(url_str)+1));
	strcpy(url_to_stroke, url_str);
	url->path = malloc(sizeof(char)*(strlen(url_str)+1));
	if (strchr(url_to_stroke,'/') != NULL){
		char *token = strtok(url_to_stroke, "?");
		strcpy(url->path, token);
	} else {
		strcpy(url->path, "");
	}
	
	
	free(url_to_stroke);
}

static void retrieve_query_from_file(const char* url_str, URL *url)
{
	char *e;
	e = strchr(url_str,'?');
	url->query = malloc(sizeof(char)*(strlen(url_str)+1));
	if (e != NULL) {
		e = e + 1;
		strcpy(url->query, e);
	} else {
		strcpy(url->query, "");
	}
	
}


static void retrieve_ref(const char* url_str, URL *url)
{
	char *fragment_part = strchr(url_str,'#');
	url->ref = malloc(sizeof(char)*(strlen(url_str)+1));
	if(fragment_part != NULL){
		fragment_part = fragment_part + 1;
		strcpy(url->ref, fragment_part);
	} else {
		strcpy(url->ref, "");
	}

}

static void parse_url(char* url_str, URL *url)
{
	retrieve_protocol(url_str,url);
	url_str = url_str + (strlen(url->protocol) + 3);
	retrieve_ref(url_str,url);
	retrieve_authority(url_str,url);
	retrieve_port_from_authority(url->authority,url);
	retrieve_userinfo(url->authority,url);
	retrieve_host(url->authority,url);
	url_str = url_str + strlen(url->authority);
	retrieve_file(url_str,url);
	retrieve_path_from_file(url->file,url);
	retrieve_query_from_file(url->file,url);
}

static void is_valid_url(const char* url_str){
	regex_t regex;
	int value_comp;
	int value_match;
	value_comp = regcomp(&regex, REGEX_URL, REG_EXTENDED);
	if (value_comp != 0) {
        elog(ERROR, "Error while compiling the regex");
    }
	value_match = regexec(&regex, url_str, 0, NULL, 0);
	if (value_match == REG_NOMATCH){
		elog(ERROR, "Please provide a valid URL");
	}
}

static int _url_cmp(char *url1_str, char *url2_str){
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	int diff;
	parse_url(url1_str,url1);
	parse_url(url1_str,url2);
	diff = strcmp(url1->file, url2->file);
	if(diff == 0){
		free(url1);
		free(url2);
		return strcmp(url1->file, url2->file);
	} else {
		free(url1);
		free(url2);
		return diff;
	}
	
}

Datum url_in(PG_FUNCTION_ARGS);
// Datum create_url(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);
Datum get_protocol(PG_FUNCTION_ARGS);
Datum get_default_port(PG_FUNCTION_ARGS);
Datum get_authority(PG_FUNCTION_ARGS);
Datum get_host(PG_FUNCTION_ARGS);
Datum get_file(PG_FUNCTION_ARGS);
Datum get_path(PG_FUNCTION_ARGS);
Datum get_query(PG_FUNCTION_ARGS);
Datum get_ref(PG_FUNCTION_ARGS);

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

// PG_FUNCTION_INFO_V1(create_url);
// Datum create_url(PG_FUNCTION_ARGS) 
// {
// 	char *protocol;
// 	char *host;
// 	char *file;
// 	url_db *var_url_db;
// 	protocol = PG_GETARG_CSTRING(0);
// 	host = PG_GETARG_CSTRING(1);
// 	file = PG_GETARG_CSTRING(2);
// 	//is_valid_url(str_url);
// 	elog(WARNING,"%s %s %s",protocol,host,file);
// 	var_url_db = (url_db *) cstring_to_text(protocol);
// 	PG_RETURN_POINTER(var_url_db);
// }

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

PG_FUNCTION_INFO_V1(get_file);
Datum get_file(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_CSTRING(cstring_to_text(url->file));
}

PG_FUNCTION_INFO_V1(get_path);
Datum get_path(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_CSTRING(cstring_to_text(url->path));
}

PG_FUNCTION_INFO_V1(get_query);
Datum get_query(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	if(strlen(url->query) == 0){
		PG_RETURN_NULL();
	} else {
		PG_RETURN_CSTRING(cstring_to_text(url->query));
	}
	
}

PG_FUNCTION_INFO_V1(get_port);
Datum get_port(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_INT32(url->port);	
}

PG_FUNCTION_INFO_V1(get_ref);
Datum get_ref(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	if(strlen(url->ref) == 0){
		PG_RETURN_NULL();
	} else {
		PG_RETURN_CSTRING(cstring_to_text(url->ref));	
	}
}

PG_FUNCTION_INFO_V1(url_lt);
Datum
url_lt(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) < 0);
}

PG_FUNCTION_INFO_V1(url_le);
Datum
url_le(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) <= 0);
}

PG_FUNCTION_INFO_V1(url_eq);
Datum
url_eq(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) == 0);
}

PG_FUNCTION_INFO_V1(url_ne);
Datum
url_ne(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) != 0);
}

PG_FUNCTION_INFO_V1(url_ge);
Datum
url_ge(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) >= 0);
}

PG_FUNCTION_INFO_V1(url_gt);
Datum
url_gt(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_BOOL(_url_cmp(url1_str, url2_str) > 0);
}

PG_FUNCTION_INFO_V1(url_cmp);
Datum
url_cmp(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));

	PG_RETURN_INT32(_url_cmp(url1_str, url2_str));
}