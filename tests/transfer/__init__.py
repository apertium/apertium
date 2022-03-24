#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import Popen, PIPE, call

import signal


class Alarm(Exception):
    pass


class TransferTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new postchunk tests."""

    bindata = "data/nno-nob.t1x.bin"
    t1xdata = "data/apertium-nno-nob.nob-nno.t1x"
    flags = ["-b", "-z"]
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
                      self.t1xdata,
                      self.bindata]
        self.assertEqual(call(compileCmd),
                         0)

    def runTest(self):
        self.compile()
        try:
            cmd = ["../apertium/apertium-transfer"] \
                + self.flags                        \
                + [self.t1xdata, self.bindata]
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


class BasicTransferTest(TransferTest):
    inputs =          ["^hun<prn><f>/ho<prn><f>$"]
    expectedOutputs = ["^prn<prn><f>{^ho<prn><f>$}$"]


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
        pass


class EmptyTransferTest(TransferTest):
    t1xdata =         "data/empty.t1x"
    inputs =          ["^hun<prn><f>/ho<prn><f>$"]
    expectedOutputs = ["^default<default>{^ho<prn><f>$}$"]
