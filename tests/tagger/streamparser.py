#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Usage: streamparser.py [FILE]

Consumes input from a file (first argument) or stdin, parsing and pretty printing the readings of lexical units found.
"""

import re, pprint, sys, itertools, fileinput
from enum import Enum
from collections import namedtuple


Knownness = Enum('Knownness', 'known unknown biunknown genunknown')
try:
    Knownness.__doc__ = """Level of knowledge associated with a lexical unit.
    Values:
        known
        unknown: Denoted by '*', analysis not available.
        biunknown: Denoted by '@', translation not available.
        genunknown: Denoted by '#', generated form not available.
"""
except AttributeError:
    # Python 3.2 users have to read the source
    pass

SReading = namedtuple('SReading', ['baseform', 'tags'])
try:
    SReading.__doc__ = """A single subreading of an analysis of a token.
    Fields:
        baseform (str): The base form (lemma, lexical form, citation form) of the reading.
        tags (list of str): The morphological tags associated with the reading.
"""
except AttributeError:
    # Python 3.2 users have to read the source
    pass

def subreadingToString(sub):
    return sub.baseform+"".join("<"+t+">" for t in sub.tags)

def readingToString(reading):
    return "+".join(subreadingToString(sub) for sub in reading)

def mainpos(reading, ltr=False):
    """Return the first part-of-speech tag of a reading. If there are
    several subreadings, by default give the first tag of the last
    subreading. If ltr=True, give the first tag of the first
    subreading, see
    http://beta.visl.sdu.dk/cg3/single/#sub-stream-apertium for more
    information.

    """
    if ltr:
        return reading[0].tags[0]
    else:
        return reading[-1].tags[0]

class LexicalUnit:

    """A lexical unit consisting of a lemma and its readings.

    Attributes:
        lexicalUnit (str): The lexical unit in Apertium stream format.
        wordform (str): The word form (surface form) of the lexical unit.
        readings (list of list of SReading): The analyses of the lexical unit with sublists containing all subreadings.
        knownness (Knownness): The level of knowledge of the lexical unit.
    """

    knownness = Knownness.known
    def __init__(self, lexicalUnit):
        self.lexicalUnit = lexicalUnit

        cohort = re.split(r'(?<!\\)/', lexicalUnit)
        self.wordform = cohort[0]
        readings = cohort[1:]

        self.readings = []
        for reading in readings:
            if len(reading) < 1:
                print("WARNING: Empty readings for {}".format(self.lexicalUnit), file=sys.stderr)
            elif reading[0] not in '*#@':
                subreadings = []

                subreadingParts = re.findall(r'([^<]+)((?:<[^>]+>)+)', reading)
                for subreading in subreadingParts:
                    baseform = subreading[0].lstrip('+')
                    tags = re.findall(r'<([^>]+)>', subreading[1])

                    subreadings.append(SReading(baseform=baseform, tags=tags))

                self.readings.append(subreadings)
            else:
                self.knownness = {'*': Knownness.unknown, '@': Knownness.biunknown, '#': Knownness.genunknown}[readings[0][0]]

    def __repr__(self):
        return self.lexicalUnit


def parse(stream, withText=False):
    """Generates lexical units from a character stream.

    Args:
        stream (iterable): A character stream containing lexical units, superblanks and other text.
        withText (bool, optional): A boolean defining whether to output preceding text with each lexical unit.

    Yields:
        LexicalUnit: The next lexical unit found in the character stream. (if withText is False)
        (str, LexicalUnit): The next lexical unit found in the character stream and the the text that seperated it from the prior unit in a tuple. (if withText is True)
    """

    buffer = ''
    textBuffer = ''
    inLexicalUnit = False
    inSuperblank = False

    for char in stream:

        if inSuperblank:
            if char == ']':
                inSuperblank = False
                textBuffer += char
            elif char == '\\':
                textBuffer += char
                textBuffer += next(stream)
            else:
                textBuffer += char
        elif inLexicalUnit:
            if char == '$':
                if withText:
                    yield (textBuffer, LexicalUnit(buffer))
                else:
                    yield LexicalUnit(buffer)
                buffer = ''
                textBuffer = ''
                inLexicalUnit = False
            elif char == '\\':
                buffer += char
                buffer += next(stream)
            else:
                buffer += char
        else:
            if char == '[':
                inSuperblank = True
                textBuffer += char
            elif char == '^':
                inLexicalUnit = True
            elif char == '\\':
                textBuffer += char
                textBuffer += next(stream)
            else:
                textBuffer += char


def parse_file(f, withText=False):
    """Generates lexical units from a file.

    Args:
        f (file): A file containing lexical units, superblanks and other text.

    Yields:
        LexicalUnit: The next lexical unit found in the file.
    """

    return parse(itertools.chain.from_iterable(f), withText)


if __name__ == '__main__':
    lexicalUnits = parse_file(fileinput.input())

    for lexicalUnit in lexicalUnits:
        pprint.pprint(lexicalUnit.readings, width=120)
