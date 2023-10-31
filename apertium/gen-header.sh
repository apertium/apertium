PATH="${APERTIUM_PATH}:${PATH}"

if [ $# != 2 ]
then if [ $# != 3 ]
     then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" \
          "gen_desc" "first_line" "$(basename "$0") [-O] <input_file> <output_file>";
          exit 1;
     elif [ "$1" != "-O" ]
     then formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" \
          "gen_desc" "first_line" "$(basename "$0") [-O] <input file> <output_file>";
          exit 1;
     fi
fi

FLEXOPTS=""
FILE1=$1;
FILE2=$2;

if [ $# = 2 ]
then if [ ! -e "$1" ]
     then echo formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$1";
          exit 1;
     fi
fi

if [ $# = 3 ]
then if [ ! -e "$2" ]
     then echo formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$2";
          exit 1;
     fi
     FLEXOPTS="-Cfer";
     FILE1=$2;
     FILE2=$3;
fi
