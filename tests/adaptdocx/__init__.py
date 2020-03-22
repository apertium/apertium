#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import Popen, PIPE


class AdaptDocxTest(unittest.TestCase):
    """
    Subclass and override inputs/expectedOutputs (and possibly other stuff)
    to create new adapt-docx tests.
    """

    def runTest(self):
        pass

    def extractText(self, xmlFile, adapta):
        try:
            if adapta:
                entrada = (xmlFile + "\n").encode("utf8")
                cmd = ["../apertium/adapt-docx"]
                proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
                entrada = proc.communicate(entrada, 2)[0]
            else:
                cmd = ["cat", xmlFile]
                proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
                entrada = proc.communicate(None, 2)[0]
            cmd = ["python3", "adaptdocx/extract_docx.py"]
            proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
            result = proc.communicate(entrada, 2)[0]
            result = result.decode("utf8").split("\n")
            return result
        finally:
            pass


class SameTextTest(AdaptDocxTest):

    def runTest(self):
        for nom in ["docx-01"]:
            original = [linia.replace("|", "") for linia in self.extractText("data/" + nom + ".xml", False)]
            adaptat  = [linia.replace("|", "") for linia in self.extractText("data/" + nom + ".xml", True)]
            self.assertListEqual(original, adaptat)


class CorrectBoundariesTest(AdaptDocxTest):

    def runTest(self):
        for nom in ["docx-01"]:
            obtingut = self.extractText("data/" + nom + ".xml", True)
            with open("data/" + nom + ".txt") as f:
                esperat = [linia.strip("\n") for linia in f.readlines()]
            self.assertListEqual(obtingut, esperat)

