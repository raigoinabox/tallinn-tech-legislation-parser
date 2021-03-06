drop table if exists legal_act_references;
drop table if exists legal_act_complexities;
drop table if exists legal_act_section_texts;
drop table if exists legal_act_sections;
drop table if exists legal_acts;
drop table if exists doing_business_results;
drop table if exists doing_business_laws;
drop table if exists doing_business_categories;
drop table if exists doing_business_law_groups;
drop table complexity_algorithms;

create table if not exists web_cache (
	url text not null unique,
	content text not null,
	cache_time timestamp not null default now()
);

create table if not exists sections (
	row_id serial primary key,
	country text not null,
	year integer not null,
	dbu_category text not null,
	document_id integer not null,
	section_id text not null,
	section_text text not null
);

create table if not exists section_connections (
	section_from_id integer not null references sections(row_id),
	section_to text not null
);

create table complexity_algorithms (
	row_id serial primary key,
	name text not null
);
insert into complexity_algorithms (row_id, name) values
(1, 'Average Vertex Degree'),
(2, 'Average Path Length'),
(3, 'Diameter'),
(4, 'Global Clustering Coefficient'),
(5, 'Kütt Complexity');

create table if not exists complexity_results (
	country text not null,
	year integer not null,
	dbu_category text not null,
	complexity real not null,
	algorithm integer not null references complexity_algorithms(row_id)
);

create table doing_business_law_groups (
	row_id serial primary key,
	name text not null unique
);

create table doing_business_categories (
	law_group_id int references doing_business_law_groups(row_id),
	name text not null
);

create table doing_business_laws (
	law_group_id int not null references doing_business_law_groups(row_id),
	country text not null,
	url text not null
);

create table doing_business_results (
	country text not null,
	year integer not null,
	dbu_category text not null,
	dtf real not null
);
insert into doing_business_results (country, year, dbu_category, dtf)
values
('GB', 2018, 'Starting a Business', 94.58),
('GB', 2017, 'Starting a Business', 94.58),
('GB', 2016, 'Starting a Business', 94.57),
('GB', 2015, 'Starting a Business', 91.23),
('GB', 2014, 'Starting a Business', 89.85),
('GB', 2013, 'Starting a Business', 89.8),
('GB', 2012, 'Starting a Business', 89.8),
('GB', 2011, 'Starting a Business', 89.79),
('GB', 2010, 'Starting a Business', 90.04),
('GB', 2009, 'Starting a Business', 90.04),
('GB', 2008, 'Starting a Business', 90.04),
('GB', 2007, 'Starting a Business', 90.05),
('GB', 2006, 'Starting a Business', 89.67),
('GB', 2005, 'Starting a Business', 89.39),
('GB', 2004, 'Starting a Business', 89.38),
('GB', 2018, 'Dealing with Construction Permits', 80.39),
('GB', 2017, 'Dealing with Construction Permits', 80.34),
('GB', 2016, 'Dealing with Construction Permits', 80.29),
('GB', 2015, 'Dealing with Construction Permits', 80.16),
('GB', 2014, 'Dealing with Construction Permits', 86.85),
('GB', 2013, 'Dealing with Construction Permits', 86.96),
('GB', 2012, 'Dealing with Construction Permits', 86.92),
('GB', 2011, 'Dealing with Construction Permits', 85.79),
('GB', 2010, 'Dealing with Construction Permits', 85.84),
('GB', 2009, 'Dealing with Construction Permits', 70.54),
('GB', 2008, 'Dealing with Construction Permits', 70.53),
('GB', 2007, 'Dealing with Construction Permits', 70.43),
('GB', 2006, 'Dealing with Construction Permits', 70.4),
('GB', 2018, 'Getting Electricity', 93.29),
('GB', 2017, 'Getting Electricity', 93.29),
('GB', 2016, 'Getting Electricity', 93.29),
('GB', 2015, 'Getting Electricity', 93.28),
('GB', 2014, 'Getting Electricity', 90.78),
('GB', 2013, 'Getting Electricity', 81.41),
('GB', 2012, 'Getting Electricity', 80.82),
('GB', 2011, 'Getting Electricity', 81.09),
('GB', 2010, 'Getting Electricity', 81.1),
('GB', 2018, 'Registering Property', 74.51),
('GB', 2017, 'Registering Property', 74.11),
('GB', 2016, 'Registering Property', 74.5),
('GB', 2015, 'Registering Property', 74.41),
('GB', 2014, 'Registering Property', 72.43),
('GB', 2013, 'Registering Property', 71.32),
('GB', 2012, 'Registering Property', 71.28),
('GB', 2011, 'Registering Property', 71.23),
('GB', 2010, 'Registering Property', 71.27),
('GB', 2009, 'Registering Property', 69.12),
('GB', 2008, 'Registering Property', 69.06),
('GB', 2007, 'Registering Property', 68.95),
('GB', 2006, 'Registering Property', 68.92),
('GB', 2005, 'Registering Property', 68.8),
('GB', 2018, 'Getting Credit', 75),
('GB', 2017, 'Getting Credit', 75),
('GB', 2016, 'Getting Credit', 75),
('GB', 2015, 'Getting Credit', 75),
('GB', 2014, 'Getting Credit', 75),
('GB', 2013, 'Getting Credit', 100),
('GB', 2012, 'Getting Credit', 100),
('GB', 2011, 'Getting Credit', 100),
('GB', 2010, 'Getting Credit', 100),
('GB', 2009, 'Getting Credit', 100),
('GB', 2008, 'Getting Credit', 100),
('GB', 2007, 'Getting Credit', 100),
('GB', 2006, 'Getting Credit', 100),
('GB', 2005, 'Getting Credit', 100),
('GB', 2018, 'Protecting Minority Investors', 75),
('GB', 2017, 'Protecting Minority Investors', 75),
('GB', 2016, 'Protecting Minority Investors', 75),
('GB', 2015, 'Protecting Minority Investors', 75),
('GB', 2014, 'Protecting Minority Investors', 75),
('GB', 2013, 'Protecting Minority Investors', 80),
('GB', 2012, 'Protecting Minority Investors', 80),
('GB', 2011, 'Protecting Minority Investors', 80),
('GB', 2010, 'Protecting Minority Investors', 80),
('GB', 2009, 'Protecting Minority Investors', 80),
('GB', 2008, 'Protecting Minority Investors', 80),
('GB', 2007, 'Protecting Minority Investors', 80),
('GB', 2006, 'Protecting Minority Investors', 80),
('GB', 2018, 'Paying Taxes', 86.7),
('GB', 2017, 'Paying Taxes', 86.63),
('GB', 2016, 'Paying Taxes', 86.25),
('GB', 2015, 'Paying Taxes', 90.62),
('GB', 2014, 'Paying Taxes', 90.08),
('GB', 2013, 'Paying Taxes', 89.9),
('GB', 2012, 'Paying Taxes', 89.36),
('GB', 2011, 'Paying Taxes', 89.38),
('GB', 2010, 'Paying Taxes', 89.96),
('GB', 2009, 'Paying Taxes', 90.54),
('GB', 2008, 'Paying Taxes', 90.38),
('GB', 2007, 'Paying Taxes', 90.35),
('GB', 2006, 'Paying Taxes', 90.32),
('GB', 2018, 'Trading across Borders', 93.76),
('GB', 2017, 'Trading across Borders', 93.76),
('GB', 2016, 'Trading across Borders', 93.76),
('GB', 2015, 'Trading across Borders', 93.76),
('GB', 2014, 'Trading across Borders', 88.24),
('GB', 2013, 'Trading across Borders', 88.16),
('GB', 2012, 'Trading across Borders', 88),
('GB', 2011, 'Trading across Borders', 87.78),
('GB', 2010, 'Trading across Borders', 85.68),
('GB', 2009, 'Trading across Borders', 84.78),
('GB', 2008, 'Trading across Borders', 85.24),
('GB', 2007, 'Trading across Borders', 84.99),
('GB', 2006, 'Trading across Borders', 84.82),
('GB', 2018, 'Enforcing Contracts', 68.69),
('GB', 2017, 'Enforcing Contracts', 68.69),
('GB', 2016, 'Enforcing Contracts', 68.69),
('GB', 2015, 'Enforcing Contracts', 70.19),
('GB', 2014, 'Enforcing Contracts', 67.41),
('GB', 2013, 'Enforcing Contracts', 67.41),
('GB', 2012, 'Enforcing Contracts', 68.45),
('GB', 2011, 'Enforcing Contracts', 68.45),
('GB', 2010, 'Enforcing Contracts', 66.36),
('GB', 2009, 'Enforcing Contracts', 66.23),
('GB', 2008, 'Enforcing Contracts', 66.23),
('GB', 2007, 'Enforcing Contracts', 66.23),
('GB', 2006, 'Enforcing Contracts', 66.94),
('GB', 2005, 'Enforcing Contracts', 68.44),
('GB', 2004, 'Enforcing Contracts', 70.31),
('GB', 2018, 'Resolving Insolvency', 80.24),
('GB', 2017, 'Resolving Insolvency', 82.04),
('GB', 2016, 'Resolving Insolvency', 82.04),
('GB', 2015, 'Resolving Insolvency', 82.04),
('GB', 2014, 'Resolving Insolvency', 82.04),
('GB', 2013, 'Resolving Insolvency', 95.33),
('GB', 2012, 'Resolving Insolvency', 95.33),
('GB', 2011, 'Resolving Insolvency', 95.33),
('GB', 2010, 'Resolving Insolvency', 90.66),
('GB', 2009, 'Resolving Insolvency', 90.66),
('GB', 2008, 'Resolving Insolvency', 91.02),
('GB', 2007, 'Resolving Insolvency', 91.68),
('GB', 2006, 'Resolving Insolvency', 91.77),
('GB', 2005, 'Resolving Insolvency', 92.39),
('GB', 2004, 'Resolving Insolvency', 92.12);

create table legal_acts (
	row_id serial primary key,
	title text,
	url text not null unique,
	language text not null
);
insert into legal_acts (title, url, language) values
('Law of Obligations Act 2018', 'https://www.riigiteataja.ee/akt/122032018004', 'est'),
('Töölepingu seadus', 'https://www.riigiteataja.ee/akt/126102018028', 'est'),
('Karistusseadustik', 'https://www.riigiteataja.ee/akt/107122018021', 'est'),
('Äriseadustik', 'https://www.riigiteataja.ee/akt/117112017022', 'est'),
('Tsiviilkohtumenetluse seadustik', 'https://www.riigiteataja.ee/akt/104072017031', 'est'),
('Kriminaalmenetluse seadustik', 'https://www.riigiteataja.ee/akt/107122018023', 'est'),
('Liiklusseadus', 'https://www.riigiteataja.ee/akt/122062018010', 'est'),
('Eesti Haigekassa tervishoiuteenuste loetelu', 'https://www.riigiteataja.ee/akt/121032018005', 'est'),
('Veeseadus', 'https://www.riigiteataja.ee/akt/104072017051', 'est'),
('Asjaõigusseadus', 'https://www.riigiteataja.ee/akt/129062018006', 'est'),
('Tsiviilseadustiku üldosa seadus', 'https://www.riigiteataja.ee/akt/120042017021', 'est'),
('Tulumaksuseadus', 'https://www.riigiteataja.ee/akt/129062018049', 'est'),
('Riigilõivuseadus', 'https://www.riigiteataja.ee/akt/129062018041', 'est'),
('Eesti Vabariigi põhiseadus', 'https://www.riigiteataja.ee/akt/115052015002', 'est');

create table legal_act_complexities (
	act_id int not null references legal_acts(row_id),
	complexity real not null
);

create table legal_act_sections (
	row_id serial primary key,
	act_id int not null references legal_acts(row_id),
	section_number text not null
);

create table legal_act_section_texts (
	section_id int not null references legal_act_sections(row_id),
	section_text text not null
);

create table legal_act_references (
	section_id int not null references legal_act_sections(row_id),
	reference text not null
);
