# r-cran-rsqlite required (on Ubuntu 16.04)

library(DBI)

Category.run <- function(category.data) {
  category.data$complexity <- scale(category.data$complexity)
  category.data$dtf <- scale(category.data$dtf)
  cor(category.data$complexity, category.data$dtf)
}

Algorithm.run <- function(algorithm.data) {
  as.vector(by(algorithm.data, algorithm.data$dbu_category, Category.run))
}

Main <- function() {
  db.conn <- dbConnect(RSQLite::SQLite(), "../data.db")
  results <- dbGetQuery(
    db.conn,
    paste(
      "select core.year, core.dbu_category, complexity, dtf, algorithm",
      "from complexity_results core",
      "join dbu_results dbre on core.year = dbre.year and core.dbu_category = dbre.dbu_category"
    )
  )
  by(results, results$algorithm, Algorithm.run)
}

print(Main())
