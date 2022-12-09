-- Authors:
--Ayadi Mustapha 000545704
--Soumaya Izmar 000546128
--Guillaume Wafflard 000479740
--Diaz Y Suarez Esteban 000476205

DROP INDEX IF EXISTS url_idx_test;
DROP TABLE IF EXISTS test;
DROP EXTENSION IF EXISTS url;

CREATE EXTENSION url;
CREATE TABLE test(id serial, url_db url);
CREATE INDEX url_idx_test ON test(url_db);

-- -- INSERT INTO test(url_db) VALUES ('http://0');
INSERT INTO test(url_db) VALUES ('https://www.google.com');
INSERT INTO test(url_db) VALUES ('https://www.coucou.com');
INSERT INTO test(url_db) VALUES ('https://uv.ulb.ac.be/enrol/index.php?id=106500');
INSERT INTO test(url_db) VALUES ('https://uv.ulb.ac.be/pluginfile.php/3623050/mod_resource/content/1/Project2%20-%20DB%20Extension.pdf');

-- We can see thanks to the query plan that the index is triggered while computing equals,sameHost and sameFile

EXPLAIN SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and equals(t1.url_db, t2.url_db); -- 0 rows
EXPLAIN SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and sameHost(t1.url_db, t2.url_db);
EXPLAIN SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and sameFile(t1.url_db, t2.url_db);

SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and equals(t1.url_db, t2.url_db);
SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and sameHost(t1.url_db, t2.url_db);
SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and sameFile(t1.url_db, t2.url_db);

INSERT INTO test(url_db) VALUES ('https://www.google.com');
SELECT t1.url_db FROM test t1, test t2 where t1.id <> t2.id and equals(t1.url_db, t2.url_db); -- 'https://www.google.com' is displayed twice
DROP INDEX url_idx_test;
DROP TABLE test;
DROP EXTENSION url;
