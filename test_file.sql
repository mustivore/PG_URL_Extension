DROP TABLE IF EXISTS test_url; --reset table

DROP EXTENSION IF EXISTS url; --update extension in case of changes
CREATE EXTENSION IF NOT EXISTS url;


CREATE TABLE test_url (
	id int PRIMARY KEY,
	link url);

INSERT INTO test_url(id, link) VALUES (1, 'http://www.google.com');
INSERT INTO test_url(id, link) VALUES (2, 'https://www.google.com');
INSERT INTO test_url(id, link) VALUES (3, 'https://www.google.com:443');
INSERT INTO test_url(id, link) VALUES (4, 'https://www.ulb.be');
INSERT INTO test_url(id, link) VALUES (5, 'https://www.ulb.ac.be');
INSERT INTO test_url(id, link) VALUES (6, 'https://cs.ulb.ac.be/public/teaching/infoh415#exercices');
INSERT INTO test_url(id, link) VALUES (7, 'https://uv.ulb.ac.be/pluginfile.php/3623050/mod_resource/content/1/Project2%20-%20DB%20Extension.pdf');
INSERT INTO test_url(id, link) VALUES (8, 'https://docs.github.com/en/get-started/getting-started-with-git/about-remote-repositories#cloning-with-https-urls');
INSERT INTO test_url(id, link) VALUES (9, 'https://www.ulb.be:443/?q=email@address.com'); --check if '@' char doesn't screw up
INSERT INTO test_url(id, link) VALUES (10, make_url_cont_spec('https://www.coucou.be/hello', 'https://goodbye.com/seeU')); --spec replace context
INSERT INTO test_url(id, link) VALUES (11, make_url_cont_spec('https://www.coucou.be/hello', 'resources/images/goodbye.png')); --add spec path to context path
INSERT INTO test_url(id, link) VALUES (12, make_url_cont_spec('https://www.coucou.be/hello', '/blog/data/resources/images/goodbye.png')); --keep context host but replace path
INSERT INTO test_url(id, link) VALUES (13, make_url_prot_host_port_file('https', 'www.ulb.be', 443, 'index.html' ));
INSERT INTO test_url(id, link) VALUES (14, make_url_prot_host_file('https', 'www.ulb.be', 'index.html' ));
INSERT INTO test_url(id, link) VALUES (15, 'https://www.ulb.ac.be');
INSERT INTO test_url(id, link) VALUES (16, 'https://www.google.com/search?q=postgresql&oq=postgresql&aqs=chrome..69i57j69i59l3j69i60j69i61j69i60l2.5881j0j1&sourceid=chrome&ie=UTF-8');
INSERT INTO test_url(id, link) VALUES (17, 'https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top');
INSERT INTO test_url(id, link) VALUES (18, 'https://user:password@www.example.com:42/forum/questions/?tag=networking&order=newest#top');

INSERT INTO test_url(id, link) VALUES (19, 
					make_url_prot_host_port_file(
						get_protocol('https://user:password@www.example.com:42/forum/questions/?tag=networking&order=newest#top')::cstring,
						get_host('https://user:password@www.example.com:42/forum/questions/?tag=networking&order=newest#top')::cstring,
						--get_port('https://user:password@www.example.com:42/forum/questions/?tag=networking&order=newest#top'),
						443,
						get_file('https://user:password@www.example.com:42/forum/questions/?tag=networking&order=newest#top')::cstring));


SELECT * from test_url;

SELECT 	id, 
		--get_protocol(link), 
		--get_authority(link), 
		get_host(link), 
		--get_port(link), 
		--get_default_port(link),
		get_file(link), 
		get_path(link), 
		get_query(link),
		--get_ref(link), 
		get_user_info(link) 
FROM test_url 
WHERE 
		get_query(link) IS NOT NULL
		--equals(link, 'https://www.google.com:443')
		--same_file(link, 'https://cs.ulb.ac.be/public/teaching/infoh415#project')
		--same_host(link, 'https://www.google.com')
;
