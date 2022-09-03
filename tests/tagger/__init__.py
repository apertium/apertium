#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import functools
import unittest
import tempfile
from os import devnull
from os.path import join as pjoin
from os.path import abspath, dirname
from subprocess import check_call, Popen, PIPE, CalledProcessError
from sys import stderr


import signal


# Utilities
def tmp(contents):
    t = tempfile.NamedTemporaryFile(mode='w', delete=False)
    t.write(contents)
    return t.name


def rel(fn):
    return abspath(pjoin(dirname(abspath(__file__)), fn))


DEBUG = False
APERTIUM_TAGGER = rel("../../apertium/apertium-tagger")
APERTIUM_FILTER_AMBIGUITY = rel("../../apertium/apertium-filter-ambiguity")


def check_output(*popenargs, **kwargs):
    # Essentially a copypasted version of check_output with input backported
    # for Python 3.2 Can be significantly abridged with Python 3.5's run(...)
    # or Python 3.3's check_output. Please remove and use stdlib version when
    # Python 3.2/Wheezy is deprecated.
    if 'stdout' in kwargs:
        raise ValueError('stdout argument not allowed, it will be overridden.')
    if 'input' in kwargs:
        if 'stdin' in kwargs:
            raise ValueError('stdin and input arguments may not both be used.')
        inputdata = kwargs['input']
        del kwargs['input']
        kwargs['stdin'] = PIPE
    else:
        inputdata = None
    with Popen(*popenargs, stdout=PIPE, **kwargs) as process:
        try:
            out, unused_err = process.communicate(inputdata)
        except:
            process.kill()
            process.wait()
            raise
        retcode = process.poll()
        if retcode:
            raise CalledProcessError(retcode, process.args, output=out)
    return out


def check_stderr(*popenargs, **kwargs):
    # Essentially a copypasted version of check_output.
    # Can be significantly abridged with Python 3.5's run(...)
    if 'stderr' in kwargs:
        raise ValueError('stderr argument not allowed, it will be overridden.')
    if 'input' in kwargs:
        if 'stdin' in kwargs:
            raise ValueError('stdin and input arguments may not both be used.')
        inputdata = kwargs['input']
        del kwargs['input']
        kwargs['stdin'] = PIPE
    else:
        inputdata = None
    with Popen(*popenargs, stderr=PIPE, **kwargs) as process:
        try:
            unused_output, err = process.communicate(inputdata)
        except:
            process.kill()
            process.wait()
            raise
        retcode = process.poll()
        if retcode:
            raise CalledProcessError(retcode, process.args, output=err)
    return err


def trace_dec(f):
    @functools.wraps(f)
    def inner(*args, **kwargs):
        if len(args) > 0 and DEBUG:
            print("run " + " ".join(args[0]))
        return f(*args, **kwargs)
    return inner


def trace_plus_unicode(f):
    return functools.partial(trace_dec(f), universal_newlines=True)


check_call = trace_plus_unicode(check_call)
check_output = trace_plus_unicode(check_output)
check_stderr = trace_plus_unicode(check_stderr)

# Test files
DIC = """
^the/the<det><def><sp>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^has/have<vbhaver><pres><p3><sg>$
^booked/book<vblex><pp>/book<vblex><past>$
^close/close<adj><sint>/close<n><sg>/close<vblex><inf>/close<vblex><pres>/close<vblex><imp>$
^cat/cat<n><sg>$
^room/room<n><sg>$
^red/red<adj><sint>$
^./.<sent>$
""".strip()

TSX = """
<?xml version="1.0" encoding="utf-8"?>
<tagger name="test">
  <tagset>
    <def-label name="DET" closed="true">
      <tags-item tags="det.*"/>
      <tags-item tags="det.*.*"/>
    </def-label>
    <def-label name="VERB">
      <tags-item tags="vblex.*"/>
      <tags-item tags="vbhaver.*"/>
    </def-label>
    <def-label name="NOUN">
      <tags-item tags="n.*"/>
    </def-label>
    <def-label name="ADJ">
      <tags-item tags="adj.*"/>
      <tags-item tags="adj"/>
    </def-label>
  </tagset>
</tagger>
""".strip()

TRAIN_NO_PROBLEM_UNTAGGED = """
^The/the<det><def><sp>$
^cat/cat<n><sg>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$

^The/the<det><def><sp>$
^red/red<adj><sint>$
^cat/cat<n><sg>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^red/red<adj><sint>$
^room/room<n><sg>$
^./.<sent>$

^The/the<det><def><sp>$
^red/red<adj><sint>$
^cat/cat<n><sg>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$
""".strip()

TRAIN_NO_PROBLEM_TAGGED = """
^The/the<det><def><sp>$
^cat/cat<n><sg>$
^books/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$

^The/the<det><def><sp>$
^red/red<adj><sint>$
^cat/cat<n><sg>$
^books/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^red/red<adj><sint>$
^room/room<n><sg>$
^./.<sent>$

^The/the<det><def><sp>$
^red/red<adj><sint>$
^cat/cat<n><sg>$
^books/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$
""".strip()

TRAIN_CAT_TO_BE_A_VERB_UNTAGGED = """
^The/The<det><def><sp>$
^falling/fall<vblex><pprs>/fall<vblex><ger>/fall<vblex><subs>$
^cat/cat<n><sg>$
^has/have<vbhaver><pres><p3><sg>$
^booked/book<vblex><pp>/book<vblex><past>$
^books/book<n><pl>/book<vblex><pres><p3><sg>$
^./.<sent>$

^Close/close<adj><sint>/close<n><sg>/close<vblex><inf>/close<vblex><pres>/close<vblex><imp>$
^the/the<det><def><sp>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^./.<sent>$

^The/the<det><def><sp>$
^falling/fall<vblex><pprs>/fall<vblex><ger>/fall<vblex><subs>$
^cat/cat<n><sg>$
^has/have<vbhaver><pres><p3><sg>$
^books/book<n><pl>/book<vblex><pres><p3><sg>$
^./.<sent>$
""".strip()

TRAIN_CAT_TO_BE_A_VERB_TAGGED = """
^The/The<det><def><sp>$
^falling/fall<vblex><pprs>$
^cat/cat<n><sg>$
^has/have<vbhaver><pres><p3><sg>$
^booked/book<vblex><pp>$
^books/book<n><pl>$
^./.<sent>$

^Close/close<vblex><imp>$
^the/the<det><def><sp>$
^books/book<n><pl>$
^./.<sent>$

^The/the<det><def><sp>$
^falling/fall<vblex><pprs>$
^cat/cat<n><sg>$
^has/have<vbhaver><pres><p3><sg>$
^books/book<n><pl>$
^./.<sent>$
""".strip()

TEST_SUCCESS = """
^The/the<det><def><sp>$
^cat/cat<n><sg>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$
""".strip()

TEST_NEW_AMBG_CLASS = """
^The/the<det><def><sp>$
^cat/cat<n><sg>/cat<adj>$
^books/book<n><pl>/book<vblex><pri><p3><sg>$
^the/the<det><def><sp>$
^room/room<n><sg>$
^./.<sent>$
""".strip()

# Expected strings
EXPECTED_SUBST = """
Error: A new ambiguity class was found.
Retraining the tagger is necessary so as to take it into account.
Word 'cat'.
New ambiguity class: {NOUN,ADJ}
""".strip().split("\n")


# Tests
class AmbiguityClassTest(unittest.TestCase):
    def setUp(self):
        self.tsx_fn = tmp(TSX)
        self.dic_fn = tmp(DIC)
        self.devnull = open(devnull, 'w')

    def changing_class_impl(self, flags, model_fn):
        test1 = tmp(TEST_SUCCESS)
        test2 = tmp(TEST_NEW_AMBG_CLASS)
        success_stderr = check_stderr(
            [APERTIUM_TAGGER, '-d'] + flags +
            ['-g', model_fn, test1],
            stdout=self.devnull)
        self.assertEqual(success_stderr.strip(), "")
        subst_stderr = check_stderr(
            [APERTIUM_TAGGER, '-d'] + flags +
            ['-g', model_fn, test2],
            stdout=self.devnull)
        subst_stderr = [line.strip()
                        for line in subst_stderr.strip().split("\n")]
        self.assertEqual(subst_stderr, EXPECTED_SUBST)
        ambg_class = check_output(
           [rel('test-find-similar-ambiguity-class'), model_fn],
           input="NOUN ADJ\n")
        substituted_class = set(ambg_class.split(" "))
        # Should get open class
        self.assertSetEqual(substituted_class, set(("VERB", "NOUN", "ADJ")))

    def test_changing_class_hmm_sup(self):
        model_fn = tmp("")
        untagged = tmp(TRAIN_NO_PROBLEM_UNTAGGED)
        tagged = tmp(TRAIN_NO_PROBLEM_TAGGED)
        check_call(
            [APERTIUM_TAGGER, '-s', '0', self.dic_fn, untagged, self.tsx_fn,
             model_fn, tagged, untagged],
            stderr=self.devnull)
        self.changing_class_impl([], model_fn)

    def test_changing_class_hmm_unsup(self):
        model_fn = tmp("")
        untagged = tmp(TRAIN_NO_PROBLEM_UNTAGGED)
        check_call(
            [APERTIUM_TAGGER, '-t', '1', self.dic_fn, untagged, self.tsx_fn,
             model_fn],
            stderr=self.devnull)
        self.changing_class_impl([], model_fn)

    def test_changing_class_sliding_window(self):
        model_fn = tmp("")
        untagged = tmp(TRAIN_NO_PROBLEM_UNTAGGED)
        check_call(
            [APERTIUM_TAGGER, '--sliding-window', '-t', '1', self.dic_fn,
             untagged, self.tsx_fn, model_fn],
            stderr=self.devnull)
        self.changing_class_impl(['--sliding-window'], model_fn)

    def test_cat_is_a_verb(self):
        model_fn = tmp("")
        untagged = tmp(TRAIN_CAT_TO_BE_A_VERB_UNTAGGED)
        tagged = tmp(TRAIN_CAT_TO_BE_A_VERB_TAGGED)
        new_ambg_class = tmp(TEST_NEW_AMBG_CLASS)
        check_call(
            [APERTIUM_TAGGER, '-s', '0', self.dic_fn, untagged, self.tsx_fn,
             model_fn, tagged, untagged],
            stderr=self.devnull)
        subst_stdout = check_output(
            [APERTIUM_TAGGER, '-d', '-g', model_fn, new_ambg_class],
            stderr=self.devnull)
        acceptable = False
        for line in subst_stdout.split("\n"):
            if (line.startswith('^cat') and ('<adj>' in line or '<n>' in line)):
                acceptable = True
        self.assertTrue(
            acceptable,
            "'cat' must be output and tagged as an adjective or a noun.\n" +
            "Actual output:\n{}".format(subst_stdout))

    @unittest.expectedFailure
    def test_g_vs_plus(self):
        inp = tmp("""
^TV-karriere/TV<np><al><cmp>+karriere<n><m><sg><ind>/Tv<n><m><sg><ind><cmp>+karriere<n><m><sg><ind>$
""".strip())
        subst_stdout = check_output(
            [APERTIUM_TAGGER, '-g', "data/nob.prob", inp],
            stderr=self.devnull)
        out = subst_stdout.split("\n")
        wantanyof = {'^TV<np><al><cmp>+karriere<n><m><sg><ind>$',
                     '^Tv<n><m><sg><ind><cmp>+karriere<n><m><sg><ind>$'}
        acceptable = set(out).intersection(wantanyof)
        msg = ("Couldn't find any of \n{}\nin output.\n" +
               "Actual output:\n {}\n").format(wantanyof,
                                               subst_stdout)
        self.assertTrue(bool(acceptable), msg)

    def test_undefined_mult_vs_plus(self):
        inp = tmp("""
^yz/y<n><f><cmp>+z<n><ut>$
""".strip())
        subst_stdout = check_output(
            [APERTIUM_TAGGER, '-g', "data/nob.prob", inp],
            stderr=self.devnull)
        out = subst_stdout.split("\n")
        wantanyof = {'^y<n><f><cmp>+z<n><ut>$',
                     }
        acceptable = set(out).intersection(wantanyof)
        msg = ("Couldn't find any of \n{}\nin output.\n" +
               "Actual output:\n {}\n").format(wantanyof,
                                               subst_stdout)
        self.assertTrue(bool(acceptable), msg)


class FilterAmbiguityTest(unittest.TestCase):
    def test_unicode_tsx(self):
        stderrpath = tmp('')
        inputpath = tmp('^아/아<noun>$\n')
        output = check_output(
            [APERTIUM_FILTER_AMBIGUITY, rel('unicode.tsx'), inputpath],
            stderr=open(stderrpath, 'w'))
        self.assertEqual('^아/아<noun>$\n', output)
        stderr = open(stderrpath, 'r').read().strip()
        self.assertEqual('', stderr)

class Alarm(Exception):
    pass

class PerceptronNullFlushTest(unittest.TestCase):

    inputs = [
        "^this/this<prn><dem><mf><sg>$ ^here/here<adv>$",
        "^is/be<vbser><pres><p3><sg>$ ^a/a<det><ind><sg>$",
        "^little/little<adj>/little<adv>$",
        "^flushing/*flushing$ ^test/test<n><sg>/test<vblex><inf>/test<vblex><pres>/test<vblex><imp>$"
    ]

    outputs = [
        "^this<prn><dem><mf><sg>$ ^here<adv>$",
        "^be<vbser><pres><p3><sg>$ ^a<det><ind><sg>$",
        "^little<adv>$",
        "^*flushing$ ^test<n><sg>$"
    ]


    def alarmHandler(self, signum, frame):
        raise Alarm

    def withTimeout(self, seconds, cmd, *args, **kwds):
        signal.signal(signal.SIGALRM, self.alarmHandler)
        signal.alarm(seconds)
        ret = cmd(*args, **kwds)
        signal.alarm(0)         # reset the alarm
        return ret

    def communicateFlush(self, string, process):
        if string:
            process.stdin.write(string.encode('utf-8'))
            process.stdin.write(b'\0')
            process.stdin.flush()

        output = []
        char = None
        try:
            char = self.withTimeout(2, process.stdout.read, 1)
        except Alarm:
            print("Timeout before reading a single character!", file=stderr)
        while char and char != b'\0':
            output.append(char)
            try:
                char = self.withTimeout(2, process.stdout.read, 1)
            except Alarm:
                print("Timeout before reading %s chars" % len(output),
                        file=stderr)
                break           # send what we got up till now

        return b"".join(output).decode('utf-8').replace('\r\n', '\n')

    def test_null_flush(self):
        try:
            proc = Popen([APERTIUM_TAGGER, '-xg', '-z', "data/eng.prob"],
                                stdin=PIPE,
                                stdout=PIPE,
                                stderr=PIPE)

            for inp, exp in zip(self.inputs, self.outputs):
                output = self.communicateFlush(inp+"[][\n]", proc)
                self.assertEqual(output, exp+"[][\n]")

            proc.communicate() # let it terminate
            proc.stdin.close()
            proc.stdout.close()
            proc.stderr.close()
            retCode = proc.poll()
            self.assertEqual(retCode, 0)

        finally:
            pass
