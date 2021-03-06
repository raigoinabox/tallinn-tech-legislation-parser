/*
 * doing_business.c
 *
 *  Created on: 13. jaan 2018
 *      Author: raigo
 */

#include "doing_business.h"
#include "legal_act.h"

#include <string.h>

static struct law_category_list law_categories;

static struct cstring_vec get_banking_and_credit_laws_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Getting Credit");
    vec_append_old(categories, "Protecting Investors");
    return categories;
}

static struct law_vec get_banking_and_credit_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Bank of England Act 1998
    vec_append_old(laws, leg_init_c("ukpga", "1998", "11"));
    // Companies Act 2006
    vec_append_old(laws, leg_init_c("ukpga", "2006", "46"));
    // Representation of the People (England and Wales)(Amendment) Regulations 2002
    vec_append_old(laws, leg_init_c("uksi", "2002", "1871"));
    // Financial Services and Markets Act 2000
    vec_append_old(laws, leg_init_c("ukpga", "2000", "8"));
    // Consumer Credit Act 2006
    vec_append_old(laws, leg_init_c("ukpga", "2006", "14"));
    // Data Protection Act 1998
    vec_append_old(laws, leg_init_c("ukpga", "1998", "29"));
    // Consumer Credit Act 1974 (Commencement No. 10) Order 1989
    vec_append_old(laws, leg_init_c("ukpga", "1974", "39"));
    return laws;
}

static struct cstring_vec get_bankruptcy_and_collateral_laws_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Closing a Business");
//	string_list_append(&categories, "Getting Credit - Legal Rights");
    vec_append_old(categories, "Getting Credit");
    return categories;
}

static struct law_vec get_bankruptcy_and_collateral_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Insolvency Act 1986
    vec_append_old(laws, leg_init_c("ukpga", "1986", "45"));
    // Enterprise Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "40"));
    // Insolvency (No. 2) Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "12"));
    // Financial Collateral Arrangements (No.2) Regulations 2003
    vec_append_old(laws, leg_init_c("uksi", "2003", "3226"));
    // Insolvency Act 2000
    vec_append_old(laws, leg_init_c("ukpga", "2000", "39"));
    // Social Security (Mortgage Interest Payments) Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "33"));
    // The Insurers (Reorganisation and Winding Up) Regulations 2003
    vec_append_old(laws, leg_init_c("uksi", "2003", "1102"));
    // Companies Act 2006
    vec_append_old(laws, leg_init_c("ukpga", "2006", "46"));
    // Bankruptcy (Scotland) Act 1993
    vec_append_old(laws, leg_init_c("ukpga", "1993", "6"));
    // The Cross-Border Insolvency Regulations 2006
    vec_append_old(laws, leg_init_c("uksi", "2006", "1030"));
    return laws;
}

static struct cstring_vec get_civil_and_procedure_codes_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Enforcing Contracts");
    vec_append_old(categories, "Closing a Business");
    vec_append_old(categories, "Protecting Investors");
    return categories;
}

static struct law_vec get_civil_and_procedure_codes()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Arbitration Act 1996
    vec_append_old(laws, leg_init_c("ukpga", "1996", "23"));
    // Civil Procedure Act 1997
    vec_append_old(laws, leg_init_c("ukpga", "1997", "12"));
    // Civil Procedure Rules - not a concrete law
    return laws;
}

static struct cstring_vec get_commercial_and_company_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Starting a Business");
    vec_append_old(categories, "Protecting Investors");
    vec_append_old(categories, "Closing a Business");
    return categories;
}

static struct law_vec get_commercial_and_company_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Coal Industry Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "3"));
    // Coal Industry Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "17"));
    // Coal Industry Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "21"));
    // Companies Act 2006
    vec_append_old(laws, leg_init_c("ukpga", "2006", "46"));
    // Enterprise Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "40"));
    // Contracts (Applicable Law) Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "36"));
    // Copyright, Designs and Patents Act 1988
    vec_append_old(laws, leg_init_c("ukpga", "1988", "48"));
    // Companies (Audit, Investigations and Community Enterprise) Act 2004
    vec_append_old(laws, leg_init_c("ukpga", "2004", "27"));
    // Companies Act 1989
    vec_append_old(laws, leg_init_c("ukpga", "1989", "40"));
    // Company and Business Names (Chamber of Commerce, Etc.) Act 1999
    vec_append_old(laws, leg_init_c("ukpga", "1999", "19"));
    // Sale of Goods (Amendment) Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "32"));
    // Sale and Supply of Goods Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "35"));
    // The Limited Liability Partnerships (Fees) Regulations 2004
    vec_append_old(laws, leg_init_c("uksi", "2004", "2620"));
    // British Steel Act 1988
    vec_append_old(laws, leg_init_c("ukpga", "1988", "35"));
    // Trade Marks Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "26"));
    return laws;
}

static struct cstring_vec get_labor_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Employing Workers");
    return categories;
}

static struct law_vec get_labor_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Trade Union and Labour Relations (Consolidation) Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "52"));
    // Trade Union Reform and Employment Rights Act 1993
    vec_append_old(laws, leg_init_c("ukpga", "1993", "19"));
    // Statutory Sick Pay Act 1991
    vec_append_old(laws, leg_init_c("ukpga", "1991", "3"));
    // Statutory Sick Pay Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "2"));
    // Sunday Trading Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "20"));
    // Social Security (Contributions) Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "1"));
    // Social Security Contributions and Benefits Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "4"));
    // Social Security (Mortgage Interest Payments) Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "33"));
    // Social Security (Recovery of Benefits) Act 1997
    vec_append_old(laws, leg_init_c("ukpga", "1997", "27"));
    // Social Security Act 1989
    vec_append_old(laws, leg_init_c("ukpga", "1989", "24"));
    // Social Security Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "27"));
    // Social Security Act 1998 - only available as PDF
    // The Transfer of Undertakings (Protection of Employment) Regulations 2006
    vec_append_old(laws, leg_init_c("uksi", "2006", "246"));
    // The Working Time (Amendment) Regulations 2001
    vec_append_old(laws, leg_init_c("uksi", "2001", "3256"));
    // The Working Time (Amendment) Regulations 2002
    vec_append_old(laws, leg_init_c("uksi", "2002", "3128"));
    // The Working Time (Amendment) Regulations 2003
    vec_append_old(laws, leg_init_c("uksi", "2003", "1684"));
    // The Working Time Regulations 1998
    vec_append_old(laws, leg_init_c("uksi", "1998", "1833"));
    // The Working Time Regulations 1999
    vec_append_old(laws, leg_init_c("uksi", "1999", "3372"));
    // National Minimum Wage (Enforcement Notices) Act 2003 (repealed)
    vec_append_old(laws, leg_init_c("ukpga", "2003", "8"));
    // National Minimum Wage Act 1998
    vec_append_old(laws, leg_init_c("ukpga", "1998", "39"));
    // Social Security (Consequential Provisions) Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "6"));
    // Social Security (Contributions) Act 1991 (repealed 1.7.1992)
    vec_append_old(laws, leg_init_c("ukpga", "1991", "42"));
    // Social Security (Incapacity for Work) Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "18"));
    // Social Security Administration Act 1992 - only as PDF
    // Social Security Contributions (Share Options) Act 2001
    vec_append_old(laws, leg_init_c("ukpga", "2001", "20"));
    // Pensions (Miscellaneous Provisions) Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "7"));
    // Pensions Act 1995 - only as PDF
    // Pensions Act 2004
    vec_append_old(laws, leg_init_c("ukpga", "2004", "35"));
    // Employment Rights (Dispute Resolution) Act 1998
    vec_append_old(laws, leg_init_c("ukpga", "1998", "8"));
    // Employment Rights Act 1996
    vec_append_old(laws, leg_init_c("ukpga", "1996", "18"));
    // Employee Share Schemes Act 2002 (repealed)
    vec_append_old(laws, leg_init_c("ukpga", "2002", "34"));
    // Employment Act 1988
    vec_append_old(laws, leg_init_c("ukpga", "1988", "19"));
    // Employment Act 1989
    vec_append_old(laws, leg_init_c("ukpga", "1989", "38"));
    // Employment Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "38"));
    // Employment Act 2002 - twice
    vec_append_old(laws, leg_init_c("ukpga", "2002", "22"));
    // Employment Relations Act 1999
    vec_append_old(laws, leg_init_c("ukpga", "1999", "26"));
    // Employment Relations Act 2004
    vec_append_old(laws, leg_init_c("ukpga", "2004", "24"));
    return laws;
}


static struct cstring_vec get_land_and_building_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Dealing with Licenses");
    vec_append_old(categories, "Registering Property");
    return categories;
}

static struct law_vec get_land_and_building_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // The Building Regulations 2010
    vec_append_old(laws, leg_init_c("uksi", "2010", "2214"));
    // Farm Land and Rural Development Act 1988
    vec_append_old(laws, leg_init_c("ukpga", "1988", "16"));
    // Land Drainage Act 1991
    vec_append_old(laws, leg_init_c("ukpga", "1991", "59"));
    // Land Drainage Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "25"));
    // Land Registration Act 1997
    vec_append_old(laws, leg_init_c("ukpga", "1997", "2"));
    // Land Registration Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "9"));
    // Regulation of Financial Services (Land Transactions) Act 2005
    vec_append_old(laws, leg_init_c("ukpga", "2005", "24"));
    // Law of Property (Miscellaneous Provisions) Act 1989
    vec_append_old(laws, leg_init_c("ukpga", "1989", "34"));
    // The Town and Country Planning (Local Development) (England) Regulations 2004
    vec_append_old(laws, leg_init_c("uksi", "2004", "2204"));
    // Local Government Act 2003
    vec_append_old(laws, leg_init_c("ukpga", "2003", "26"));
    // Party Wall etc. Act 1996
    vec_append_old(laws, leg_init_c("ukpga", "1996", "40"));
    // Capital Allowances Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "1"));
    // Trusts of Land and Appointment of Trustees Act 1996
    vec_append_old(laws, leg_init_c("ukpga", "1996", "47"));
    return laws;
}

static struct cstring_vec get_security_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Protecting Investors");
    return categories;
}

static struct law_vec get_securities_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // United Kingdom Listing Authority Listing Rules - not law
    // Financial Services and Markets Act 2000
    vec_append_old(laws, leg_init_c("ukpga", "2000", "8"));
    // The Financial Collateral Arrangements (No.2) Regulations 2003
    vec_append_old(laws, leg_init_c("uksi", "2003", "3226"));
    // Enterprise Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "40"));
    return laws;
}

static struct cstring_vec get_tax_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Paying Taxes");
    return categories;
}

static struct law_vec get_tax_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Income and Corporation Taxes Act 1988 - only enacted
    vec_append_old(laws, leg_init_c("ukpga", "1988", "1"));
    // Income Tax (Earnings and Pensions) Act 2003
    vec_append_old(laws, leg_init_c("ukpga", "2003", "1"));
    // Income Tax (Trading and Other Income) Act 2005
    vec_append_old(laws, leg_init_c("ukpga", "2005", "5"));
    // Social Security Contributions (Share Options) Act 2001
    vec_append_old(laws, leg_init_c("ukpga", "2001", "20"));
    // Stamp Duty (Temporary Provisions) Act 1992 (repealed 27.7.1999)
    vec_append_old(laws, leg_init_c("ukpga", "1992", "2"));
    // Tax Credits (Initial Expenditure) Act 1998 (repealed)
    vec_append_old(laws, leg_init_c("ukpga", "1998", "16"));
    // Tax Credits Act 1999 (repealed)
    vec_append_old(laws, leg_init_c("ukpga", "1999", "10"));
    // Social Security Act 1998 - only as PDF
    // Capital Allowances Act 1990 - only enacted
    vec_append_old(laws, leg_init_c("ukpga", "1990", "1"));
    // Tax Credits Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "21"));
    // Value Added Tax Act 1994
    vec_append_old(laws, leg_init_c("ukpga", "1994", "23"));
    return laws;
}

static struct cstring_vec get_trade_law_categories()
{
    struct cstring_vec categories;
    vec_init_old(categories);
    vec_append_old(categories, "Trading across Borders");
    return categories;
}

static struct law_vec get_trade_laws()
{
    struct law_vec laws;
    vec_init_old(laws);
    // Shipping and Trading Interests (Protection) Act 1995
    vec_append_old(laws, leg_init_c("ukpga", "1995", "22"));
    // Merchant Shipping Act 1988 (repealed)
    vec_append_old(laws, leg_init_c("ukpga", "1988", "12"));
    // Import and Export Control Act 1990
    vec_append_old(laws, leg_init_c("ukpga", "1990", "45"));
    // Export Control Act 2002
    vec_append_old(laws, leg_init_c("ukpga", "2002", "28"));
    // Export and Investment Guarantees Act 1991
    vec_append_old(laws, leg_init_c("ukpga", "1991", "67"));
    // Carriage of Goods by Sea Act 1992
    vec_append_old(laws, leg_init_c("ukpga", "1992", "50"));
    return laws;
}

static char* get_leg_url(struct legal_act_id legislation)
{
    struct str_builder url = str_init();
    str_appendc(&url, "http://www.legislation.gov.uk/");
	str_appendc(&url, legislation.type);
    str_appendc(&url, "/");
	str_appendc(&url, legislation.year);
    str_appendc(&url, "/");
	str_appendc(&url, legislation.number);

	return str_content(&url);
}

void dbu_init()
{
    vec_init_old(law_categories);
    struct dbu_law_category law_category =
    {
        .name = "Banking and Credit Laws",
        .dbu_categories = get_banking_and_credit_laws_categories(),
        .laws = get_banking_and_credit_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Bankruptcy and Collateral Laws",
         .dbu_categories = get_bankruptcy_and_collateral_laws_categories(),
          .laws = get_bankruptcy_and_collateral_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Civil Procedure Codes",
         .dbu_categories = get_civil_and_procedure_codes_categories(),
          .laws = get_civil_and_procedure_codes()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Commercial and Company Laws",
         .dbu_categories = get_commercial_and_company_law_categories(),
          .laws = get_commercial_and_company_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Labor Laws",
         .dbu_categories = get_labor_law_categories(),
          .laws = get_labor_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Land and Building Laws",
         .dbu_categories = get_land_and_building_law_categories(),
          .laws = get_land_and_building_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Securities Laws",
         .dbu_categories = get_security_law_categories(),
          .laws = get_securities_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Tax Laws",
         .dbu_categories = get_tax_law_categories(),
          .laws = get_tax_laws()
    };
    vec_append_old(law_categories, law_category);
    law_category = (struct dbu_law_category)
    {
        .name = "Trade Laws",
         .dbu_categories = get_trade_law_categories(),
          .laws = get_trade_laws()
    };
    vec_append_old(law_categories, law_category);

    for (int i = 0; i < vec_length_old(law_categories); i++)
    {
        struct dbu_law_category law_group = vec_elem_old(law_categories, i);
		law_group.law_urls = vec_init(sizeof(char*));
        for (int j = 0; j < vec_length_old(law_group.laws); j++) {
            struct legal_act_id law = vec_elem_old(law_group.laws, j);
			char* law_url = get_leg_url(law);
			vec_append(&law_group.law_urls, &law_url);
        }
    }
}

struct law_category_list get_english_law_categories()
{
    return law_categories;
}
