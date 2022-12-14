#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include <utils/builtins.h>
#include <string.h>
#include <regex.h>

// Authors:
// Ayadi Mustapha 000545704
// Soumaya Izmar 000546128
// Guillaume Wafflard 000479740
// Diaz Y Suarez Esteban 000476205

#define REGEX_URL "((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{1,256}(\\.[a-z]{2,6}\\b)?([-a-zA-Z0-9@:%._\\+~#?&//=]*)"
#define REGEX_HOST "(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{1,256}(\\.[a-z]{2,6}\\b)?"
#define REGEX_FILENAME "([-a-zA-Z0-9@:%._\\+~#?&//=]*)"

PG_MODULE_MAGIC;

// the string url is parsed into this struct
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

// used to store the URL into the DB
typedef struct varlena url_db;

/*
	retrieve the protocol from the given url string and store it into the struct URL
*/
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

/*
	retrieve the authority from the given url string and store it into the struct URL
*/
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

/*
	retrieve the port from the given url string authority and store it into the struct URL
*/
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

/*
	retrieve the user info from the given url string authority and store it into the struct URL
*/
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

/*
	retrieve the host from the given url string authorithy and store it into the struct URL
*/
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

/*
	retrieve the file from the given url string and store it into the struct URL
*/
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

/*
	retrieve the path from the given file part and store it into the struct URL
*/
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

/*
	retrieve the query part from the given file part and store it into the struct URL
*/
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

/*
	retrieve the ref from the given url string and store it into the struct URL
*/
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

/*
	Retrieve the protocol, ref, authority, port, userinfo, host, file, path and query
	from url in string and store these infos in the struct URL.
*/
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

/*
	Returns the default port according to the protocol
	http => 80
	https => 443
*/
static int get_default_port_from_url(URL* url) 
{
	if(strcmp(url->protocol,"https") == 0){
		return 443;
	} else {
		return 80;
	}
}

/*
	Test if the provided URL is valid via a REGEX
	If the URL is not valid, an error is displayed
*/
static bool is_valid_url(const char* url_str, bool return_statement){
	regex_t regex;
	int value_comp;
	int value_match;
	value_comp = regcomp(&regex, REGEX_URL, REG_EXTENDED);
	if (value_comp != 0) {
        elog(ERROR, "Error while compiling the regex");
    }
	value_match = regexec(&regex, url_str, 0, NULL, 0);
	if (value_match == REG_NOMATCH){
		if (return_statement) {
			return false;
		} else {
			elog(ERROR, "Please provide a valid URL");
		}
	}
	return true;
}

/*
	Test if the provided port number is valid
	Check that the port number is not a negative number or bigger than 65353
	If the port number is not valid, an error is displayed
*/
static void is_valid_port(const int port){
  if ( 0 > port || port > 65353){
    elog(ERROR, "Please provide a valid port");
  }
}

/*
	Test if the provided host is valid via a REGEX
	If the host is not valid, an error is displayed
*/
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

/*
	Test if the provided file is valid via a REGEX
	If the file is not valid, an error is displayed
*/
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

/*
	Test if the provided protocol is valid (if it's equal to http or https)
	If the protocol is not valid, an error is displayed
*/
static void is_valid_protocol (const char* protocol){
  if (!(strcmp(protocol, "http") == 0 || strcmp(protocol, "https") == 0)){
    elog(ERROR, "Please provide a valid protocol");
 	} 
}

/*
	Compares two URLs excluding the fragment component
	Compares the two urls depending their protocol, authority and file parts
	Returns a negative if url1 is smaller than url2
	Returns 0 if url1 = url2
	return a positive number > 0 if url1 > url2
*/
static int _url_cmp(URL *url1, URL *url2){
	int diff;
	diff = strcmp(url1->protocol, url2->protocol);
	if(diff == 0){
		diff = strcmp(url1->authority, url2->authority);
		if (diff == 0){
			return strcmp(url1->file, url2->file);
		} 
		return diff;
	}
	return diff;
}

/*
	Creates a URL object from the String representation.
*/
Datum url_in(PG_FUNCTION_ARGS);
/*
	Creates a URL by parsing the given spec within a specified context.
*/
Datum make_url_cont_spec(PG_FUNCTION_ARGS);
/*
	Creates a URL from the specified protocol name, host name, and file name.
*/
Datum make_url_prot_host_file(PG_FUNCTION_ARGS);
/*
	Creates a URL object from the specified protocol, host, port number, and file.
*/
Datum make_url_prot_host_port_file(PG_FUNCTION_ARGS);
/*
	Constructs a string representation of this URL.
*/
Datum url_out(PG_FUNCTION_ARGS);
Datum get_protocol(PG_FUNCTION_ARGS);
Datum get_default_port(PG_FUNCTION_ARGS);
Datum get_authority(PG_FUNCTION_ARGS);
Datum get_host(PG_FUNCTION_ARGS);
Datum get_file(PG_FUNCTION_ARGS);
Datum get_path(PG_FUNCTION_ARGS);
Datum get_query(PG_FUNCTION_ARGS);
Datum get_ref(PG_FUNCTION_ARGS);
Datum get_user_info(PG_FUNCTION_ARGS);
Datum get_port(PG_FUNCTION_ARGS);
/*
	Compares the hosts of two URLs.
*/
Datum url_same_host(PG_FUNCTION_ARGS);
/*
	Compares two URLs, excluding the fragment component
*/
Datum url_same_file(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_in);
Datum url_in(PG_FUNCTION_ARGS) 
{
	char *str_url;
    url_db *var_url_db;
	str_url = PG_GETARG_CSTRING(0);
	is_valid_url(str_url, false);
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


PG_FUNCTION_INFO_V1(make_url_prot_host_port_file);
Datum make_url_prot_host_port_file(PG_FUNCTION_ARGS)
{
	char *str_prot;
	int port_url;
	char port_char[20];
	char* str_host;
	char*str_file;
	url_db *var_url_db;
	char* final_url;
	str_prot = PG_GETARG_CSTRING(0);
	is_valid_protocol(str_prot);
	str_host =  PG_GETARG_CSTRING(1);
	is_valid_host(str_host);
	port_url = PG_GETARG_INT32(2);
	is_valid_port(port_url);
	sprintf(port_char, "%d", port_url);
	str_file = PG_GETARG_CSTRING(3);
	is_valid_file(str_file);
	final_url = malloc(sizeof(char)*(strlen(str_prot) + strlen(str_host) +strlen(str_file)+strlen(port_char) + 6));
	strcpy(final_url,str_prot);
	strcat(final_url, "://");
	strcat(final_url, str_host);
	strcat(final_url, ":");
	strcat(final_url, port_char);
	if(strncmp("/", str_file,1) != 0){
		strcat(final_url, "/");
	}
	strcat(final_url, str_file);
	var_url_db = (url_db *) cstring_to_text(final_url);
	free(final_url);
	PG_RETURN_POINTER(var_url_db);
}

PG_FUNCTION_INFO_V1(make_url_prot_host_file);
Datum make_url_prot_host_file(PG_FUNCTION_ARGS)
{
	char *str_prot;
	char* str_host;
	char*str_file;
	url_db *var_url_db;
	char* final_url;
	str_prot = PG_GETARG_CSTRING(0);
	is_valid_protocol(str_prot);
	str_host =  PG_GETARG_CSTRING(1);
	is_valid_host(str_host);
	str_file = PG_GETARG_CSTRING(2);
	is_valid_file(str_file);
	final_url = malloc(sizeof(char)*(strlen(str_prot) + strlen(str_host) +strlen(str_file) + 3 + 2));
	strcpy(final_url,str_prot);
	strcat(final_url, "://");
	strcat(final_url, str_host);
	if(strncmp("/", str_file,1) != 0){
		strcat(final_url, "/");
	}
	strcat(final_url, str_file);
	var_url_db = (url_db *) cstring_to_text(final_url);
	free(final_url);
	PG_RETURN_POINTER(var_url_db);
}

PG_FUNCTION_INFO_V1(make_url_cont_spec);
Datum make_url_cont_spec(PG_FUNCTION_ARGS) {
	char* context = PG_GETARG_CSTRING(0);
	char* spec = PG_GETARG_CSTRING(1);
	url_db *var_url_db;
	char* final_url;
	if (is_valid_url(spec, true)) { //1. check whether spec is a complete URL
		 final_url = malloc(sizeof(char)*(strlen(spec)+1));
		 strcpy(final_url, spec);//only keep spec
	} else { //keep context
		URL *url = (URL *) malloc(sizeof(URL)); 
		is_valid_url(context, false);
		parse_url(context, url);
		final_url = malloc(sizeof(char)*(strlen(url->protocol)+3+strlen(url->host)+strlen(url->path)+strlen(spec)+1+1));
		strcpy(final_url,url->protocol);
		strcat(final_url, "://");
	 	strcat(final_url, url->host);
		if (spec[0]=='/') { //spec is an absolute path
			strcat(final_url, spec); 
		} else {
			is_valid_file(spec); //spec is a relative path
			strcat(final_url, url->path);
			strcat(final_url, "/");
			strcat(final_url, spec); 
		}
	}
	var_url_db = (url_db *) cstring_to_text(final_url);
	free(final_url);
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

PG_FUNCTION_INFO_V1(get_default_port);
Datum get_default_port(PG_FUNCTION_ARGS) 
{
	Datum url_db = PG_GETARG_DATUM(0);
	char *url_str = TextDatumGetCString(url_db);
	URL *url = (URL *) malloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_INT32(get_default_port_from_url(url));
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

PG_FUNCTION_INFO_V1(url_same_file);
Datum url_same_file(PG_FUNCTION_ARGS) 
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(strcmp(url1->file, url2->file) == 0);
}

PG_FUNCTION_INFO_V1(url_same_host);
Datum url_same_host(PG_FUNCTION_ARGS) 
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(strcmp(url1->host, url2->host) == 0);
}

PG_FUNCTION_INFO_V1(url_lt);
Datum
url_lt(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) < 0);
}

PG_FUNCTION_INFO_V1(url_le);
Datum
url_le(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) <= 0);
}

PG_FUNCTION_INFO_V1(url_eq);
Datum
url_eq(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) == 0);
}

PG_FUNCTION_INFO_V1(url_ne);
Datum
url_ne(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) != 0);
}

PG_FUNCTION_INFO_V1(url_ge);
Datum
url_ge(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) >= 0);
}

PG_FUNCTION_INFO_V1(url_gt);
Datum
url_gt(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_BOOL(_url_cmp(url1, url2) > 0);
}

PG_FUNCTION_INFO_V1(url_cmp);
Datum
url_cmp(PG_FUNCTION_ARGS)
{
	char *url1_str = TextDatumGetCString(PG_GETARG_DATUM(0));
	char *url2_str = TextDatumGetCString(PG_GETARG_DATUM(1));
	URL *url1 = (URL *) malloc(sizeof(URL));
	URL *url2 = (URL *) malloc(sizeof(URL));
	parse_url(url1_str,url1);
	parse_url(url2_str,url2);
	PG_RETURN_INT32(_url_cmp(url1, url2));
}
