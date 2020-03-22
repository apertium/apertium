#! /usr/bin/python3

"""
Extract text from a .xml in a .docx file
Reads the file from stdin
"""

import lxml.etree as ET
import io
import re
import os, sys, argparse


class docx():

    def __init__(self, linies):
        self.parser = ET.XMLParser(remove_blank_text=True)
        self.tot = ET.fromstring("".join(lin.strip() for lin in linies).encode("utf8"), self.parser)
        self.pars = [paragraf(p) for p in docx.iter(self.tot, "w:p")]

    @staticmethod
    def iter(element, tag):
        ns, nom = tag.split(":")
        for e in element.iter("{%s}%s" % (docx.ns[ns], nom)):
            yield e

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
        self.txt = "|".join(texts)
        self.mides = [len(t) for t in texts]

    def get_atrib(self, element, nom, defecte = None):
        pn = nom.split(":", 1)
        pn = "{%s}%s" % (docx.ns[pn[0]], pn[1])
        valor = element.get(pn, defecte)
        return valor


linia = ""
while True:
    c = sys.stdin.read(1)
    if not c or c == '\0':
        break
    else:
        linia += c
linies = [ linia ]
#sys.stdout.write("#línies: %d" % (len(linies)))
doc = docx(linies)
for p in doc.pars:
    print(p.txt)
sys.stdout.write('\0')
sys.stdout.flush()

