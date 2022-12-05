# PostgreSQL Extension for URL

This extension implement the Java Class URL into PostgreSQL. 
See doc : [https://docs.oracle.com/javase/8/docs/api/java/net/URL.html#URL-java.net.URL-java.lang.String]

Install the librairies (change 'XX' with your postgres version) : 
```
sudo apt install postgresql-server-dev-XX
```

Install the extension :
```
make
sudo make install 
```
This will install the extension ___url--1.0.sql___ in the directory ```/usr/share/postgresql/XX/extension/```
