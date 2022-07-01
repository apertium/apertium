#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import Popen, PIPE, call

import signal


class Alarm(Exception):
    pass


class InterchunkTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new interchunk tests."""

    bindata = "data/nno-nob.t2x.bin"
    t2xdata = "data/apertium-nno-nob.nno-nob.t2x"
    flags = ["-z"]
    inputs = [""]
    expectedOutputs = [""]
    expectedRetCodeFail = False

    def alarmHandler(self, signum, frame):
        raise Alarm

    def withTimeout(self, seconds, cmd, *args, **kwds):
        signal.signal(signal.SIGALRM, self.alarmHandler)
        signal.alarm(seconds)
        ret = cmd(*args, **kwds)
        signal.alarm(0)         # reset the alarm
        return ret

    def communicateFlush(self, string):
        self.proc.stdin.write(string.encode('utf-8'))
        self.proc.stdin.write(b'\0')
        self.proc.stdin.flush()

        output = []
        char = None
        try:
            char = self.withTimeout(2, self.proc.stdout.read, 1)
        except Alarm:
            pass
        while char and char != b'\0':
            output.append(char)
            try:
                char = self.withTimeout(2, self.proc.stdout.read, 1)
            except Alarm:
                break           # send what we got up till now

        return b"".join(output).decode('utf-8')

    def compile(self):
        compileCmd = ["../apertium/apertium-preprocess-transfer",
                      self.t2xdata,
                      self.bindata]
        self.assertEqual(call(compileCmd),
                         0)

    def runTest(self):
        self.compile()
        try:
            cmd = ["../apertium/apertium-interchunk"] \
                + self.flags                         \
                + [self.t2xdata, self.bindata]
            self.proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)

            for inp, exp in zip(self.inputs, self.expectedOutputs):
                self.assertEqual(self.communicateFlush(inp+"[][\n]"),
                                 exp+"[][\n]")

            self.proc.communicate()  # let it terminate
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
