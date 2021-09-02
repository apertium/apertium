#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import Popen, PIPE


class CleanstreamTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new cleanstream tests."""

    flags = [""]
    inputs = [""]
    expectedOutputs = [""]
    expectedRetCodeFail = False

    def runTest(self):
        try:
            for inp, exp in zip(self.inputs, self.expectedOutputs):
                self.proc = Popen(
                    ["../apertium/apertium-cleanstream"] + self.flags,
                    stdin=PIPE,
                    stdout=PIPE,
                    stderr=PIPE)

                (got, goterr) = self.proc.communicate(inp.encode('utf-8'))
                self.assertEqual(goterr.decode('utf-8'), "")
                self.assertEqual(got.decode('utf-8'), exp)

                self.proc.stdin.close()
                self.proc.stdout.close()
                self.proc.stderr.close()
                retCode = self.proc.poll()
                if self.expectedRetCodeFail:
                    self.assertNotEqual(retCode, 0)
                else:
                    self.assertEqual(retCode, 0)

        finally:
            pass


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
