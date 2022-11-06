-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION base36" to load this file. \quit

CREATE OR REPLACE FUNCTION base36_in(cstring)
RETURNS url
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

--base36_out(base36)
CREATE OR REPLACE FUNCTION url_out(cstring)
RETURNS cstring
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE url (
	INPUT          = url_in,
	OUTPUT         = url_out,
	LIKE           = varchar,
	CATEGORY       = 'N'
);
COMMENT ON TYPE url IS 'bigint written in base36: [0-9A-Z]+';


