PATH="${APERTIUM_PATH}:${PATH}"

if [ $# != 2 ]
then if [ $# != 3 ]
     then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" \
          "deformat_desc" "first_line" "$(basename "$0") -[aAmM] <input_file> <output_file>"
          exit 1;
     elif [ "$1" != "-a" ] && [ "$1" != "-A" ] && [ "$1" != "-m" ] && [ "$1" != "-M" ]
     then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" \
          "deformat_desc" "first_line" "$(basename "$0") -[AaMm] <input file> <output_file>"
          exit 1;
     fi
fi

FLEXOPTS=""
FILE1=$1;
FILE2=$2;

if [ $# = 2 ]
then if [ ! -e "$1" ]
     then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APER1000" "file_name" "$1";
          exit 1;
     fi
fi

MODE="apertium" # default mode

if [ $# = 3 ]
then if [ ! -e "$2" ]
     then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APER1000" "file_name" "$2";
          exit 1;
     fi

     if [ "$1" = "-a" ]
     then FLEXOPTS="";
          MODE="apertium";
     elif [ "$1" = "-m" ]
     then FLEXOPTS="";
          MODE="matxin";
     elif [ "$1" = "-M" ]
     then FLEXOPTS="-Cfer";
          MODE="matxin";
     fi

     FILE1=$2;
     FILE2=$3;
fi
