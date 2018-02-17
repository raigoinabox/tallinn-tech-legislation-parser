DROP TABLE IF EXISTS web_cache;
DROP TABLE IF EXISTS complexity_results;
DROP TABLE IF EXISTS dbu_results;

CREATE TABLE web_cache (
	url TEXT NOT NULL UNIQUE,
	content TEXT NOT NULL
);

CREATE TABLE complexity_results (
	country TEXT NOT NULL,
	year INTEGER NOT NULL,
	dbu_category TEXT NOT NULL,
	complexity INTEGER NOT NULL
);

CREATE TABLE dbu_results (
	country TEXT NOT NULL,
	year INTEGER NOT NULL,
	dbu_category TEXT NOT NULL,
	dtf INTEGER NOT NULL
);