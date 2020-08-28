#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from subprocess import Popen, PIPE, call

import signal


class Alarm(Exception):
    pass


class PostchunkTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new postchunk tests."""

    bindata = "data/nno-nob.t3x.bin"
    t3xdata = "data/apertium-nno-nob.nno-nob.t3x"
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
                      self.t3xdata,
                      self.bindata]
        self.assertEqual(call(compileCmd),
                         0)

    def runTest(self):
        self.compile()
        try:
            cmd = ["../apertium/apertium-postchunk"] \
                + self.flags                         \
                + [self.t3xdata, self.bindata]
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


class SimplePostchunkTest(PostchunkTest):
    inputs =          ["^vblex<vblex><imp>{^gå<vblex><imp>$}$^default<default>{^.<sent><clb>$}$",
                       "^vblex<vblex><imp>{^gå<vblex><imp>$}$^default<default>{^.<sent><clb>$}$"]
    expectedOutputs = ["^gå<vblex><imp>$^.<sent><clb>$",
                       "^gå<vblex><imp>$^.<sent><clb>$"]


class EmptyNoMacroPostchunkTest(PostchunkTest):
    inputs =          ["^nomacro<nomacro>{}$"]
    expectedOutputs = [""]


class EmptyMacroPostchunkTest(PostchunkTest):
    inputs =          ["^hasmacro<hasmacro>{}$"]
    expectedOutputs = [""]


class UseMacroPostchunkTest(PostchunkTest):
    inputs =          ["^thing<thing><sg>{^thing<thing><ND>$}$"]
    expectedOutputs = ["^thing<thing><sg>$"]

class WordboundBlankTest(PostchunkTest):
    inputs =          ["^n_n<SN><sg>{[[t:b:123456]]^worda<n><ND><m>$ ;[testblank] [[t:s:xyzab12]]^wordb# xyz<n><ND><f>$}$"]
    expectedOutputs = ["[[t:s:xyzab12]]^wordb# xyz<n><ND><f>$ ;[testblank] [[t:b:123456]]^worda<n><ND><m>$ [[t:b:123456; t:s:xyzab12]]^worda+wordb# xyz$"]

class SingleLUWordboundBlankTest(PostchunkTest):
    inputs =          ["^thing_wb<thing><sg>{^[[t:i:xyzabc]]thing<thing><ND>$}$ ^n_n<SN><sg>{[[t:b:123456]]^worda<n><ND><m>$ ;[testblank] [[t:s:xyzab12]]^wordb# xyz<n><ND><f>$}$ [blanks] ^thing_wb<thing><sg>{^[[t:i:xyzabc]]thing<thing><ND>$}$ [blankx] ^vblex<vblex><imp>{[[t:b:123zbc]]^gå<vblex><imp>$}$^default<default>{^.<sent><clb>$}$ [blanks3] ^thing<thing><sg>{^[[t:i:xyzabc]]thing<thing><ND>$}$"]
    expectedOutputs = ["[[t:i:xyzabc]]^newthing<adj><sg>$ [[t:i:xyzabc]]^thing<thing><sg>$ [[t:i:xyzabc]]^thing<thing><sg>+newpr<pr>$ [[t:s:xyzab12]]^wordb# xyz<n><ND><f>$ ;[testblank] [[t:b:123456]]^worda<n><ND><m>$ [[t:b:123456; t:s:xyzab12]]^worda+wordb# xyz$ [blanks] [[t:i:xyzabc]]^newthing<adj><sg>$ [[t:i:xyzabc]]^thing<thing><sg>$ [[t:i:xyzabc]]^thing<thing><sg>+newpr<pr>$ [blankx] [[t:b:123zbc]]^gå<vblex><imp>$^.<sent><clb>$ [blanks3] [[t:i:xyzabc]]^thing<thing><sg>$"]

class SuperblankTest(PostchunkTest):
    inputs =          [ "[blank1];; ^n_n<SN><sg>{^worda<n><ND><m>$ ;[blank2] ^wordb# xyz<n><ND><f>$}$ ;[blank3]; ",
                        "[blank1];; ^n_k<SN><sgn>{^worda<nn><NDn><mn>$ ;[blank2] ^wordb# xyz<nn><NDn><fn>$}$ ;[blank3]; ", #Blanks when no rules match
                        "[blank1];; ^n_n2<SN><sg>{^worda<n><ND><m>$ ;[blank2] ^wordb# xyz<n><ND><f>$ ;[blank3]; ^wordc<n>$}$ ;[blank4]; ", #When output rule has more <b/> than input blanks
                        "[blank1];; ^n_n3<SN><sg>{^worda<n><ND><m>$ ;[blank2] ^wordb# xyz<n><ND><f>$ ;[blank3]; ^wordc<n>$}$ ;[blank4]; ", #Output rule has no <b/>
                        "[blank1];; ^n_n4<SN><sg>{^worda<n><ND><m>$ ;[blank2] ^wordb# xyz<n><ND><f>$ ;[blank3]; ^wordc<n>$}$ ;[blank4]; ", #Output rule has one <b/>
                        "[blank1];; ^n_n<SN><sg>{^worda<n><ND><m>$ ;[blank2] ^wordb# xyz<n><ND><f>$}$ ;[blank3]; ^n_n4<SN><sg>{^worda<n><ND><m>$ ;[blank4] ^wordb# xyz<n><ND><f>$ ;[blank5]; ^wordc<n>$}$ ;[blank6]; "] #Multiple matching rules
    
    expectedOutputs = [ "[blank1];; ^wordb# xyz<n><ND><f>$ ;[blank2] ^worda<n><ND><m>$ ^worda+wordb# xyz$ ;[blank3]; ",
                        "[blank1];; ^worda<nn><NDn><mn>$ ;[blank2] ^wordb# xyz<nn><NDn><fn>$ ;[blank3]; ",
                        "[blank1];; ^wordb# xyz<n><ND><f>$ ;[blank2] ^worda<n><ND><m>$ ;[blank3]; ^worda+wordb# xyz$ ^wordc<n>$ ;[blank4]; ",
                        "[blank1];; ^wordb# xyz<n><ND><f>$^worda<n><ND><m>$^worda+wordb# xyz$^wordc<n>$ ;[blank2]  ;[blank3];  ;[blank4]; ",
                        "[blank1];; ^wordb# xyz<n><ND><f>$^worda<n><ND><m>$ ;[blank2] ^worda+wordb# xyz$^wordc<n>$ ;[blank3];  ;[blank4]; ",
                        "[blank1];; ^wordb# xyz<n><ND><f>$ ;[blank2] ^worda<n><ND><m>$ ^worda+wordb# xyz$ ;[blank3]; ^wordb# xyz<n><ND><f>$^worda<n><ND><m>$ ;[blank4] ^worda+wordb# xyz$^wordc<n>$ ;[blank5];  ;[blank6]; "]





class BincompatTest(SimplePostchunkTest):
    bindata = "data/bincompat.t3x.bin"

    def compile(self):
        pass
