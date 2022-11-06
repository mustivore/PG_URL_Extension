/*
 * example PostgreSQL input/output function for bigint
 *
 * To learn how to write code for PostgreSQL extensions:
 * C-Language Functions (https://www.postgresql.org/docs/13/xfunc-c.html)
 */
/* Add required libraries here */

#include <stdio.h>
#include "postgres.h"
#include <uriparser/Uri.h>

PG_MODULE_MAGIC;

static void parse_uri(const char *s, UriUriA *urip)
{
	UriParserStateA state;

	state.uri = urip;
	uriParseUriA(&state, s);

	if(state.errorCode != URI_SUCCESS){
		ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
							errmsg("invalid input syntax for type uri at or near \"%s\"",
								state.errorPos)));
	}
}

Datum url_in(PG_FUNCTION_ARGS);
Datum url_out(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(url_in);
Datum 
url_in(PG_FUNCTION_ARGS) 
{
	char *s = PG_GETARG_CSTRING(0);
	UriUriA uri;

	parse_uri(s, &uri);
	uriFreeUriMembersA(&uri);

	//vardata = (uritype *) cstring_to_text(s);
	PG_RETURN_POINTER(uri);
}

PG_FUNCTION_INFO_V1(url_out);
Datum 
url_out(PG_FUNCTION_ARGS) 
{
	Datum datum = PG_GETARG_DATUM(0);
	PG_RETURN_CSTRING(TextDatumGetCString(datum));
}
