DROP TABLE IF EXISTS web_pages;

CREATE TABLE web_pages (
	url TEXT NOT NULL UNIQUE,
	content TEXT NOT NULL
);
