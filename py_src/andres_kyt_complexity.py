import lzma
import numpy

class Section:
    def __init__(self, lemmas):
        self._lemmas = lemmas
        self._references = []
        
    def add_ref(self, reference):
        self._references.append(reference)
        
    def get_lemmas(self):
        return self._lemmas
    
    def get_references(self):
        return self._references


def add_edge(a, b, w):
    if a not in w.keys():
        w[a] = dict()
    if b not in w[a].keys():
        w[a][b] = 1
    else:
        w[a][b] += 1
    return w


def morpho_complexity(lemmas):
    """
    Computer morphological complexity of a given array of pre-parsed lemmas. It does this by
    assigning all individual lemmas a unique one-char ID (a, yes and dodecahedron should all contribute equally
    to the complexity) and then passing the resulting string through a Huffman encoder
    :param lemmas: The lemmas to work with
    :return: complexity of the string or 0 if the incoming array was empty
    """
    our_lemmas = dict()
    subs = 32
    s_lemmas = ""
    for l in lemmas:
        if l not in our_lemmas.keys():
            our_lemmas[l] = subs
            subs += 1
        s_lemmas += chr(our_lemmas[l])

    l = len(lzma.compress(s_lemmas.encode('UTF-8'),
                          format=lzma.FORMAT_RAW, filters=[{"id": lzma.FILTER_LZMA2}]))

    return l / len(lemmas) if s_lemmas > "" else 0


def extract_paragraphs(sections):
    edges = dict()
    lc = dict()

    for section_id, section in sections.items():
        lc[section_id] = morpho_complexity(section.get_lemmas())
        for reference in section.get_references():
            edges = add_edge(section_id, reference, edges)
    return edges, lc


def calc_complexity(sections):
    edges, lc = extract_paragraphs(sections)

    # Generate the numpy matrix
    matrix = []

    s_keys = sorted(lc.keys())
    for a in s_keys:
        a_dict = edges.get(a, dict())
        row = []
        for b in s_keys:
            row.append(a_dict.get(b, 0) * max(lc[a], lc[b]))
        matrix.append(row)

    c1 = sum(lc.values())
    c2 = sum(list(map(lambda x: sum(x), matrix)))

    matrix = numpy.array(matrix)

    # Set diagonal to 1 so it becomes a DSM. Eigenvalues will be 0 otherwise
    numpy.fill_diagonal(matrix, 1)
    matrix = matrix + matrix.T
    matrix[matrix > 1] = 1

    c3 = numpy.real_if_close(sum(numpy.linalg.eigvals(matrix)))
    
    cpl = c1 + c2 * c3 / len(lc.keys())
    return cpl
