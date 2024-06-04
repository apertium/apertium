#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from base_test import IOTest
import unittest

class InterchunkTest(IOTest, unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new interchunk tests."""

    bindata = "data/nno-nob.t2x.bin"
    t2xdata = "data/apertium-nno-nob.nno-nob.t2x"
    flags = ["-z"]
    inputs = [""]
    expectedOutputs = [""]

    def compile(self):
        self.checkCommand(['../apertium/apertium-preprocess-transfer',
                           self.t2xdata, self.bindata])

    def runTest(self):
        self.compile()
        cmd = ['../apertium/apertium-interchunk'] \
            + self.flags \
            + [self.t2xdata, self.bindata]
        self.runCommand(cmd, zip(self.inputs, self.expectedOutputs))

class SimpleInterchunkTest(InterchunkTest):
    inputs =          ["^prn<prn><f>{^ho<prn><f>$}$ ^prn2<prn2><f>{^ho<prn2><f>$}$",
                       "^vblex<vblex><imp>{^gå<vblex><imp>$}$^default<default>{^.<sent><clb>$}$"]
    expectedOutputs = ["^prn2<prn2><f>{^ho<prn2><f>$}$^prn<prn><f>{^ho<prn><f>$}$",
                       "^vblex<vblex><imp>{^gå<vblex><imp>$}$^default<default>{^.<sent><clb>$}$"]

class WordboundBlankTest(InterchunkTest):
    inputs =          ["^n_n<SN><sg>{[[t:b:123456]]^worda<n><ND><m>$ ;[testblank] [[t:s:xyzab12]]^wordb# xyz<n><ND><f>$}$",
                        "^prn<prn><f>{[[t:b:abc823]]^ho<prn><f>$}$ ^prn2<prn2><f>{[[t:i:poa023; t:span:12xas23]]^ho<prn2><f>$}$"]
    expectedOutputs = ["^n_n<SN><sg>{[[t:b:123456]]^worda<n><ND><m>$ ;[testblank] [[t:s:xyzab12]]^wordb# xyz<n><ND><f>$}$",
                        "^prn2<prn2><f>{[[t:i:poa023; t:span:12xas23]]^ho<prn2><f>$}$^prn<prn><f>{[[t:b:abc823]]^ho<prn><f>$}$"]

class SuperblankTest(InterchunkTest):
    inputs =          [ "[blank1];; ^test1<test1>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];;", #superblankrule1
                        "[blank1];; ^test1<test1x>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2x>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test3<test3x>{^wordc# xyz<n><ND><f>$}$ [blank4];;", #Blanks when no rules match
                        "[blank1];; ^test2<test2>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];;", #superblankrule2 -> When output rule has more <b/> than input blanks, print all then spaces
                        "[blank1];; ^test3<test3>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test1<test1>{^wordc# xyz<n><ND><f>$}$ [blank4];;", #superblankrule3 -> Output rule has no <b/>, flush all blanks after rule output
                        "[blank1];; ^test1<test1>{^worda<n><ND><m>$}$ ;[blank2] ^test3<test3>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test2<test2>{^wordc# xyz<n><ND><f>$}$ [blank4];;", #superblankrule4 -> Output rule has one <b/>, print one blank, then flush all after rule output
                        "[blank1];; ^test1<test1>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];; ^test1<test1>{^worda<n><ND><m>$}$ ;[blank5] ^test3<test3>{^wordb# xyz<n><ND><f>$}$ ;[blank6]; ^test2<test2>{^wordc# xyz<n><ND><f>$}$ [blank7];;", #Multiple matching rules -> superblankrule1 & superblankrule4
                        "[blank1];; ^test1<test1>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test2x<test2z>{^wordc# xyz<n><ND><f>$}$ [blank4];; ^test2<test2x>{^wordb# xyz<n><ND><f>$}$ ;[blank5];"] #Rule followed by unknown

    expectedOutputs = [ "[blank1];; ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank2] ^test1<test1>{^worda<n><ND><m>$}$ ;[blank3]; ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];;",
                        "[blank1];; ^test1<test1x>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2x>{^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^test3<test3x>{^wordc# xyz<n><ND><f>$}$ [blank4];;",
                        "[blank1];; ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank2] ^test2<test2>{^worda<n><ND><m>$}$ ;[blank3]; ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ^test2<test2>{^worda<n><ND><m>$}$ ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];;",
                        "[blank1];; ^test2<test2>{^wordb# xyz<n><ND><f>$}$^test3<test3>{^worda<n><ND><m>$}$^test1<test1>{^wordc# xyz<n><ND><f>$}$ ;[blank2]  ;[blank3];  [blank4];;",
                        "[blank1];; ^test3<test3>{^wordb# xyz<n><ND><f>$}$^test1<test1>{^worda<n><ND><m>$}$ ;[blank2] ^test2<test2>{^wordc# xyz<n><ND><f>$}$ ;[blank3];  [blank4];;",
                        "[blank1];; ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank2] ^test1<test1>{^worda<n><ND><m>$}$ ;[blank3]; ^test3<test3>{^wordc# xyz<n><ND><f>$}$ [blank4];; ^test3<test3>{^wordb# xyz<n><ND><f>$}$^test1<test1>{^worda<n><ND><m>$}$ ;[blank5] ^test2<test2>{^wordc# xyz<n><ND><f>$}$ ;[blank6];  [blank7];;",
                        "[blank1];; ^test2<test2>{^wordb# xyz<n><ND><f>$}$ ;[blank2] ^test1<test1>{^worda<n><ND><m>$}$ ;[blank3]; ^test2x<test2z>{^wordc# xyz<n><ND><f>$}$ [blank4];; ^test2<test2x>{^wordb# xyz<n><ND><f>$}$ ;[blank5];"]


class BincompatTest(SimpleInterchunkTest):
    bindata = "data/bincompat.t2x.bin"

    def compile(self):
        pass

class EmptyTransferTest(InterchunkTest):
    t2xdata =         "data/empty.t1x"
    inputs =          ["^default<default>{^ho<prn><f>$}$"]
    expectedOutputs = ["^default<default>{^ho<prn><f>$}$"]
