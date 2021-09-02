#!/usr/bin/env python3

import sys
import os
sys.path.append(os.path.realpath("."))

import unittest

import cleanstream
import tagger
import pretransfer
import transfer
import interchunk
import postchunk
import adaptdocx

if __name__ == "__main__":
    os.chdir(os.path.dirname(__file__))
    failures = 0
    for module in [tagger,
                   pretransfer,
                   transfer,
                   interchunk,
                   postchunk,
                   adaptdocx,
                   cleanstream]:
        suite = unittest.TestLoader().loadTestsFromModule(module)
        res = unittest.TextTestRunner(verbosity=2).run(suite)
        if(not(res.wasSuccessful())):
            failures += 1
    sys.exit(min(failures, 255))
