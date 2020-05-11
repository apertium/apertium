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

    def runCheckExit(self, proc, inp):
        """Run proc on input, then close and assert that we exited with 0.
Return output from proc."""
        timeout = 2
        res = proc.communicate(inp, timeout)
        for fd in [proc.stdin, proc.stdout, proc.stderr]:
            if fd is not None:
                fd.close()
        self.assertEqual(proc.poll(), 0)
        return res

    def adapt(self, xmlFile):
        entrada = (xmlFile + "\n").encode("utf8")
        cmd = ["../apertium/apertium-adapt-docx"]
        proc = Popen(cmd, stdin=PIPE, stdout=PIPE)
        return self.runCheckExit(proc, entrada)[0]

    def extractText(self, xmlFile, adapta):
        try:
            if adapta:
                entrada = self.adapt(xmlFile)
            else:
                cmd = ["cat", xmlFile]
                proc = Popen(cmd, stdin=PIPE, stdout=PIPE)
                entrada = self.runCheckExit(proc, None)[0]
            cmd = ["python3", "adaptdocx/extract_docx.py"]
            proc = Popen(cmd, stdin=PIPE, stdout=PIPE)
            result = self.runCheckExit(proc, entrada)[0]
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


class NoNewlinesTest(AdaptDocxTest):

    def runTest(self):
        for nom in ["docx-newlines"]:
            obtingut = self.adapt("data/" + nom + ".xml")
            esperat = open("data/" + nom + ".adapted.xml", "rb").read()
            self.assertEqual(obtingut, esperat)
