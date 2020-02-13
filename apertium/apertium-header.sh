#!/bin/bash
# -*- sh-basic-offset: 2 -*-

# Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.


message ()
{
  echo "USAGE: $(basename "$0") [-d datadir] [-f format] [-u] <direction> [in [out]]"
  echo " -d datadir       directory of linguistic data"
  echo " -f format        one of: txt (default), html, rtf, odt, docx, wxml, xlsx, pptx,"
  echo "                  xpresstag, html-noent, html-alt, latex, latex-raw, line"
  echo " -a               display ambiguity"
  echo " -u               don't display marks '*' for unknown words"
  echo " -n               don't insert period before possible sentence-ends"
  echo " -m memory.tmx    use a translation memory to recycle translations"
  echo " -o direction     translation direction using the translation memory,"
  echo "                  by default 'direction' is used instead"
  echo " -l               lists the available translation directions and exits"
  echo " direction        typically, LANG1-LANG2, but see modes.xml in language data"
  echo " in               input file (stdin by default)"
  echo " out              output file (stdout by default)"
  exit 1
}

list_directions ()
{
  for mode in "$DATADIR"/modes/*.mode; do
    echo "  $(basename "${mode%%.mode}")"
  done
}

locale_utf8 ()
{
  export LC_CTYPE
  LC_CTYPE=$(locale -a|grep -i "utf[.-]*8"|head -1)
  if [[ -z ${LC_CTYPE} ]]; then
    echo "Error: Install an UTF-8 locale in your system"
    exit 1
  fi
}

check_encoding () {
  local file=$1
  local encoding
  encoding=$(file -b --mime-encoding "${file}")
  if [[ "${encoding}" != utf-8 && "${encoding}" != us-ascii ]]; then
    echo "Input seems to be non-UTF-8, please convert to UTF-8 (e.g. with 'iconv -f ${encoding} -t utf-8')" >&2
    exit 1
  fi
}

test_zip ()
{
  if ! command -v zip &>/dev/null; then
    echo "Error: Install 'zip' command in your system";
    exit 1;
  fi

  if ! command -v unzip &>/dev/null; then
    echo "Error: Install 'unzip' command in your system";
    exit 1;
  fi
}

test_gawk ()
{
  if ! command -v gawk &>/dev/null; then
    echo "Error: Install 'gawk' in your system"
    exit 1
  fi
}

translate_latex()
{
  test_gawk

  if [[ -z "$INFILE" || "$INFILE" = /dev/stdin ]]; then
    INFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    cat > "$INFILE"
    BORRAFICHERO="true"
  fi
  check_encoding "${INFILE}"
  locale_utf8

  set -o pipefail
  "$APERTIUM_PATH/apertium-prelatex" "$INFILE" | \
    "$APERTIUM_PATH/apertium-utils-fixlatex" | \
    "$APERTIUM_PATH/apertium-deslatex" "${FORMAT_OPTIONS[@]}" | \
    if [ "$TRANSLATION_MEMORY_FILE" = "" ];
    then cat;
    else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
    fi | \
      if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
      sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
    else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
    fi | \
      "$APERTIUM_PATH/apertium-relatex"| \
      awk '{gsub("</CONTENTS-noeos>", "</CONTENTS>"); print;}' | \
      if [ "$REDIR" == "" ]; then "$APERTIUM_PATH/apertium-postlatex-raw"; else "$APERTIUM_PATH/apertium-postlatex-raw" > "$SALIDA"; fi

    if [ "$BORRAFICHERO" = "true" ]; then
      rm -Rf "$INFILE"
    fi
}

# TODO: What was the intended difference between this and
# translate_latex? They were identical, apart from one not cleaning up
# tmp files
translate_latex_raw()
{
  translate_latex
}


translate_odt ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR/apertium.XXXXXXXX")

  locale_utf8
  test_zip

  if [ "$INFILE" = "" ]; then
    INFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    cat > "$INFILE"
    BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")

  unzip -q -o -d "$INPUT_TMPDIR" "$INFILE"
  find "$INPUT_TMPDIR" -name content.xml -o -name styles.xml |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  "$APERTIUM_PATH/apertium-desodt" "${FORMAT_OPTIONS[@]}" |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ];
  then cat;
  else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
  fi | \
    if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
    sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | \
    "$APERTIUM_PATH/apertium-reodt"|\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd "$INPUT_TMPDIR"
  rm -Rf ObjectReplacements
  zip -q -r - . >"$OTRASALIDA"
  cd "$VUELVE"
  rm -Rf "$INPUT_TMPDIR"

  if [ "$BORRAFICHERO" = "true" ]; then
    rm -Rf "$INFILE";
  fi

  if [ "$REDIR" == "" ]; then cat "$OTRASALIDA"; else cat "$OTRASALIDA" > "$SALIDA"; fi
  rm -Rf "$OTRASALIDA"
}

translate_docx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR/apertium.XXXXXXXX")

  locale_utf8
  test_zip

  if [ "$INFILE" = "" ]; then
    INFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    cat > "$INFILE"
    BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")

  if [ "$UWORDS" = "no" ]; then
    OPCIONU="-u";
  else OPCIONU="";
  fi

  unzip -q -o -d "$INPUT_TMPDIR" "$INFILE"

  find "$INPUT_TMPDIR" -name "*.xlsx" -print0 | while read -r -d '' i; do
    LOCALTEMP=$(mktemp "$TMPDIR/apertium.XXXXXXXX");
    "$APERTIUM_PATH/apertium" -f xlsx -d "$DATADIR" "$OPCIONU" "$PAIR" <"$i" >"$LOCALTEMP";
    cp "$LOCALTEMP" "$i";
    rm "$LOCALTEMP";
  done;

  find "$INPUT_TMPDIR" -name "*.xml" |\
  grep -E -v -i '(settings|theme|styles|font|rels|docProps)' |\
  "$APERTIUM_PATH/adapt-docx" -n |\
  "$APERTIUM_PATH/apertium-deswxml" "${FORMAT_OPTIONS[@]}" |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ];
  then cat;
  else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
  fi | \
    if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
    sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | \
    "$APERTIUM_PATH/apertium-rewxml"|\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd "$INPUT_TMPDIR"
  zip -q -r - . >"$OTRASALIDA"
  cd "$VUELVE"
  rm -Rf "$INPUT_TMPDIR"

  if [ "$BORRAFICHERO" = "true" ]; then
    rm -Rf "$INFILE";
  fi

  if [ "$REDIR" == "" ]; then cat "$OTRASALIDA"; else cat "$OTRASALIDA" > "$SALIDA"; fi
  rm -Rf "$OTRASALIDA"
}

translate_pptx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR/apertium.XXXXXXXX")

  locale_utf8
  test_zip

  if [ "$INFILE" = "" ]; then
    INFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    cat > "$INFILE"
    BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")

  if [ "$UWORDS" = "no" ]; then
    OPCIONU="-u";
  else OPCIONU="";
  fi

  unzip -q -o -d "$INPUT_TMPDIR" "$INFILE"

  find "$INPUT_TMPDIR" -name "*.xlsx" -print0 | while read -r -d '' i; do
    LOCALTEMP=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    "$APERTIUM_PATH/apertium" -f xlsx -d "$DATADIR" "$OPCIONU" "$PAIR" <"$i" >"$LOCALTEMP";
    cp "$LOCALTEMP" "$i"
    rm "$LOCALTEMP"
  done;

  find . -path '**/slides/slide*.xml' |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  "$APERTIUM_PATH/apertium-despptx" "${FORMAT_OPTIONS[@]}" |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ];
  then cat;
  else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
  fi | \
    if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
    sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | \
    "$APERTIUM_PATH/apertium-repptx" |\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd "$INPUT_TMPDIR"
  zip -q -r - . >"$OTRASALIDA"
  cd "$VUELVE"
  rm -Rf "$INPUT_TMPDIR"

  if [ "$BORRAFICHERO" = "true" ]; then
    rm -Rf "$INFILE";
  fi

  if [ "$REDIR" == "" ]; then cat "$OTRASALIDA"; else cat "$OTRASALIDA" > "$SALIDA"; fi
  rm -Rf "$OTRASALIDA"
}


translate_xlsx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR/apertium.XXXXXXXX")

  locale_utf8
  test_zip

  if [ "$INFILE" = "" ]; then
    INFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
    cat > "$INFILE"
    BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")

  unzip -q -o -d "$INPUT_TMPDIR" "$INFILE"
  find "$INPUT_TMPDIR" -name "sharedStrings.xml" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  "$APERTIUM_PATH/apertium-desxlsx" "${FORMAT_OPTIONS[@]}" |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ];
  then cat;
  else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
  fi | \
    if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
    sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | \
    "$APERTIUM_PATH/apertium-rexlsx" |\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd "$INPUT_TMPDIR"
  zip -q -r - . >"$OTRASALIDA"
  cd "$VUELVE"
  rm -Rf "$INPUT_TMPDIR"

  if [ "$BORRAFICHERO" = "true" ]; then
    rm -Rf "$INFILE";
  fi

  if [ "$REDIR" == "" ]; then cat "$OTRASALIDA"; else cat "$OTRASALIDA" > "$SALIDA"; fi
  rm -Rf "$OTRASALIDA"
}

translate_htmlnoent ()
{
  "$APERTIUM_PATH/apertium-deshtml" "${FORMAT_OPTIONS[@]}" "$INFILE" | \
      if [ "$TRANSLATION_MEMORY_FILE" = "" ]; then
          cat
      else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
      fi | if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
      sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | if [ "$FORMAT" = "none" ]; then
      if [ "$REDIR" == "" ]; then cat; else cat > "$SALIDA"; fi
  else
    if [ "$REDIR" == "" ]; then "$APERTIUM_PATH/apertium-rehtml-noent"; else "$APERTIUM_PATH/apertium-rehtml-noent" > "$SALIDA"; fi
  fi
}

translate_htmlalt ()
{
  "$APERTIUM_PATH/apertium-deshtml-alt" "${FORMAT_OPTIONS[@]}" "$INFILE" | \
      if [ "$TRANSLATION_MEMORY_FILE" = "" ]; then
          cat
      else "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
      fi | if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
      sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  else "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
  fi | if [ "$FORMAT" = "none" ]; then
      if [ "$REDIR" == "" ]; then cat; else cat > "$SALIDA"; fi
  else
    if [ "$REDIR" == "" ]; then "$APERTIUM_PATH/apertium-rehtml-noent"; else "$APERTIUM_PATH/apertium-rehtml-noent" > "$SALIDA"; fi
  fi
}

translate_line ()
{
  # TODO: lt-proc inserts spaces before parts of mwe's that cross
  # lines, even though the parts get output as individual lu's
  "$APERTIUM_PATH/apertium-destxt" -n "$INFILE" | \
      sed 's/[[:space:]]*\[$/[][/' |\
      if [ "$TRANSLATION_MEMORY_FILE" = "" ]; then
          cat
      else
        "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE";
      fi | \
          if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
              sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
          else
            "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
          fi | \
              if [ "$REDIR" == "" ]; then
                  "$APERTIUM_PATH/apertium-retxt"
              else
                  "$APERTIUM_PATH/apertium-retxt" > "$SALIDA"
              fi
}




##########################################################
# Option and argument parsing, setting globals variables #
##########################################################
PATH="${APERTIUM_PATH}:${PATH}"
[[ -z $TMPDIR ]] && TMPDIR=/tmp
TMCOMPFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
trap 'rm -Rf "$TMCOMPFILE"' EXIT

# Default values, may be overridden below:
PAIR=""
INFILE="/dev/stdin"
FORMAT="txt"
DATADIR=$DEFAULT_DIRECTORY
TRANSLATION_MEMORY_DIRECTION=$PAIR
LIST_MODES_AND_EXIT=false
FORMAT_OPTIONS=()

# Skip (but store) non-option arguments that come before options:
declare -a ARGS_PREOPT ARGS_ALL
declare -i OPTIND=1
ARGS_ALL=( "$@" )              # so we can index into it with a variable
while [[ $OPTIND -le $# ]]; do
  arg=${ARGS_ALL[$OPTIND-1]}
  case $arg in
    -*) break ;;
    *) ARGS_PREOPT+=( "$arg" ); (( OPTIND++ )) ;;
  esac
done


while getopts ":uahlf:d:m:o:n" opt; do
  case "$opt" in
    f) FORMAT=$OPTARG ;;
    d) DATADIR=$OPTARG ;;
    m) TRANSLATION_MEMORY_FILE=$OPTARG ;;
    o) TRANSLATION_MEMORY_DIRECTION=$OPTARG ;;
    u) UWORDS="no" ;;
    n) FORMAT_OPTIONS+=(-n) ;;
    a) OPTION_TAGGER="-m" ;;
    l) LIST_MODES_AND_EXIT=true ;;
    h) message ;;
    \?) echo "ERROR: Unknown option $OPTARG" >&2; message >&2 ;;
    :) echo "ERROR: $OPTARG requires an argument" >&2; message >&2 ;;
  esac
done
shift $(( OPTIND-1 ))

if $LIST_MODES_AND_EXIT; then list_directions; exit 0; fi

# Restore non-option arguments that came before options back into arg list:
set -- "${ARGS_PREOPT[@]}" "$@"

case "$#" in
  3)
    SALIDA=$3
    REDIR=">"
    INFILE=$2
    PAIR=$1
    ;;
  2)
    INFILE=$2
    PAIR=$1
    ;;
  1)
    PAIR=$1
    ;;
  *)
    message >&2
    ;;
esac

if [[ ! -e "$INFILE" ]]; then
  echo "Error: file '$INFILE' not found." >&2
  message >&2
elif [[ ! -r "$INFILE" ]]; then
  echo "Error: file '$INFILE' is not readable by you." >&2
  message >&2
fi

if [[ -n $TRANSLATION_MEMORY_FILE ]]; then
  if ! "$APERTIUM_PATH/lt-tmxcomp" "$TRANSLATION_MEMORY_DIRECTION" "$TRANSLATION_MEMORY_FILE" "$TMCOMPFILE" >/dev/null; then
    echo "Error: Cannot compile TM '$TRANSLATION_MEMORY_FILE'" >&2
    echo"   hint: use -o parameter" >&2
    message >&2
  fi
fi

if [[ ! -d "$DATADIR/modes" ]]; then
  echo "Error: Directory '$DATADIR/modes' does not exist." >&2
  message >&2
fi

if [[ ! -e "$DATADIR/modes/$PAIR.mode" ]]; then
  echo -n "Error: Mode $PAIR does not exist"
  c=$(find "$DATADIR/modes" -name '*.mode' | wc -l)
  if [ "$c" -le 1 ]; then
    echo "."
  else
    echo ". Try one of:"
    list_directions
  fi
  exit 1
fi

#Parametro opcional, de no estar, lee de la entrada estandar (stdin)

case "$FORMAT" in
  none)
    if [ "$UWORDS" = "no" ]; then
      OPTION="-n";
    else OPTION="-g";
    fi
    ;;
  txt|rtf|html|xpresstag|mediawiki)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    ;;
  odt)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_odt
    exit 0
    ;;
  latex)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_latex
    exit 0
    ;;
  latex-raw)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_latex_raw
    exit 0
    ;;
  line)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_line
    exit 0
    ;;


  docx)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_docx
    exit 0
    ;;
  xlsx)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_xlsx
    exit 0
    ;;
  pptx)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_pptx
    exit 0
    ;;
  html-noent)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_htmlnoent
    exit 0
    ;;
  html-alt)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    translate_htmlalt
    exit 0
    ;;

  wxml)
    if [ "$UWORDS" = "no" ]; then OPTION="-n";
    else OPTION="-g";
    fi;
    locale_utf8
    ;;

  txtu)
    FORMAT="txt";
    OPTION="-n"
    ;;
  htmlu)
    FORMAT="html";
    OPTION="-n";
    ;;
  xpresstagu)
    FORMAT="xpresstag";
    OPTION="-n";
    ;;
  rtfu)
    FORMAT="rtf";
    OPTION="-n";
    MILOCALE=$(locale -a | grep -E -i -v -m1 'utf|^C|^POSIX$')
    if [ "$MILOCALE" = "" ]; then
      echo "Error: Install a ISO-8859-1 compatible locale in your system";
      exit 1;
    fi
    export LC_CTYPE=$MILOCALE
    ;;

  odtu)
    OPTION="-n"
    translate_odt
    exit 0
    ;;

  docxu)
    OPTION="-n"
    translate_docx
    exit 0
    ;;

  xlsxu)
    OPTION="-n"
    translate_xlsx
    exit 0
    ;;

  pptxu)
    OPTION="-n"
    translate_pptx
    exit 0
    ;;

  wxmlu)
    OPTION="-n";
    locale_utf8
    ;;



  *) # Por defecto asumimos txt
    echo "$0 WARNING: Unknown format ${FORMAT}, treating as 'txt'" >&2
    FORMAT="txt"
    OPTION="-g"
    ;;
esac

if [ -z "$REF" ]
then
    REF=$FORMAT
fi

set -e -o pipefail

if [ "$FORMAT" = "none" ]; then
    cat "$INFILE"
else
  "$APERTIUM_PATH/apertium-des$FORMAT" "${FORMAT_OPTIONS[@]}" "$INFILE"
fi | if [ "$TRANSLATION_MEMORY_FILE" = "" ];
     then
         cat
     else
       "$APERTIUM_PATH/lt-tmxproc" "$TMCOMPFILE"
     fi | if [ ! -x "$DATADIR/modes/$PAIR.mode" ]; then
              sh "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
          else
            "$DATADIR/modes/$PAIR.mode" "$OPTION" "$OPTION_TAGGER"
          fi | if [ "$FORMAT" = "none" ]; then
                   if [ "$REDIR" = "" ]; then
                       cat
                   else
                     cat > "$SALIDA"
                   fi
               else
                 if [ "$REDIR" = "" ]; then
                     "$APERTIUM_PATH/apertium-re$FORMAT"
                 else
                   "$APERTIUM_PATH/apertium-re$FORMAT" > "$SALIDA"
                 fi
               fi

