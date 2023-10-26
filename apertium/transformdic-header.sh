if [ $# != 3 ]
then echo "USAGE: $(basename $0) lr|rl <input_file> <output_file>";
     exit 1;
fi

FILE1=$2;
FILE2=$3;

if [ ! -e $2 ]
     then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$2";
     exit 1;
fi

if [ $1 = "lr" ]
then xsltproc $XSLTPROC_OPTIONS_LR $STYLESHEET $FILE1 >$FILE2
elif [ $1 = "rl" ]
then xsltproc $XSLTPROC_OPTIONS_RL $STYLESHEET $FILE1 >$FILE2
else
  then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR81080" "opt" "$1";
fi
