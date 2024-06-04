#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from subprocess import Popen, PIPE, call

import signal

class Alarm(Exception):
    pass

class IOTest:
    expectedRetCodeFail = False

    def alarmHandler(self, signum, frame):
        raise Alarm

    def withTimeout(self, seconds, cmd, *args, **kwargs):
        signal.signal(signal.SIGALRM, self.alarmHandler)
        signal.alarm(seconds)
        ret = cmd(*args, **kwargs)
        signal.alarm(0) # reset
        return ret

    def communicateFlush(self, string):
        self.proc.stdin.write(string.encode('utf-8') + b'\0')
        self.proc.stdin.flush()

        output = b''
        c = None
        while c != b'\0':
            try:
                c = self.withTimeout(2, self.proc.stdout.read, 1)
            except Alarm:
                break
            if c != b'\0':
                output += c

        return output.decode('utf-8')

    def checkCommand(self, cmd):
        self.assertEqual(0, call(cmd))

    def runCommand(self, cmd, pairs):
        self.proc = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)

        blank = '[][\n]'
        for inp, exp in pairs:
            with self.subTest(input_line=inp):
                self.assertEqual(exp+blank, self.communicateFlush(inp+blank))

        self.proc.communicate() # let it terminate
        self.proc.stdin.close()
        self.proc.stdout.close()
        self.proc.stderr.close()

        retCode = self.proc.poll()
        if self.expectedRetCodeFail:
            self.assertNotEqual(0, retCode)
        else:
            self.assertEqual(0, retCode)
