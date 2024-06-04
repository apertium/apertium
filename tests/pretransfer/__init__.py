#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from base_test import IOTest
import unittest

class PretransferTest(IOTest, unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new pretransfer tests."""

    flags = ["-z"]
    inputs = [""]
    expectedOutputs = [""]

    def runTest(self):
        self.runCommand(['../apertium/apertium-pretransfer'] + self.flags,
                        zip(self.inputs, self.expectedOutputs))

class BasicPretransferTest(PretransferTest):
    inputs =          ["^a<n>$", "^a<n>+c<po>$",   "^a<vblex><pres># b$", "[<div>]^a<n>$", "[<div>]^a<vblex><pres># b$"]
    expectedOutputs = ["^a<n>$", "^a<n>$ ^c<po>$", "^a# b<vblex><pres>$", "[<div>]^a<n>$", "[<div>]^a# b<vblex><pres>$"]

class JoinGroupPretransferTest(PretransferTest):
    inputs =          ["[<div>]^a<vblex><pres>+c<po># b$",   "[<div>]^a<vblex><pres>+c<po>+d<po># b$"]
    expectedOutputs = ["[<div>]^a# b<vblex><pres>$ ^c<po>$", "[<div>]^a# b<vblex><pres>$ ^c<po>$ ^d<po>$"]

class WordboundBlankTestPretransferTest(PretransferTest):
    inputs =          ["[[t:i:abc123]]^a<vblex><pres>+c<po># b$", "[[t:i:xyz456]]^a<vblex><pres>+c<po>+d<po># b$"]
    expectedOutputs = ["[[t:i:abc123]]^a# b<vblex><pres>$ [[t:i:abc123]]^c<po>$", "[[t:i:xyz456]]^a# b<vblex><pres>$ [[t:i:xyz456]]^c<po>$ [[t:i:xyz456]]^d<po>$"]
