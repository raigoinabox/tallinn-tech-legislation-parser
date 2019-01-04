GetCorrelationsToDtfByAlgorithm <-
  function(complexities) {
    correlations <-
      t(do.call(
        rbind,
        by(complexities,
           complexities$algorithm,
           function(data)
             by(data,
                data$dbu_category,
                GetCorrelationToDtf))
      ))
    rbind(
      correlations,
      "Total correlation" = by(complexities,
         complexities$algorithm,
         GetCorrelationToDtf)
    )
  }

GetCorrelationToDtf <- function(data) {
  cor(data$complexity, data$dtf)
}