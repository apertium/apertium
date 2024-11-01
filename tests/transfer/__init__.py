#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from base_test import IOTest
import unittest

from subprocess import call

class TransferTest(IOTest, unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new postchunk tests."""

    bindata = "data/nno-nob.t1x.bin"
    t1xdata = "data/apertium-nno-nob.nob-nno.t1x"
    flags = ["-b", "-z"]
    inputs = [""]
    expectedOutputs = [""]
    expectedCompRetCodeFail = False

    def compile(self):
        retCode = call(["../apertium/apertium-preprocess-transfer",
						self.t1xdata,
						self.bindata])
        if self.expectedCompRetCodeFail:
            self.assertNotEqual(retCode, 0)
        else:
            self.assertEqual(retCode, 0)
        return retCode == 0

    def runTest(self):
        if not self.compile():
            return
        cmd = ["../apertium/apertium-transfer"] \
            + self.flags                        \
            + [self.t1xdata, self.bindata]
        self.runCommand(cmd, zip(self.inputs, self.expectedOutputs))

class BasicTransferTest(TransferTest):
    inputs =          ["^hun<prn><f>/ho<prn><f>$"]
    expectedOutputs = ["^prn<prn><f>{^ho<prn><f>$}$"]

class SlashTagTest(TransferTest):
    inputs =          ['^hun<prn><m/f>/ho<prn><m/f>$']
    expectedOutputs = ['^prn<prn><m/f>{^ho<prn><m/f>$}$']

class SlLemqTest(TransferTest):
    inputs =          ["^skyldes<vblex><pstv><pres>/komme# av<vblex><pres>$"]
    expectedOutputs = ["sl-lemq:'' tl-lemq:'# av'"]

class WordboundBlankTest(TransferTest):
    inputs =          ["[blank1] [[t:s:123456]]^worda<n><acr>/wordta<n><acr>$ ;[blank2]; [[t:b:xyz123; t:l:xyz347]]^wordb<n><acr>/wordtb<n><acr>$ [blank3];  [[t:i:abc123; t:s:abc123]]^hun<prn><f>/ho<prn><f>$"]
    expectedOutputs = ["[blank1] ^prn<prn><f>{[[t:i:abc123; t:s:abc123]]^ho<prn><f>$[[t:b:xyz123; t:l:xyz347]]^wordtb<nacr><sg><m>$}$ ;[blank2]; ^det<n><acr>{[[t:s:123456; t:i:abc123; t:s:abc123]]^wordta<n><acr>+ho<prn><f>$}$ [blank3];  "]

class SingleLUWordboundBlankTest(TransferTest):
    inputs =          ["[blank1] [[t:s:123456]]^worda<n><acr>/wordta<n><acr>$ ;[blank2]; [[t:b:xyz123; t:l:xyz347]]^wordb<n><acr>/wordtb<n><acr>$ [blank3]; "]
    expectedOutputs = ["[blank1] ^nacr<n><acr>{[[t:s:123456]]^test<n><sg>$ [[t:s:123456]]^wordta<n><acr>$}$ ^nacr2<n><acr>{[[t:s:123456]]^testlem<n><pl>$ [[t:s:123456]]^wordta<n><acr>+postp<adj><sg>$}$ ;[blank2]; ^nacr<n><acr>{[[t:b:xyz123; t:l:xyz347]]^test<n><sg>$ [[t:b:xyz123; t:l:xyz347]]^wordtb<n><acr>$}$ ^nacr2<n><acr>{[[t:b:xyz123; t:l:xyz347]]^testlem<n><pl>$ [[t:b:xyz123; t:l:xyz347]]^wordtb<n><acr>+postp<adj><sg>$}$ [blank3]; "]

class SuperblankTest(TransferTest):
    inputs =          [ "[blank1] ^worda<det>/wordta<det>$ ;[blank2]; ^wordb<adj>/wordtb<adj>$ [blank3];  ^hun<n><acr>/ho<n><acr>$ [blank4]  ",          #Rule: superblankrule1 -> No <b/> in rule output, should flush all blanks after rule output
                        "[blank1] ^worda<detn>/wordta<detn>$ ;[blank2]; ^wordb<adjn>/wordtb<adjn>$ [blank3];  ^hun<nn><acr>/ho<nn><acr>$ [blank4]  ",   #No rule matches, should print all blanks as is
                        "[blank1] ^wordb<adj>/wordtb<adj>$ ;[blank2]; ^worda<det>/wordta<det>$ [blank3];  ^hun<n><acr>/ho<n><acr>$ [blank4]  ",         #Rule: superblankrule2 -> One <b/> in rule output, should print one and flush the rest
                        "[blank1] ^hun<n><acr>/ho<n><acr>$ ;[blank2]; ^worda<det>/wordta<det>$ [blank3];   [blank4]  ",                                 #Rule: superblankrule3 -> Input rule has 1 blank, output has 3, should print input blank for the first <b/> and just spaces for the rest
                        "[blank1] ^hun<n><acr>/ho<n><acr>$ ;[blank2]; ^worda<det>/wordta<det>$ [blank3]; ^wordb<adj>/wordtb<adj>$ ;[blank4]; ^worda<det>/wordta<det>$ [blank5];  ^hun<n><acr>/ho<n><acr>$ [blank6]  "] #Multiple rule matches
    expectedOutputs = [ "[blank1] ^test1<adj>{^wordta<det>$^wordtb<adj>$^ho<n><acr>$}$ ;[blank2];  [blank3];   [blank4]  ",
                        "[blank1] ^default<default>{^wordta<detn>$}$ ;[blank2]; ^default<default>{^wordtb<adjn>$}$ [blank3];  ^default<default>{^ho<nn><acr>$}$ [blank4]  ",
                        "[blank1] ^test1<det>{^wordta<det>$ ;[blank2]; ^ho<n><acr>$}$ [blank3];   [blank4]  ",
                        "[blank1] ^test1<det>{^ho<n><acr>$ ;[blank2]; ^wordta<det>$ ^ho<n><acr>$ ^wordta<det>$}$ [blank3];   [blank4]  ",
                        "[blank1] ^test1<det>{^ho<n><acr>$ ;[blank2]; ^wordta<det>$ ^ho<n><acr>$ ^wordta<det>$}$ [blank3]; ^test1<det>{^wordta<det>$ ;[blank4]; ^ho<n><acr>$}$ [blank5];   [blank6]  "]

class BincompatTest(BasicTransferTest):
    bindata = "data/bincompat.t1x.bin"

    def compile(self):
        return True


class EmptyTransferTest(TransferTest):
    t1xdata =         "data/empty.t1x"
    inputs =          ["^hun<prn><f>/ho<prn><f>$"]
    expectedOutputs = ["^default<default>{^ho<prn><f>$}$"]


class BadAttrTest(TransferTest):
    t1xdata = "data/bad-attr.t1x"
    expectedCompRetCodeFail = True

class SlashLuTransferTest(TransferTest):
    t1xdata =         "data/lu.t1x"
    inputs =          ["^hun<prn><f/z>/ho<prn><f/z>$",
                       '^a<n><2/3>/b<n><abc>$']
    expectedOutputs = ["^ho<prn><f/z><abc><1/2>$",
                       '^b<n><abc>$']
