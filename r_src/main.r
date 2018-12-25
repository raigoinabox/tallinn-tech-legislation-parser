# r-cran-rpostgresql required (on Ubuntu 18.04)
# r-cran-xtable required (on Ubuntu 18.04)
# r-cran-igraph required (on Ubuntu 18.04)

library(DBI)
library(xtable)
library(RPostgreSQL)
library(igraph)

source("dtf.r")
source("dao.r")

StandardError <- function(x) {
  sd(x, na.rm = TRUE) / sqrt(length(x))
}

GetScatterPlotToDtf <- function(data) {
  plot(
    data$complexity,
    data$dtf,
    main = paste("Algorithm", levels(factor(data$algorithm))),
    xlab = "Complexities",
    ylab = "Distance To Frontier"
  )
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
    complexities[complexities$dbu_category == 'Starting a Business', ]
  
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
  cat("\\newcommand\\", paste0(as.vector(
    head(args,-1), mode = "character"
  )), "{", value, "}\n", sep = "")
}

FileName <- function(...) {
  paste0(
    "/home/raigo/Dokumendid/magistri_lõputöö/thesis_2/img/",
    paste(..., sep = "_")
  )
}

PrintCompAnalysis <- function(latexName, mode, graph, graphNumber) {
  graph.comp <- components(graph, mode = mode)
  CatData(paste0(latexName, firstup(mode), "Gcc"), max(graph.comp$csize))
  CatData(paste0(latexName, firstup(mode), "DcCount"), graph.comp$no - 1)
}

# mode for in-degree or out-degree and log for scaling
PlotDegreeDistribution <- function(graph, latexName, graph.number, mode = "all", ...) {
  deg.dist <- degree.distribution(graph, mode = mode)
  pow.law.res <- fit_power_law(deg.dist)
  CatData(latexName, "Scale", firstup(mode), pow.law.res$alpha)
  MyPowerLaw <- function(x) {
    x ^ (-pow.law.res$alpha)
  }
  pdf(file = FileName(graph.number, mode, "degree.pdf"))
  plot(deg.dist, xlab = "Degree", ylab = "Relative degree distribution")
  curve(MyPowerLaw, add = TRUE)
  dev.off()
  
  pdf(file = FileName(graph.number, mode, "degree_log.pdf"))
  plot(deg.dist, xlab = "Degree", ylab = "Relative degree distribution", log = "xy")
  curve(MyPowerLaw, add = TRUE)
  dev.off()
  # print(subset(as.data.frame(deg.dist), deg.dist > 0))
  # print(pow.law.res)
}

PlotRobustness <- function(mode, simple.graph, displayName, latexName) {
  subgraph <- as.undirected(simple.graph)
  subgraphs <- vector("list", gorder(subgraph))
  for (i in seq_len(gorder(subgraph))) {
    subgraphs[[i]] <- subgraph
    
    if (mode == "rand") {
      key.vertex <- sample(gorder(subgraph), 1)
    } else {
      key.vertex <- which.max(betweenness(subgraph))
    }
    subgraph <- delete.vertices(subgraph, key.vertex)
  }
  
  comp.size.vec <- sapply(subgraphs, function(subgraph) {
    components(subgraph)$csize
  })
  mean.dist.vec <- sapply(subgraphs, mean_distance)
  pdf(file = FileName(displayName, "robust", mode, "path.pdf"))
  plot(mean.dist.vec[!is.na(mean.dist.vec)],
       type = "l",
       xlab = "Nodes removed",
       ylab = "Average shortest path length")
  dev.off()
  
  which.gcc <- sapply(comp.size.vec, which.max)
  
  gcc.size <- mapply("[", comp.size.vec, which.gcc)
  pdf(file = FileName(displayName, "robust", mode, "gcc.pdf"))
  plot(gcc.size,
       type = "l",
       xlab = "Nodes removed",
       ylab = "GCC size")
  dev.off()
  
  comp.size.mean <- sapply(mapply("[", comp.size.vec, -which.gcc), mean)
  pdf(file = FileName(displayName, "robust", mode, "comp.pdf"))
  plot(comp.size.mean,
       type = "l",
       xlab = "Nodes removed",
       ylab = "Average component size")
  dev.off()
  
  if (mode == "rand") {
    word <- "Rand"
  } else {
    word <- "Targ"
  }
  perc <- head(which(gcc.size < gorder(simple.graph) / 10), 1)
  CatData(latexName, word, "Perc", perc)
  CatData(latexName, word, "PercPath", mean.dist.vec[perc])
  CatData(latexName, word, "PercComp", comp.size.mean[perc])
}

PrintActAnalysis <- function(displayName, simple.graph, latexName, graph) {
  CatData(paste0(latexName, "Nodes"), gorder(simple.graph))
  CatData(paste0(latexName, "References"), gsize(graph))
  CatData(paste0(latexName, "Links"), gsize(simple.graph))
  
  PrintCompAnalysis(latexName, "weak", simple.graph)
  PrintCompAnalysis(latexName, "strong", simple.graph)
  CatData(paste0(latexName, "MeanDeg"), mean(degree(simple.graph, mode = "in")))
  CatData(paste0(latexName, "MaxDeg"), max(degree(simple.graph, mode = "all")))
  PlotDegreeDistribution(simple.graph, latexName, displayName)
  PlotDegreeDistribution(simple.graph, latexName, displayName, mode = "out")
  PlotDegreeDistribution(simple.graph, latexName, displayName, mode = "in")
  
  CatData(latexName, "Tran", "Global", transitivity(simple.graph))
  local.transitivity <-
    transitivity(as.undirected(simple.graph), type = "local")
  CatData(latexName, "Tran", "Local", mean(local.transitivity, na.rm = TRUE))
  
  pdf(file = FileName(displayName, "transitivity.pdf"))
  plot(
    sort(local.transitivity),
    type = "l",
    xlab = "Vertex",
    ylab = "Clustering coefficient"
  )
  dev.off()
  CatData(latexName, "MeanDist", mean_distance(simple.graph))
  CatData(paste0(latexName, "Diameter"), diameter(simple.graph))
  comps <- components(simple.graph)
  CatData(paste0(latexName, "Radius"), radius(induced_subgraph(
    simple.graph, which(comps$membership == which.max(comps$csize))
  )))
  strong.comps <- components(simple.graph, mode = "strong")
  my.subgraph <-
    induced_subgraph(simple.graph, which(strong.comps$membership == which.max(strong.comps$csize)))
  out.radius <- radius(my.subgraph, mode = "out")
  CatData(paste0(latexName, "RadiusOut"), out.radius)
  in.radius <- radius(my.subgraph, mode = "in")
  CatData(paste0(latexName, "RadiusIn"), in.radius)
  
  CatData(latexName, "Betweenness", mean(betweenness(simple.graph)))
  CatData(latexName, "EdgeBetweenness", mean(edge.betweenness(simple.graph)))
  CatData(latexName, "Cutpoints", length(articulation.points(simple.graph)))
  CatData(latexName, "Bicomponents", biconnected.components(simple.graph)$no)
  
  graph.kcore <- coreness(simple.graph)
  max.kcore <- max(graph.kcore)
  CatData(latexName, "Kcore", max.kcore)
  CatData(latexName, "KcoreSize",  sum(max.kcore == graph.kcore))
  
  pdf(file = FileName(displayName, "kcore.pdf"))
  plot(induced_subgraph(simple.graph, which(max.kcore == graph.kcore)))
  dev.off()
  
  knnk <- knn(as.undirected(simple.graph))$knnk
  CatData(latexName, "Knnk", cor(seq_along(knnk), knnk, use = "complete.obs"))
  pdf(file = FileName(displayName, "knnk.pdf"))
  plot(na.omit(data.frame(seq_along(knnk), knnk)),
       type = "l",
       xlab = "Degree",
       ylab = "Average nearest neighbor degree")
  dev.off()
  
  PlotRobustness("rand", simple.graph, displayName, latexName)
  PlotRobustness("target", simple.graph, displayName, latexName)
}

Main <- function() {
  db.driver <- dbDriver("PostgreSQL")
  db.conn <-
    dbConnect(db.driver, dbname = "tallinn_tech_legislation_parser", host = "")
  on.exit(dbDisconnect(db.conn))
  
  options(
    xtable.tabular.environment = 'tabulary',
    xtable.math.style.negative = TRUE,
    xtable.width = '\\textwidth',
    xtable.floating = FALSE
  )
  pdf.options(
    width = 6,
    height = 6
  )
  if (TRUE) {
    # if (!dir.exists("img")) {
    #   dir.create("img")
    # }
    
    acts <- GetLargestActs(db.conn)
    act.latex.labels <- c("corpTax", "incoTax", "graphThree", "graphFour")
    act.file.labels <- c("601", "782", "graph3", "graph4")
    
    for (i in seq_len(nrow(acts))) {
      row <- acts[i, ]
      row_id <- row$row_id
      graph <- GetGraph(db.conn, row_id)
      latex.label <- act.latex.labels[i]
      file.label <- act.file.labels[i]
      simple.graph <- simplify(graph)
      cat("%", row$title, "\n")
      CatData(latex.label, row$title)
      PrintActAnalysis(file.label, simple.graph, latex.label, graph)
      CatData(latex.label, "Complexity", row$complexity)
      cat("\n")
    }
    
    if (file.exists("random.graph")) {
      graph.random <- read.graph("random.graph", format = "graphml")
    } else {
      largest.graph <- simplify(GetGraph(db.conn, acts[0, ]$row_id))
      graph.random <-
        sample_gnm(length(V(largest.graph)), length(E(largest.graph)), directed = TRUE)
      write.graph(graph.random, "random.graph", format = "graphml")
    }
    
    cat("% Random graph\n")
    PrintActAnalysis("random", graph.random, "rand", graph.random)
    cat("\n")

    est.acts <- GetEstonianActs(db.conn)
    est.act.latex.labels <- c("estOne")
    est.act.file.labels <- c("est1")
    for (i in seq_len(nrow(est.acts))) {
      row <- est.acts[i, ]
      row_id <- row$row_id
      graph <- GetGraph(db.conn, row_id)
      latex.label <- est.act.latex.labels[i]
      file.label <- est.act.file.labels[i]
      simple.graph <- simplify(graph)
      cat("%", row$title, "\n")
      CatData(latex.label, row$title)
      PrintActAnalysis(file.label, simple.graph, latex.label, graph)
      cat("\n")
    }
  } else {
    # before the switch to study of topological structure
    complexities <- GetComplexities(db.conn)
    
    print("Starting a business complexity")
    print(xtable(print(
      GetStartingABusinessComplexity(complexities)
    )))
    print("Correlation to each other")
    print(xtable(print(cor(
      do.call(
        cbind,
        by(complexities, complexities$algorithm, function(data)
          data[c("complexity")])
      )
    ))))
    print("Correlation to dtf")
    print(xtable(print(
      GetCorrelationsToDtfByAlgorithm(complexities)
    )))
    # print("Correlation to year")
    # print(GetCorrelationsToYearByAlgorithm(complexities))
    by(complexities,
       complexities$algorithm,
       GetScatterPlotToDtf)
    # by(complexities, complexities$algorithm, GetScatterPlotToYear)
  }
}

Main()
