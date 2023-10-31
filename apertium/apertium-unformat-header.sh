#!/usr/bin/env bash

PATH="${APERTIUM_PATH}:${PATH}"
INPUT_FILE="/dev/stdin"
OUTPUT_FILE="/dev/stdout"

[ -z "$TMPDIR" ] && TMPDIR=/tmp


message ()
{
  formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "unformat_desc" "basename" "$(basename "$0")"
  exit 1;
}

locale_utf8 ()
{
  LC_CTYPE=$(locale -a|grep -i "utf[.]*8"|head -1)
  export LC_CTYPE
  if [ "$LC_CTYPE" = "" ]
  then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81580";
       exit 1;
  fi
}

test_zip ()
{
  if ! command -v zip &>/dev/null; then
    formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81610" "command" "zip";
    exit 1;
  fi

  if ! command -v unzip &>/dev/null; then
    formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81610" "command" "unzip";
    exit 1;
  fi
}

test_gawk ()
{
  if ! command -v gawk &>/dev/null; then
    formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81610" "command" "gawk"
    exit 1
  fi
}

unformat_latex()
{
  test_gawk

  if [ "$FICHERO" = "" ]
  then FICHERO=$(mktemp "$TMPDIR"/apertium.XXXXXXXX)
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi

  apertium-prelatex "$FICHERO" | \
  apertium-utils-fixlatex | \
  apertium-deslatex  >"$SALIDA"

  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO"
  fi
}


unformat_odt ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR"/apertium.XXXXXXXX)

  locale_utf8
  test_zip

  unzip -q -o -d "$INPUT_TMPDIR" "$FICHERO"
  find "$INPUT_TMPDIR" | grep content\\\.xml |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  apertium-desodt >"$SALIDA"
  rm -Rf "$INPUT_TMPDIR"
}

unformat_docx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR"/apertium.XXXXXXXX)

  locale_utf8
  test_zip

  unzip -q -o -d "$INPUT_TMPDIR" "$FICHERO"

  for i in $(find "$INPUT_TMPDIR"|grep "xlsx$");
  do LOCALTEMP=$(mktemp "$TMPDIR"/apertium.XXXXXXXX)
     apertium -f xlsx -d "$DIRECTORY" "$OPCIONU" "$PREFIJO" <"$i" >"$LOCALTEMP";
     cp "$LOCALTEMP" "$i";
     rm "$LOCALTEMP";
  done;

  find "$INPUT_TMPDIR" | grep "xml" |\
  grep -v -i \\\(settings\\\|theme\\\|styles\\\|font\\\|rels\\\|docProps\\\) |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  apertium-deswxml >"$SALIDA"
  rm -Rf "$INPUT_TMPDIR"
}

unformat_pptx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR"/apertium.XXXXXXXX)

  locale_utf8
  test_zip

  unzip -q -o -d "$INPUT_TMPDIR" "$FICHERO"

  for i in $(find "$INPUT_TMPDIR"|grep "xlsx$");
  do LOCALTEMP=$(mktemp "$TMPDIR"/apertium.XXXXXXXX)
     apertium -f xlsx -d "$DIRECTORY" "$OPCIONU" "$PREFIJO" <"$i" >"$LOCALTEMP"
     cp "$LOCALTEMP" "$i"
     rm "$LOCALTEMP"
  done;

  find . -path '**/slides/slide*.xml' |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  apertium-despptx >"$SALIDA"
  rm -Rf "$INPUT_TMPDIR"
}


unformat_xlsx ()
{
  INPUT_TMPDIR=$(mktemp -d "$TMPDIR"/apertium.XXXXXXXX)

  locale_utf8
  test_zip

  unzip -q -o -d "$INPUT_TMPDIR" "$FICHERO"
  find "$INPUT_TMPDIR" | grep "sharedStrings.xml" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  apertium-desxlsx >"$SALIDA"
  rm -Rf "$INPUT_TMPDIR"

}


while getopts "f:" opt; do
    case "$opt" in
        f) FORMAT=$OPTARG ;;
        \?) formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81080" "opt" "$OPTARG" >&2; message >&2 ;;
        :) formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81620" "opt" "$OPTARG" >&2; message >&2 ;;
    esac
done

shift "$((OPTIND-1))"

case "$#" in
     2)
       OUTPUT_FILE=$2;
       INPUT_FILE=$1;
       if [ ! -e "$INPUT_FILE" ];
       then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$INPUT_FILE"
            message;
       fi
       ;;
     1)
       INPUT_FILE=$1;
       if [ ! -e "$INPUT_FILE" ];
       then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$INPUT_FILE"
            message;
       fi
       ;;
     0)
       ;;
     *)
       message
       ;;
esac

if [ -z "$FORMAT" ]; then FORMAT="txt"; fi

FORMATADOR=$FORMAT;
FICHERO=$INPUT_FILE;
SALIDA=$OUTPUT_FILE;


case "$FORMATADOR" in
        rtf)
                MILOCALE=$(locale -a | grep -E -i -v -m1 'utf|^C|^POSIX$')
		if [ "$MILOCALE" = "" ]
		then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81660";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
        html-noent)
        	FORMATADOR="html"
        	;;

        latex)
                unformat_latex
                exit 0
                ;;

        odt)
		unformat_odt
		exit 0
		;;
	docx)
		unformat_docx
		exit 0
		;;
	xlsx)
		unformat_xlsx
		exit 0
		;;
	pptx)
		unformat_pptx
		exit 0
		;;

	wxml)
	        locale_utf8
	        ;;
	*)
	        ;;

esac

apertium-des"$FORMATADOR" "$FICHERO" >"$SALIDA"
