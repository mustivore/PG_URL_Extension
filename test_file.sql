DROP TABLE IF EXISTS test; --reset table

DROP EXTENSION IF EXISTS url; --update extension in case of changes
CREATE EXTENSION IF NOT EXISTS url;


CREATE TABLE test (
	id int PRIMARY KEY,
	link url);

INSERT INTO test(id, link) VALUES (1, 'http://www.google.com');
INSERT INTO test(id, link) VALUES (2, 'https://www.google.com');
INSERT INTO test(id, link) VALUES (3, 'https://www.google.com:443');
INSERT INTO test(id, link) VALUES (4, 'https://www.ulb.be');
INSERT INTO test(id, link) VALUES (5, 'https://www.ulb.ac.be');
INSERT INTO test(id, link) VALUES (6, 'https://cs.ulb.ac.be/public/teaching/infoh415#exercices');
INSERT INTO test(id, link) VALUES (7, 'https://uv.ulb.ac.be/pluginfile.php/3623050/mod_resource/content/1/Project2%20-%20DB%20Extension.pdf');
INSERT INTO test(id, link) VALUES (8, 'https://docs.github.com/en/get-started/getting-started-with-git/about-remote-repositories#cloning-with-https-urls');
INSERT INTO test(id, link) VALUES (9, 'https://www.ulb.be:443/?q=email@address.com'); --check if '@' char doesn't screw up


SELECT * from test;


SELECT 	id, 
		getAuthority(link), 
		getDefaultPort(link), 
		getFile(link), 
		getHost(link), 
		getPath(link), 
		getPort(link), 
		getProtocol(link), 
		getQuery(link), 
		getRef(link), 
		getUserInfo(link) 
FROM test 
WHERE 
		--equals(link, 'https://www.google.com:443')
		--same_file(link, 'https://cs.ulb.ac.be/public/teaching/infoh415#project')
		sameHost(link, 'https://www.ulb.be')
		;
