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

    def runTest(self):
        compileCmd = ["../apertium/apertium-preprocess-transfer", self.t3xdata, self.bindata]
        self.assertEqual(call(compileCmd),
                         0)
        try:
            cmd = ["../apertium/apertium-postchunk"] + self.flags + [self.t3xdata, self.bindata]
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
