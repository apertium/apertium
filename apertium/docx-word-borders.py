#! /usr/bin/python3

"""
Prepara un fitxer xml per traduir.
Si una paraula està dividida en diversos elements "r", l'ajunta en el primer de tots.
Si un element "r" queda buit, l'esborra.
Si un element "t" comença o acaba amb un espai li posa el tag "xml:space"="preserve"
"""

import lxml.etree as ET
import io
import re
import os, sys, argparse


class docx():
    ns = {
        "r":   "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
        "wp":  "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing",
        "w":   "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
        "w10": "urn:schemas-microsoft-com:office:word",
        "w14": "http://schemas.microsoft.com/office/word/2010/wordml",
        "wps": "http://schemas.microsoft.com/office/word/2010/wordprocessingShape",
        "wpg": "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup",
        "mc":  "http://schemas.openxmlformats.org/markup-compatibility/2006",
        "v":   "urn:schemas-microsoft-com:vml",
        "o":   "urn:schemas-microsoft-com:office:office",
        "m":   "http://schemas.openxmlformats.org/officeDocument/2006/math",
        "xml": "http://www.w3.org/XML/1998/namespace"
    }

    def __init__(self, linies):
        docx.ns = self.ns
        # for prefix, uri in docx.ns.items():
        #     ET.register_namespace(prefix, uri)
        self.parser = ET.XMLParser(remove_blank_text=True)
        self.tot = ET.fromstring("".join(lin.strip() for lin in linies).encode("utf8"), self.parser)
        self.pars = [paragraf(p) for p in docx.iter(self.tot, "w:p")]

    def corregeix(self):
        for par in self.pars:
            par.corregeix()

    @staticmethod
    def iter(element, tag):
        ns, nom = tag.split(":")
        for e in element.iter("{%s}%s" % (docx.ns[ns], nom)):
            yield e

    def tostring(self, llegible):
        f = io.BytesIO(b"")
        arbre = ET.ElementTree(self.tot)
        arbre.write(f, encoding="utf-8", xml_declaration=True, pretty_print=llegible)
        str = f.getvalue().decode("utf-8")
        return str

class paragraf():
    def __init__(self, element):
        self.element = element
        self.rr = [r for r in docx.iter(element, "w:r")]
        self.tt = [t for r in self.rr for t in docx.iter(r, "w:t")]
        texts = []
        for t in self.tt:
            if self.get_atrib(t, "xml:space") == "preserve":
                texts.append(t.text)
            else:
                texts.append(t.text.strip())
        self.txt = "".join(texts)
        self.mides = [len(t) for t in texts]

    def get_atrib(self, element, nom, defecte = None):
        pn = nom.split(":", 1)
        pn = "{%s}%s" % (docx.ns[pn[0]], pn[1])
        valor = element.get(pn, defecte)
        return valor

    def set_atrib(self, element, nom, valor):
        pn = nom.split(":", 1)
        pn = "{%s}%s" % (docx.ns[pn[0]], pn[1])
        valor = element.set(pn, valor)

    def del_atrib(self, element, nom):
        pn = nom.split(":", 1)
        pn = "{%s}%s" % (docx.ns[pn[0]], pn[1])
        element.attrib.pop(pn, None)

    def text(self):
        return self.txt

    def parts(self):
        inici = 0
        pp = []
        for m in self.mides:
            pp.append(self.txt[inici:inici+m])
            inici += m
        return pp

    finalsSegurs = re.compile('[ ¡!"“”()¿?;,:.]')

    def cercaFrontera(self, posicio):
        """
        Cerca una frontera a la posició donada o més a la dreta (dins self.txt).
        La posició és un punt entre caràcters (com en slice).
        """
        if posicio == 0 or posicio >= len(self.txt):
            return posicio
        if paragraf.finalsSegurs.match(self.txt[posicio-1]):
            return posicio
        m = paragraf.finalsSegurs.search(self.txt[posicio:])
        if m:
            return posicio + m.start()
        else:
            return len(self.txt)

    def corregeix(self):
        """
        Modifica les fronteres de dos w:t consecutius si no coincideixen amb una frontera de mot.
        Suprimeix els w:t que han quedat buits.
        """
        if not self.mides:
            return
        canvis = False
        e, d = 0, 1
        inie, fie = 0, self.mides[0]
        while d < len(self.mides):
            if e == d:
                d += 1
                continue
            if self.mides[e] == 0:
                e += 1
                fie += self.mides[e]
                continue
            f = self.cercaFrontera(fie)
            moure = f - fie
            if moure > 0:
                canvis = True
                while moure > 0 and  d < len(self.mides):
                    if moure >= self.mides[d]:
                        self.mides[e] += self.mides[d]
                        fie += self.mides[d]
                        moure -= self.mides[d]
                        self.mides[d] = 0
                        d += 1
                    else:
                        self.mides[e] += moure
                        fie += moure
                        self.mides[d] -= moure
                        moure = 0
                    # print(self.parts())
            e += 1
            fie += self.mides[e]
        if not canvis:
            return
        novesMides = []
        nousRR = []
        nousTT = []
        inici = 0
        for i in range(len(self.mides)):
            mida = self.mides[i]
            if mida > 0:
                novesMides.append(mida)
                nouText = self.txt[inici:inici+mida]
                inici += mida
                self.tt[i].text = nouText
                if nouText == nouText.strip():
                    self.del_atrib(self.tt[i], "xml:space")
                else:
                    self.set_atrib(self.tt[i], "xml:space", "preserve")
                nousRR.append(self.rr[i])
                nousTT.append(self.tt[i])
            else:
                self.element.remove(self.rr[i])
        self.mides = novesMides
        self.rr = nousRR
        self.tt = nousTT

    def tostring(self, llegible):
        f = io.BytesIO(b"")
        arbre = ET.ElementTree(self.element)
        arbre.write(f, encoding="utf-8", xml_declaration=False, pretty_print=llegible)
        str = f.getvalue().decode("utf-8")
        str = re.sub(" xmlns:\\w+=\"[^\"]+\"", "", str)
        return str


def llegeixopcions(args = None):
    parser = argparse.ArgumentParser(description="Corregeix mots que contenen separacions interiors dins fitxers .xml de .docx.", prog=os.path.basename(__file__))
    parser.add_argument("-p", "--pretty", required=False, action="store_true", help="Treu el fitxer amb un format llegible", default=False)
    parser.add_argument("-n", "--nom", required=False, action="store_true", help="Afegeix <file name...> al principi, i separa les línies amb un espai", default=False)
    parser.add_argument("-f", "--fitxer", required=False, help="Fitxer que es processa; per defecte, llegeix el nom de stdin")
    if not args:
        args = sys.argv[1:]
    opcions = vars(parser.parse_args(args))
    if opcions['fitxer']:
        opcions['fitxer'] = [opcions['fitxer']]
    else:
        opcions['fitxer'] = [fitxer.strip() for fitxer in sys.stdin.readlines()]
    return opcions

def processa(opcions):
    for fitxer in opcions['fitxer']:
        with open(fitxer, encoding="utf8") as f:
            d = docx(f.readlines())
        for p in d.pars:
            p.corregeix()
        resultat = d.tostring(opcions['pretty'])
        if opcions['nom']:
            resultat = "<file name=\"{}\"/>\n".format(fitxer) + resultat
        if opcions['pretty']:
            print(resultat)
        else:
            print(resultat.replace("\n", " "))
    sys.stdin.flush()

if __name__ == "__main__":
    opcions = llegeixopcions()
    #print(opcions)
    processa(opcions)
