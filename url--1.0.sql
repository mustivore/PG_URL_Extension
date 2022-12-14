-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION url" to load this file. \quit
-- Authors:
--Ayadi Mustapha 000545704
--Soumaya Izmar 000546128
--Guillaume Wafflard 000479740
--Diaz Y Suarez Esteban 000476205
CREATE TYPE url;

CREATE OR REPLACE FUNCTION url_in(cstring)
RETURNS url
AS '$libdir/url'
LANGUAGE C IMMUTABLE STRICT;

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

CREATE OR REPLACE FUNCTION make_url_prot_host_port_file(cstring, cstring ,integer, cstring) RETURNS url
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';
    
CREATE OR REPLACE FUNCTION make_url_prot_host_file(cstring, cstring, cstring) RETURNS url
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE OR REPLACE FUNCTION make_url_cont_spec(cstring,cstring) RETURNS url
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

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

CREATE OR REPLACE FUNCTION get_file(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE OR REPLACE FUNCTION get_path(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE OR REPLACE FUNCTION get_query(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE OR REPLACE FUNCTION get_port(url) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE OR REPLACE FUNCTION get_ref(url) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url'; 

CREATE FUNCTION url_cmp(url, url) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_lt(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_le(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_eq(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_ne(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_ge(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION url_gt(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE OPERATOR < (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = >,
    NEGATOR = >=,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = url_lt
);

CREATE OPERATOR <= (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = >=,
    NEGATOR = >,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = url_le
);

CREATE OPERATOR = (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = =,
    NEGATOR = <>,
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES,
    MERGES,
    PROCEDURE = url_eq
);

CREATE OPERATOR <> (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = <>,
    NEGATOR = =,
    RESTRICT = neqsel,
    JOIN = neqjoinsel,
    PROCEDURE = url_ne
);

CREATE OPERATOR >= (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = <=,
    NEGATOR = <,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = url_ge
);

CREATE OPERATOR > (
    LEFTARG = url,
    RIGHTARG = url,
    COMMUTATOR = <,
    NEGATOR = <=,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = url_gt
);

CREATE OPERATOR CLASS btree_url_ops 
DEFAULT FOR TYPE url USING btree 
AS
    OPERATOR        1       < ,
    OPERATOR        2       <= ,
    OPERATOR        3       = ,
    OPERATOR        4       >= ,
    OPERATOR        5       > ,
    FUNCTION        1       url_cmp(url, url);

CREATE FUNCTION equals(url, url)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(=) $2'
  LANGUAGE SQL IMMUTABLE STRICT;

CREATE FUNCTION url_same_file(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';


-- 'http://0' is the smallest URL that we can build. we trigger always triggered because $1 and $2 (url1 & url2)
--  is always bigger or equals than 'http://0' 
CREATE FUNCTION sameFile(url, url)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(>=) url_in(''http://0'') AND $2 OPERATOR(>=) url_in(''http://0'') AND url_same_file($1,$2)'
  LANGUAGE SQL IMMUTABLE;

CREATE FUNCTION url_same_host(url, url) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/url';

CREATE FUNCTION sameHost(url, url)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(>=) url_in(''http://0'') AND $2 OPERATOR(>=) url_in(''http://0'') AND url_same_host($1,$2)'
  LANGUAGE SQL IMMUTABLE;