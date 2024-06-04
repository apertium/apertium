#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from base_test import IOTest
import unittest

class RestoreCapsTest(IOTest, unittest.TestCase):
    compileFlags = []
    runFlags = ['-k']
    ruleFile = 'data/basic.crx'
    pairs = [
        ('[[c:AA/AA]]^xyz<vblex>/XyZ$ ^qry<adj>/Qry$',
         '[[c:AA/AA]]xyz[[/]] qry'),
        ('^iPhone<np><top>/iPhone$', 'iPhone'),
        ('^Daniel<np><ant>/Daniel$', 'Daniel')
    ]

    def compile(self):
        self.checkCommand(['../apertium/apertium-compile-caps'] \
                          + self.compileFlags                     \
                          + [self.ruleFile, 'data/compiled.crx.bin'])

    def runTest(self):
        self.compile()
        cmd = ['../apertium/apertium-restore-caps'] \
            + self.runFlags                         \
            + ['data/compiled.crx.bin']
        self.runCommand(cmd, self.pairs)

class BeginRepeatTest(RestoreCapsTest):
    ruleFile = 'data/begin-repeat.crx'
    pairs = [
        ('[1] [[c:AA/aa]]^the<det><def>/the$ [[c:AA/aa]]^big<adj><sint>/big$ [[c:AA/aa]]^dog<n><sg>/dog$',
         '[1] [[c:AA/aa]]THE[[/]] [[c:AA/aa]]BIG[[/]] [[c:AA/aa]]DOG[[/]]'),
        ('[2] [[c:aa/aa]]^the<det><def>/the$ [[c:AA/aa]]^big<adj><sint>/big$ [[c:AA/aa]]^dog<n><sg>/dog$',
         '[2] [[c:aa/aa]]The[[/]] [[c:AA/aa]]big[[/]] [[c:AA/aa]]dog[[/]]'),
        ('[3] ^something<n>/something$^.<sent>/.$ [[c:AA/aa]]^the<det><def>/the$',
         '[3] Something. [[c:AA/aa]]The[[/]]'),
    ]

class StripWblanksTest(RestoreCapsTest):
    runFlags = []
    ruleFile = 'data/basic.crx'
    pairs = [
        ('[[c:AA/AA]]^xyz<vblex>/XyZ$ ^qry<adj>/Qry$',
         'xyz qry'),
        ('^iPhone<np><top>/iPhone$', 'iPhone'),
        ('^Daniel<np><ant>/Daniel$', 'Daniel')
    ]
