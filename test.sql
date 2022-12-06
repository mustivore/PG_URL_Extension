DROP INDEX IF EXISTS url_idx_test;
DROP TABLE IF EXISTS test;
DROP EXTENSION IF EXISTS url;

CREATE EXTENSION url;
CREATE TABLE test(id serial, url_db url);
CREATE INDEX url_idx_test ON test(url_db);

INSERT INTO test(url_db) VALUES ('http://www.google.com');
INSERT INTO test(url_db) VALUES ('https://www.google.com');
INSERT INTO test(url_db) VALUES ('www.google.com'); --should not work
INSERT INTO test(url_db) VALUES ('https://www.coucou.com');
INSERT INTO test(url_db) VALUES ('https://uv.ulb.ac.be/pluginfile.php/3623050/mod_resource/content/1/Project2%20-%20DB%20Extension.pdf');
-- EXPLAIN SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and equals(t1.url_db, t2.url_db);
EXPLAIN SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and sameHost(t1.url_db, t2.url_db);