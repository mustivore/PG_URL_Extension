DROP TABLE IF EXISTS test;
DROP EXTENSION IF EXISTS url;

CREATE EXTENSION url;
CREATE TABLE test(id int, url url);

INSERT INTO test(id, url) VALUES (1, 'http://www.google.com');
INSERT INTO test(id, url) VALUES (2, 'https://www.google.com');
INSERT INTO test(id, url) VALUES (3, 'www.google.com'); --should not work
INSERT INTO test(id, url) VALUES (4, 'https://www.coucou.com');
INSERT INTO test(id, url) VALUES (5, 'https://www.google.com/search?q=url+database&oq=URLS+data&aqs=chrome.2.69i57j0i19i512j0i19i22i30l8.7870j0j7&sourceid=chrome&ie=UTF-8')
SELECT url FROM test as t where get_protocol(t.url)='https';