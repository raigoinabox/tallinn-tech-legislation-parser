GetComplexities <- function(db.conn) {
  dbGetQuery(
    db.conn,
    paste(
      "select core.dbu_category, complexity, dtf, complexity_algorithms.name algorithm, core.year",
      "from complexity_results core",
      "join doing_business_results dbre on core.year = dbre.year",
      "join complexity_algorithms on core.algorithm = complexity_algorithms.row_id",
      "and core.dbu_category = dbre.dbu_category",
      "and core.country = dbre.country",
      "where core.country = 'GB'"
    )
  )
}

GetGraph <- function(db.conn, act.id) {
  edges <- dbGetQuery(
    db.conn,
    paste(
      "select section_number, reference from legal_act_sections",
      "left join legal_act_references on legal_act_sections.row_id = legal_act_references.section_id",
      "where act_id = ",
      act.id
    )
  )
  
  graph <- graph_from_data_frame(edges)
  if (length(V(graph)[name == "NA"]) != 0) {
    delete.vertices(graph, "NA")  
  } else {
    graph
  }
}

GetLargestActs <- function(db.conn) {
  dbGetQuery(
    db.conn,
    paste(
      "select legal_acts.row_id, legal_acts.title, complexity",
      "from legal_acts join legal_act_sections on legal_acts.row_id = legal_act_sections.act_id",
      "join legal_act_references on legal_act_sections.row_id = legal_act_references.section_id",
      "join legal_act_complexities on legal_acts.row_id = legal_act_complexities.act_id",
      "where legal_acts.language = 'eng'",
      "group by legal_acts.row_id, complexity order by count(*) desc limit 10"
    )
  )
}

GetEstonianActs <- function(db.conn) {
  dbGetQuery(
    db.conn,
    paste(
      "select legal_acts.row_id, legal_acts.title",
      "from legal_acts join legal_act_sections on legal_acts.row_id = legal_act_sections.act_id",
      "join legal_act_references on legal_act_sections.row_id = legal_act_references.section_id",
      "where legal_acts.language = 'est'",
      "group by legal_acts.row_id order by count(*) desc limit 1"
    )
  )
}