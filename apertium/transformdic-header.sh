if [ $# != 3 ]
then echo "USAGE: $(basename $0) lr|rl <input_file> <output_file>";
     exit 1;
fi

FILE1=$2;
FILE2=$3;

if [ ! -e $2 ] 
then echo "ERROR: '$1' file not found";
     exit 1;
fi

if [ $1 = "lr" ]
then xsltproc $XSLTPROC_OPTIONS_LR $STYLESHEET $FILE1 >$FILE2
elif [ $1 = "rl" ]
then xsltproc $XSLTPROC_OPTIONS_RL $STYLESHEET $FILE1 >$FILE2
else 
  echo "ERROR: $1 option invalid";
fi
