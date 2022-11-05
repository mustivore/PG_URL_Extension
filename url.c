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


PG_FUNCTION_INFO_V1(uri_in);
Datum url_in(PG_FUNCTION_ARGS) {
	char *s = PG_GETARG_CSTRING(0);
	uritype *vardata;
	UriUriA uri;

	parse_uri(s, &uri);
	uriFreeUriMembersA(&uri);

	vardata = (uritype *) cstring_to_text(s);
	PG_RETURN_URI_P(vardata);
}

PG_FUNCTION_INFO_V1(uri_out);
Datum url_out(PG_FUNCTION_ARGS) {
	Datum datum = PG_GETARG_DATUM(0);
	PG_RETURN_CSTRING(TextDatumGetCString(datum));
}
