#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from subprocess import Popen, PIPE, call
import signal

class Alarm(Exception):
    pass

class RestoreCapsTest(unittest.TestCase):
    compileFlags = []
    runFlags = []
    ruleFile = 'data/basic.crx'
    pairs = [
        ('[[c:AA/AA]]^xyz<vblex>/XyZ$ ^qry<adj>/Qry$',
         '[[c:AA/AA]]xyz[[/]] qry'),
        ('^iPhone<np><top>/iPhone$', 'iPhone'),
        ('^Daniel<np><ant>/Daniel$', 'Daniel')
    ]

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
        cmd = ['../apertium/apertium-compile-caps'] \
            + self.compileFlags                     \
            + [self.ruleFile, 'data/compiled.crx.bin']
        self.assertEqual(call(cmd), 0)

    def runTest(self):
        self.compile()
        try:
            cmd = ['../apertium/apertium-restore-caps'] \
                + self.runFlags                         \
                + ['data/compiled.crx.bin']
            self.proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)

            for inp, exp in self.pairs:
                self.assertEqual(self.communicateFlush(inp+'[][\n]'),
                                 exp+'[][\n]')

            self.proc.communicate()
            self.proc.stdin.close()
            self.proc.stdout.close()
            self.proc.stderr.close()
            retCode = self.proc.poll()
            self.assertEqual(retCode, 0)

        finally:
            pass
