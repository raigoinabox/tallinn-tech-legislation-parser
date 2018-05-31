# r-cran-rsqlite required (on Ubuntu 16.04)

library(DBI)

Category.run <- function(category.data) {
  category.data <- category.data[c("complexity", "dtf")]
  category.data <- as.data.frame(scale(category.data))
  cor(category.data$complexity, category.data$dtf)
}

Algorithm.run <- function(algorithm.data) {
  by(algorithm.data, algorithm.data$dbu_category, Category.run)
}

Main <- function() {
  db.conn <- dbConnect(RSQLite::SQLite(), "../data.db")
  results <- dbGetQuery(
    db.conn,
    paste(
      "select core.dbu_category, complexity, dtf, algorithm",
      "from complexity_results core",
      "join dbu_results dbre on core.year = dbre.year and core.dbu_category = dbre.dbu_category"
    )
  )
  do.call(rbind, by(results, results$algorithm, Algorithm.run))
}

print(Main())
