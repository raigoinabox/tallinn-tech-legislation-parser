# r-cran-rpostgresql required (on Ubuntu 18.04)
# r-cran-xtable required (on Ubuntu 18.04)
# r-cran-igraph required (on Ubuntu 18.04)

library(DBI)
library(xtable)
library(RPostgreSQL)
library(igraph)

source("dtf.r")
source("dao.r")
source("topological_structure.r")
source("doing_business.r")

StandardError <- function(x) {
  sd(x, na.rm = TRUE) / sqrt(length(x))
}

firstup <- function(x) {
  substr(x, 1, 1) <- toupper(substr(x, 1, 1))
  x
}

CatData <- function(...) {
  args <- list(...)
  value <- tail(args, 1)[[1]]
  if (is.numeric(value)) {
    value <- signif(value, 3)
  }
  cat("\\newcommand*\\", paste0(as.vector(
    head(args,-1), mode = "character"
  )), "{", value, "}\n", sep = "")
}

Comment <- function(...) {
  cat("%", ..., "\n")
}

FileName <- function(...) {
  paste0(
    "/home/raigo/Dokumendid/magistri_lõputöö/thesis_2/img/",
    paste(..., sep = "_")
  )
}

Main <- function() {
  pdf.options(
    width = 6,
    height = 6
  )

  db.driver <- dbDriver("PostgreSQL")
  db.conn <-
    dbConnect(db.driver, dbname = "tallinn_tech_legislation_parser", host = "")
  on.exit(dbDisconnect(db.conn))
    
  directory <- file.path("/home/raigo/Dokumendid/magistri_lõputöö/thesis_2/img")
  PrintTopologicalStructure(db.conn, directory);
  PrintDoingBusinessAnalysis(db.conn, directory);
}

Main()
