# r-cran-rsqlite required (on Ubuntu 16.04)

library(DBI)

main <- function() {
  conn <- dbConnect(RSQLite::SQLite(), "../data.db")
  results <- dbGetQuery(
    conn,
    paste(
      "select core.year, core.dbu_category, complexity, dtf",
      "from complexity_results core",
      "join dbu_results dbre on core.year = dbre.year and core.dbu_category = dbre.dbu_category"
    )
  )
  categories <- unique(results["dbu_category"])
  years <- unique(results["year"])
  compl_matr <- matrix(
    data = NA,
    nrow = nrow(years),
    ncol = nrow(categories),
    dimnames = list(unlist(years), unlist(categories))
  )
  dbu_matr <- matrix(
    data = NA,
    nrow = nrow(years),
    ncol = nrow(categories),
    dimnames = list(unlist(years), unlist(categories))
  )
  
  for (i in 1:nrow(results)) {
    row <- results[i,]
    compl_matr[as.character(row$year), row$dbu_category] <- row$complexity
    dbu_matr[as.character(row$year), row$dbu_category] <- row$dtf
  }
  
  # compl_matr <- compl_matr[complete.cases(compl_matr),]
  # dbu_matr <- dbu_matr[complete.cases(dbu_matr),]
  compl_matr
  cor_matrix <- cor(compl_matr, dbu_matr, use = "complete.obs")
  diag(cor_matrix)
}

main()