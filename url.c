#include <stdio.h>
#include "postgres.h"
#include <uriparser/Uri.h>
#include <utils/builtins.h>

PG_MODULE_MAGIC;

typedef struct _url {
   char* scheme;
   char* host;
   char* path;
   char* query;
   char* port;
   char* url;
   //char* protocol;
}  URL;

static inline void parse_url(const char* url_str, URL *url)
{
	UriUriA uri;
	UriParserStateA state;

	state.uri = &uri;

	uriParseUriA(&state, url_str);

	if(state.errorCode != URI_SUCCESS){
		ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
							errmsg("invalid input syntax for type uri at or near \"%s\"",
								state.errorPos)));
	}
	strcpy(url->scheme, uri.scheme.first);
	strcpy(url->path, uri.userInfo.first);
	strcpy(url->query, uri.query.first); 
	strcpy(url->port, uri.portText.first);
	strcpy(url->url, url_str);
	uriFreeUriMembersA(&uri);
}

Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);

typedef struct varlena uritype;

PG_FUNCTION_INFO_V1(url_in);
Datum 
url_in(PG_FUNCTION_ARGS) 
{
	char* url_str = PG_GETARG_CSTRING(0);

	URL* url = (URL*) palloc(sizeof(URL));
	parse_url(url_str, url);
	PG_RETURN_POINTER(url);
}

PG_FUNCTION_INFO_V1(url_out);
Datum 
url_out(PG_FUNCTION_ARGS) 
{
	URL* url = (URL *) PG_GETARG_POINTER(0);
	PG_RETURN_CSTRING(url->url);
}
