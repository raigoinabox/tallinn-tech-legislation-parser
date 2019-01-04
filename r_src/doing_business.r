GetScatterPlotToDtf <- function(data) {
  plot(data$complexity,
       data$dtf,
       xlab = "Complexities",
       ylab = "Distance To Frontier")
}

GetScatterPlotToYear <- function(data) {
  plot(
    data$complexity,
    data$year,
    main = paste("Algorithm", levels(factor(data$algorithm))),
    xlab = "Complexities",
    ylab = "Year"
  )
}

GetCorrelationToYear <- function(data) {
  cor(data$complexity, data$year)
}

GetStartingABusinessComplexity <- function(complexities) {
  starting.a.business.complexities <-
    complexities[complexities$dbu_category == 'Starting a Business',]
  
  do.call(
    cbind,
    by(starting.a.business.complexities,
       starting.a.business.complexities$algorithm,
       function(my.data)
         by(my.data, my.data$year, function(my.data)
           my.data[c("complexity")]))
  )
}

GetCorrelationsToYearByAlgorithm <- function(complexities) {
  correlations <-
    t(do.call(
      rbind,
      by(complexities,
         complexities$algorithm,
         function(data)
           by(data, data$dbu_category, GetCorrelationToYear))
    ))
  rbind(
    correlations,
    apply(correlations, 2, mean, na.rm = TRUE),
    apply(correlations, 2, StandardError),
    by(complexities, complexities$algorithm, GetCorrelationToYear)
  )
}

PrintDoingBusinessAnalysis <- function(db.conn, directory) {
  options(
    xtable.tabular.environment = 'tabulary',
    xtable.math.style.negative = TRUE,
    xtable.width = '\\textwidth',
    xtable.floating = FALSE
  )
  
  complexities <- GetComplexities(db.conn)
  
  Comment("Starting a business complexity")
  CatData("StartComp",
          print(
            xtable(GetStartingABusinessComplexity(complexities), align = "RRRRRR"),
            print.results = FALSE,
            comment = FALSE
          ))
  
  Comment("Correlation to each other")
  corr.other <- cor(do.call(
    cbind,
    by(complexities, complexities$algorithm, function(data)
      data[c("complexity")])
  ))
  CatData("CorrOther",
          print(
            xtable(corr.other, align = strrep("R", ncol(corr.other) + 1)),
            print.results = FALSE,
            comment = FALSE,
            hline.after = c(-1, 0, seq_len(nrow(corr.other)))
          ))
  
  dtf.corr <- GetCorrelationsToDtfByAlgorithm(complexities)
  Comment("Correlation to dtf")
  CatData("DtfCorrelation",
          print(
            xtable(dtf.corr, align = strrep("R", ncol(dtf.corr) + 1)),
            print.results = FALSE,
            comment = FALSE,
            hline.after = c(-1, 0, seq_len(nrow(dtf.corr)))
          ))
  # print("Correlation to year")
  # print(GetCorrelationsToYearByAlgorithm(complexities))
  by(complexities,
     complexities$algorithm,
     GetScatterPlotToDtf)
  alg.split <- split(complexities, complexities$algorithm)
  latex.labels <-
    c("algoOne", "algoTwo", "algoThree", "algoFour", "algoFive")
  mapply(function(index, alg.table) {
    pdf(file = file.path(directory, paste0("algo_", index, "_dtf.pdf")))
    plot(alg.table$complexity,
         alg.table$dtf,
         xlab = "Complexities",
         ylab = "Distance To Frontier")
    dev.off()
  }, seq_along(alg.split), alg.split)
  # by(complexities, complexities$algorithm, GetScatterPlotToYear)
}