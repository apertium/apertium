if [ $# != 2 ]
then echo "USAGE: $(basename $0) <input_file> <output_file>";
     exit 1;
fi

FILE1=$1;
FILE2=$2;

if [ ! -e $1 ] 
then echo "ERROR: '$1' file not found";
     exit 1;
fi

xsltproc $XSLTPROC_OPTIONS $STYLESHEET $FILE1 >$FILE2
