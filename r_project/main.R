# r-cran-rsqlite required (on Ubuntu 16.04)

library(DBI)

GetCorrelation <- function(data) {
  data <- as.data.frame(scale(data[c("complexity", "dtf")]))
  cor(data$complexity, data$dtf)
}

GetCorrelationByDbuCategory <- function(data) {
  by(data, data$dbu_category, GetCorrelation)
}

GetComplexityOnly <- function(my.data) {
  my.data[c("complexity")]
}

SplitByYear <- function(my.data) {
  by(my.data, my.data$year, GetComplexityOnly)
}

Main <- function() {
  db.conn <- dbConnect(RSQLite::SQLite(), "../data.db")
  complexities <- dbGetQuery(
    db.conn,
    paste(
      "select core.dbu_category, complexity, dtf, algorithm, core.year",
      "from complexity_results core",
      "join dbu_results dbre on core.year = dbre.year",
      "and core.dbu_category = dbre.dbu_category",
      "and core.country = dbre.country",
      "where core.country = 'GB'"
    )
  )
  
  starting.a.business.complexities <-
    complexities[complexities$dbu_category == 'Starting a Business', ]
  starting.a.business.complexities <-
    by(
      starting.a.business.complexities,
      starting.a.business.complexities$algorithm,
      SplitByYear
    )
  print(do.call(cbind, starting.a.business.complexities))
  
  correlations <-
    t(do.call(
      rbind,
      by(
        complexities,
        complexities$algorithm,
        GetCorrelationByDbuCategory
      )
    ))
  print(correlations)
  print(colMeans(correlations, na.rm = TRUE))
  print(c(by(
    complexities, complexities$algorithm, GetCorrelation
  )))
}

Main()
