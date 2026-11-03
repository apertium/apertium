#!/bin/bash

curl https://raw.githubusercontent.com/unicode-org/icu/refs/heads/main/icu4c/source/data/unidata/norm2/nfc.txt -o nfc.txt
echo '# Non-destructive subset of NFC normalization' > nfcn.txt
echo '# based on https://github.com/unicode-org/icu/blob/main/icu4c/source/data/unidata/norm2/nfc.txt' >> nfcn.txt
grep -vE '>[^ ]+$' nfc.txt | grep -vE '^#' >> nfcn.txt
