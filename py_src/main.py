'''
Created on 6. dets 2018

@author: raigo
'''

import psycopg2
import spacy

import andres_kyt_complexity

NLP = spacy.load('en')

def insert_result(cursor, act_id, complexity):
    cursor.execute(
        "insert into legal_act_complexities (act_id, complexity) values (%s, %s)",
        (act_id, complexity))


def main():
    db_conn = psycopg2.connect("dbname=tallinn_tech_legislation_parser")

    cursor = db_conn.cursor()
    cursor.execute("truncate legal_act_complexities")
    cursor.execute(
        """select
            legal_act_sections.act_id,
            legal_act_sections.section_number,
            legal_act_section_texts.section_text,
            legal_act_references.reference
        from legal_act_sections
            join legal_act_references
                on legal_act_sections.row_id = legal_act_references.section_id
            join legal_act_section_texts
                on legal_act_sections.row_id = legal_act_section_texts.section_id""")

    acts = {}
    for row in cursor:
        act_id = row[0]
        section_id = row[1]
        section_text = row[2]
        reference = row[3]
        if act_id not in acts:
            acts[act_id] = {}
        
        act = acts[act_id]
        if section_id not in act:
            act[section_id] = andres_kyt_complexity.Section(
                [token.lemma_ for token in NLP(section_text)])
        
        section = act[section_id]
        section.add_ref(reference)
        
    for act_id, act in acts.items():
        complexity = andres_kyt_complexity.calc_complexity(act)
        insert_result(cursor, act_id, complexity)
        

    cursor.close()
    db_conn.commit()
    db_conn.close()

if __name__ == '__main__':
    main()
