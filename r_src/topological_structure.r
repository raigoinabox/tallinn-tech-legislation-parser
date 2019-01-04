PrintCompAnalysis <- function(latexName, mode, graph, graphNumber) {
  graph.comp <- components(graph, mode = mode)
  CatData(paste0(latexName, firstup(mode), "Gcc"), max(graph.comp$csize))
  CatData(paste0(latexName, firstup(mode), "DcCount"), graph.comp$no - 1)
}

# mode for in-degree or out-degree and log for scaling
PlotDegreeDistribution <- function(graph, latexName, directory, mode = "all", ...) {
  deg.dist <- degree.distribution(graph, mode = mode)
  pow.law.res <- fit_power_law(deg.dist)
  CatData(latexName, "Scale", firstup(mode), pow.law.res$alpha)
  MyPowerLaw <- function(x) {
    x ^ (-pow.law.res$alpha)
  }
  pdf(file = file.path(directory, paste0(mode, "_degree.pdf")))
  plot(deg.dist, xlab = "Degree", ylab = "Relative degree distribution")
  curve(MyPowerLaw, add = TRUE)
  dev.off()
  
  pdf(file = file.path(directory, paste0(mode, "_degree_log.pdf")))
  plot(deg.dist, xlab = "Degree", ylab = "Relative degree distribution", log = "xy")
  curve(MyPowerLaw, add = TRUE)
  dev.off()
  # print(subset(as.data.frame(deg.dist), deg.dist > 0))
  # print(pow.law.res)
}

PlotRobustness <- function(mode, simple.graph, latexName, directory) {
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
  pdf(file = file.path(directory, paste0("robust_", mode, "_path.pdf")))
  plot(mean.dist.vec[!is.na(mean.dist.vec)],
       type = "l",
       xlab = "Nodes removed",
       ylab = "Average shortest path length")
  dev.off()
  
  which.gcc <- sapply(comp.size.vec, which.max)
  
  gcc.size <- mapply("[", comp.size.vec, which.gcc)
  pdf(file = file.path(directory, paste0("robust_", mode, "_gcc.pdf")))
  plot(gcc.size,
       type = "l",
       xlab = "Nodes removed",
       ylab = "GCC size")
  dev.off()
  
  comp.size.mean <- sapply(mapply("[", comp.size.vec, -which.gcc), mean)
  pdf(file = file.path(directory, paste0("robust_", mode, "_comp.pdf")))
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

PrintActAnalysis <- function(simple.graph, latexName, graph, directory) {
  CatData(paste0(latexName, "Nodes"), gorder(simple.graph))
  CatData(paste0(latexName, "References"), gsize(graph))
  CatData(paste0(latexName, "Links"), gsize(simple.graph))
  
  PrintCompAnalysis(latexName, "weak", simple.graph)
  PrintCompAnalysis(latexName, "strong", simple.graph)
  CatData(paste0(latexName, "MeanDeg"), mean(degree(simple.graph, mode = "in")))
  CatData(paste0(latexName, "MaxDeg"), max(degree(simple.graph, mode = "all")))
  PlotDegreeDistribution(simple.graph, latexName, directory)
  PlotDegreeDistribution(simple.graph, latexName, directory, mode = "out")
  PlotDegreeDistribution(simple.graph, latexName, directory, mode = "in")
  
  CatData(latexName, "Tran", "Global", transitivity(simple.graph))
  local.transitivity <-
    transitivity(as.undirected(simple.graph), type = "local")
  CatData(latexName, "Tran", "Local", mean(local.transitivity, na.rm = TRUE))
  
  pdf(file = file.path(directory, "transitivity.pdf"))
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
  
  pdf(file = file.path(directory, "kcore.pdf"))
  plot(induced_subgraph(simple.graph, which(max.kcore == graph.kcore)))
  dev.off()
  
  knnk <- knn(as.undirected(simple.graph))$knnk
  CatData(latexName, "Knnk", cor(seq_along(knnk), knnk, use = "complete.obs"))
  pdf(file = file.path(directory, "knnk.pdf"))
  plot(na.omit(data.frame(seq_along(knnk), knnk)),
       type = "l",
       xlab = "Degree",
       ylab = "Average nearest neighbor degree")
  dev.off()
  
  PlotRobustness("rand", simple.graph, latexName, directory)
  PlotRobustness("target", simple.graph, latexName, directory)
}

PrintTopologicalStructure <- function(db.conn, directory) {
  acts <- GetLargestActs(db.conn)
  act.latex.labels <-
    c(
      "corpTax",
      "incoTax",
      "gbThree",
      "gbFour",
      "gbFive",
      "gbSix",
      "gbSeven",
      "gbEight",
      "gbNine",
      "gbTen"
    )
  act.file.labels <-
    c("601",
      "782",
      "gb3",
      "gb4",
      "gb5",
      "gb6",
      "gb7",
      "gb8",
      "gb9",
      "gb10")
  
  for (i in seq_len(nrow(acts))) {
    row <- acts[i, ]
    row_id <- row$row_id
    graph <- GetGraph(db.conn, row_id)
    latex.label <- act.latex.labels[i]
    simple.graph <- simplify(graph)
    act.dir <- file.path(directory, act.file.labels[i])
    if (!dir.exists(act.dir)) {
      dir.create(act.dir)
    }
    
    cat("%", row$title, "\n")
    CatData(latex.label, row$title)
    PrintActAnalysis(simple.graph, latex.label, graph, act.dir)
    CatData(latex.label, "Complexity", row$complexity)
    cat("\n")
  }
  
  if (file.exists("random.graph")) {
    graph.random <- read.graph("random.graph", format = "graphml")
  } else {
    largest.graph <- simplify(GetGraph(db.conn, acts[1, ]$row_id))
    graph.random <-
      sample_gnm(length(V(largest.graph)), length(E(largest.graph)), directed = TRUE)
    write.graph(graph.random, "random.graph", format = "graphml")
  }

  rand.dir <- file.path(directory, "rand")
  if (!dir.exists(rand.dir)){
    dir.create(rand.dir)
  }
  cat("% Random graph\n")
  # PrintActAnalysis(simplify(graph.random), "random", graph.random, rand.dir)
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
    act.dir <- file.path(directory, file.label)
    if (!dir.exists(act.dir)) {
      dir.create(act.dir)
    }
    cat("%", row$title, "\n")
    CatData(latex.label, row$title)
    PrintActAnalysis(simplify(graph), latex.label, graph, act.dir)
    cat("\n")
  }
}