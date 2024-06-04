#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import run


class CleanstreamTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new cleanstream tests."""

    flags = [""]
    inputs = [""]
    expectedOutputs = [""]
    expectedRetCodeFail = False

    def runTest(self):
        for inp, exp in zip(self.inputs, self.expectedOutputs):
            with self.subTest(input_line=inp):
                proc = run(
                    ["../apertium/apertium-cleanstream"] + self.flags,
                    input=inp.encode('utf-8'), capture_output=True, check=False,
                )
                self.assertEqual(proc.stderr.decode('utf-8'), "")
                self.assertEqual(proc.stdout.decode('utf-8'), exp)
                if self.expectedRetCodeFail:
                    self.assertNotEqual(0, proc.returncode)
                else:
                    self.assertEqual(0, proc.returncode)

class BasicCleanstream(CleanstreamTest):
    inputs =          ["^a<n>$ ^a<n>$", "^a<n>+c<po>$", "^a<vblex><pres># b$", "[<div>]^x<n>$", "[<div>]\\^^a<vblex><pres># b$"]
    expectedOutputs = ["^a<n>$ ^a<n>$", "^a<n>+c<po>$", "^a<vblex><pres># b$", " ^x<n>$",        "  ^a<vblex><pres># b$"]


class NewlineCleanstream(CleanstreamTest):
    flags = ["-n"]
    inputs =          ["^a<n>$ ^a<n>$", "^a<n>+c<po>$", "^a<vblex><pres># b$", "[<div>]^x<n>$", "[<div>]\\^^a<vblex><pres># b$"]
    expectedOutputs = ["\n^a<n>$\n^a<n>$\n", "\n^a<n>+c<po>$\n", "\n^a<vblex><pres># b$\n", "\n\n^x<n>$\n", "\n\n^a<vblex><pres># b$\n"]


class KeepblankCleanstream(CleanstreamTest):
    flags = ["-b"]
    inputs =          ["^a<n>$ ^a<n>$", "^a<n>+c<po>$", "^a<vblex><pres># b$", "[<div>]^x<n>$", "[<div>]\\^^a<vblex><pres># b$"]
    expectedOutputs = ["^a<n>$ ^a<n>$", "^a<n>+c<po>$", "^a<vblex><pres># b$", " [<div>]^x<n>$", " [<div>] ^a<vblex><pres># b$"]
