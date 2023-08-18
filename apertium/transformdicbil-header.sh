if [ $# != 2 ]
then echo "USAGE: $(basename $0) <input_file> <output_file>";
     exit 1;
fi

FILE1=$1;
FILE2=$2;

if [ ! -e $1 ]
then icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APER1000" "file_name" "$1";
     exit 1;
fi

xsltproc $XSLTPROC_OPTIONS $STYLESHEET $FILE1 >$FILE2
