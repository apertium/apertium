#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

import itertools
from subprocess import Popen, PIPE, call
from tempfile import mkdtemp
from shutil import rmtree

import signal
class Alarm(Exception):
    pass

class PretransferTest(unittest.TestCase):
    """Subclass and override inputs/expectedOutputs (and possibly other
stuff) to create new pretransfer tests."""

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
        try:
            self.proc = Popen(["../apertium/apertium-pretransfer"] + self.flags,
                              stdin=PIPE,
                              stdout=PIPE,
                              stderr=PIPE)

            for inp, exp in zip(self.inputs, self.expectedOutputs):
                self.assertEqual(self.communicateFlush(inp+"[][\n]"),
                                 exp+"[][\n]")

            self.proc.communicate() # let it terminate
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


class BasicPretransferTest(PretransferTest):
    inputs =          ["^a<n>$", "^a<n>+c<po>$",   "^a<vblex><pres># b$", "[<div>]^a<n>$", "[<div>]^a<vblex><pres># b$"]
    expectedOutputs = ["^a<n>$", "^a<n>$ ^c<po>$", "^a# b<vblex><pres>$", "[<div>]^a<n>$", "[<div>]^a# b<vblex><pres>$"]

class JoinGroupPretransferTest(PretransferTest):
    inputs =          ["[<div>]^a<vblex><pres>+c<po># b$",   "[<div>]^a<vblex><pres>+c<po>+d<po># b$"]
    expectedOutputs = ["[<div>]^a# b<vblex><pres>$ ^c<po>$", "[<div>]^a# b<vblex><pres>$ ^c<po>$ ^d<po>$"]

class WordboundBlankTestPretransferTest(PretransferTest):
    inputs =          ["[[t:i:abc123]]^a<vblex><pres>+c<po># b$", "[[t:i:xyz456]]^a<vblex><pres>+c<po>+d<po># b$"]
    expectedOutputs = ["[[t:i:abc123]]^a# b<vblex><pres>$ [[t:i:abc123]]^c<po>$", "[[t:i:xyz456]]^a# b<vblex><pres>$ [[t:i:xyz456]]^c<po>$ [[t:i:xyz456]]^d<po>$"]
