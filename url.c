#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include <utils/builtins.h>
#include <string.h>
#include <regex.h>

#define REGEX_URL "((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)"
#define REGEX_HOST "(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$)"
#define REGEX_FILENAME "(^([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9/-]*[A-Za-z0-9])$)"

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


static void is_valid_port(const int port){
  if ( 0 > port || port > 65353){
    elog(ERROR, "Please provide a valid port");
  }
}
static void is_valid_host(const char* host){
  regex_t regex;
  int value_comp;
  int value_match;
  value_comp = regcomp(&regex, REGEX_HOST, REG_EXTENDED);
  if (value_comp != 0) {
        elog(ERROR, "Error while compiling the regex");
    }
  value_match = regexec(&regex, host, 0, NULL, 0);
  if (value_match == REG_NOMATCH){
    elog(ERROR, "Please provide a valid host");
  }
}
static void is_valid_file (const char* filename){
  regex_t regex;
  int value_comp;
  int value_match;
  value_comp = regcomp(&regex, REGEX_FILENAME, REG_EXTENDED);
  if (value_comp != 0) {
        elog(ERROR, "Error while compiling the regex");
    }
  value_match = regexec(&regex, filename, 0, NULL, 0);
  if (value_match == REG_NOMATCH){
    elog(ERROR, "Please provide a valid file");
  }
}
static void is_valid_protocol (const char* protocol){
  if (!(strcmp(protocol, "http") == 0 || strcmp(protocol, "https") == 0)){
    elog(ERROR, "Please provide a valid protocol");
 	} 
}


static int _url_cmp(char *url1_str, char *url2_str){
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	int diff;
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
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
Datum make_url(PG_FUNCTION_ARGS);
Datum make_url_cont_spec(PG_FUNCTION_ARGS);
Datum make_url_prot_host_file(PG_FUNCTION_ARGS);
Datum make_url_prot_host_port_file(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);
Datum get_protocol(PG_FUNCTION_ARGS);
Datum get_default_port(PG_FUNCTION_ARGS);
Datum get_authority(PG_FUNCTION_ARGS);
Datum get_host(PG_FUNCTION_ARGS);
Datum get_file(PG_FUNCTION_ARGS);
Datum get_path(PG_FUNCTION_ARGS);
Datum get_query(PG_FUNCTION_ARGS);
Datum get_ref(PG_FUNCTION_ARGS);
Datum equals(PG_FUNCTION_ARGS);
Datum same_file(PG_FUNCTION_ARGS);
Datum same_host(PG_FUNCTION_ARGS);


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

	PG_FUNCTION_INFO_V1(make_url);
Datum make_url(PG_FUNCTION_ARGS)
{
	char *str_url;
    url_db *var_url_db;
	str_url = PG_GETARG_CSTRING(0);
	is_valid_url(str_url);
	var_url_db = (url_db *) cstring_to_text(str_url);
	PG_RETURN_POINTER(var_url_db);
}
PG_FUNCTION_INFO_V1(make_url_prot_host_port_file);
Datum make_url_prot_host_port_file(PG_FUNCTION_ARGS)
{
	char *str_prot;
  int port_url;
  char port_char[20];
  char* str_host;
  char*str_file;
  url_db *var_url_db;
  char final_url[512]="";
	str_prot = PG_GETARG_CSTRING(0);
  is_valid_protocol(str_prot);
  //is_valid_url(str_url);
  str_host =  PG_GETARG_CSTRING(1);
  is_valid_host(str_host);
  port_url = PG_GETARG_INT32(2);
  is_valid_port(port_url);
  sprintf(port_char, "%d", port_url);
  str_file = PG_GETARG_CSTRING(3);
  is_valid_file(str_file);
  strcat(final_url,str_prot);
  strcat(final_url, "://");
  strcat(final_url, str_host);
  strcat(final_url, ":");
  strcat(final_url, port_char);
  strcat(final_url, "/");
  strcat(final_url, str_file);
	var_url_db = (url_db *) cstring_to_text(final_url);
	PG_RETURN_POINTER(var_url_db);
}
PG_FUNCTION_INFO_V1(make_url_prot_host_file);
Datum make_url_prot_host_file(PG_FUNCTION_ARGS)
{
	char *str_prot;
  char* str_host;
  char*str_file;
  url_db *var_url_db;
  char final_url[512]="";
	str_prot = PG_GETARG_CSTRING(0);
  is_valid_protocol(str_prot);
  str_host =  PG_GETARG_CSTRING(1);
  is_valid_host(str_host);
  str_file = PG_GETARG_CSTRING(2);
  is_valid_file(str_file);
  strcat(final_url,str_prot);
  strcat(final_url, "://");
  strcat(final_url, str_host);
  strcat(final_url, "/");
  strcat(final_url, str_file);
	var_url_db = (url_db *) cstring_to_text(final_url);
	PG_RETURN_POINTER(var_url_db);
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

static int get_default_port_from_str(char* url_str) 
{
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	if(strcmp(url->protocol,"https") == 0){
		return 443;
	} else {
		return 80;
	}
	free(url);
}

PG_FUNCTION_INFO_V1(get_default_port);
Datum get_default_port(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
		PG_RETURN_INT32(get_default_port_from_str(url_str));
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

/*static boolean equals_from_str(char* url1_str, char* url2_str) {
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	is_valid_url(url1_str);
	is_valid_url(url2_str);
	if (strcmp(url1->protocol, url2->protocol)) return false;
	if (strcmp(url1->host, url2->host)) return false;
	if (strcmp(url1->file, url2->file)) return false;
	if (strcmp(url1->ref, url2->ref)) return false;
	printf("%s %s --> %d %d",url1->host, url2->host, get_default_port_from_str(url1_str), get_default_port_from_str(url2_str) );

	if (url1->port == -1 || url2->port == -1) {
		printf("--> %d %d",get_default_port_from_str(url1_str), get_default_port_from_str(url2_str) );
		if (get_default_port_from_str(url1_str) != get_default_port_from_str(url2_str)) return false; 
	} else {
		if (url1->port != url2->port) return false; 
	}
	return true;
}*/


/*PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS)
/*	Two URL objects are equal if they have the same protocol, reference equivalent hosts, 
	have the same port number on the host, and the same file and fragment of the file. 
	cf. https://docs.oracle.com/javase/8/docs/api/java/net/URL.html#equals-java.lang.Object * /
{
	Datum url_db1 = PG_GETARG_DATUM(0);
	Datum url_db2 = PG_GETARG_DATUM(1);
	char *url1_str = TextDatumGetCString(url_db1);
	char *url2_str = TextDatumGetCString(url_db2);
	PG_RETURN_BOOL(equals_from_str(url1_str, url2_str));
}*/

PG_FUNCTION_INFO_V1(equals);
Datum equals(PG_FUNCTION_ARGS)
/*	Two URL objects are equal if they have the same protocol, reference equivalent hosts, 
	have the same port number on the host, and the same file and fragment of the file. 
	cf. https://docs.oracle.com/javase/8/docs/api/java/net/URL.html#equals-java.lang.Object*/
{
	Datum url_db1 = PG_GETARG_DATUM(0);
	Datum url_db2 = PG_GETARG_DATUM(1);
	char *url1_str = TextDatumGetCString(url_db1);
	char *url2_str = TextDatumGetCString(url_db2);
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	is_valid_url(url1_str);
	is_valid_url(url2_str);
	if (strcmp(url1->protocol, url2->protocol)) return false;
	if (strcmp(url1->host, url2->host)) return false;
	if (strcmp(url1->file, url2->file)) return false;
	if (strcmp(url1->ref, url2->ref)) return false;
	if (url1->port == -1 || url2->port == -1) {
		if (get_default_port_from_str(url1_str) != get_default_port_from_str(url2_str)) return false; 
	} else {
		if (url1->port != url2->port) return false; 
	}
	PG_RETURN_BOOL(true);
}


PG_FUNCTION_INFO_V1(same_file);
Datum same_file(PG_FUNCTION_ARGS)
{
	Datum url_db1 = PG_GETARG_DATUM(0);
	Datum url_db2 = PG_GETARG_DATUM(1);
	char *url1_str = TextDatumGetCString(url_db1);
	char *url2_str = TextDatumGetCString(url_db2);
	char *url1_short = strtok(url1_str,"#");
	char *url2_short = strtok(url2_str,"#");
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_short, url1);
	parse_url(url2_short, url2);	
	if (strcmp(url1->host, url2->host)) return false;
	if (strcmp(url1->file, url2->file)) return false;
	//needs other checks ? 
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(same_host);
Datum same_host(PG_FUNCTION_ARGS)
{
	Datum url_db1 = PG_GETARG_DATUM(0);
	Datum url_db2 = PG_GETARG_DATUM(1);
	char *url1_str = TextDatumGetCString(url_db1);
	char *url2_str = TextDatumGetCString(url_db2);
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str, url1);
	parse_url(url2_str, url2);	
	if (strcmp(url1->host, url2->host)) return false;
	//needs other checks ? 
	PG_RETURN_BOOL(true);
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


