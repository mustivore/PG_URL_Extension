-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit

CREATE TYPE url;

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

--base36_out(base36)
CREATE OR REPLACE FUNCTION url_out(url)
RETURNS cstring
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE url (
	INPUT          = url_in,
	OUTPUT         = url_out,
	LIKE           = varchar,
	CATEGORY       = 'N'
);

CREATE OR REPLACE FUNCTION get_protocol(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE OR REPLACE FUNCTION get_default_port(url) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE OR REPLACE FUNCTION get_authority(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';    

CREATE OR REPLACE FUNCTION get_user_info(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE OR REPLACE FUNCTION get_host(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 