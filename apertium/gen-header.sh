if [ $# != 2 ]
then if [ $# != 3 ]
     then echo "USAGE: $(basename $0) [-O] <input_file> <output_file>";
          exit 1;
     elif [ $1 != "-O" ]
     then echo "USAGE: $(basename $0) [-O] <input file> <output_file>";
          exit 1;
     fi
fi

FLEXOPTS=""
FILE1=$1;
FILE2=$2;

if [ $# = 2 ]
then if [ ! -e $1 ] 
     then echo "ERROR: '$1' file not found";
          exit 1;
     fi 
fi

if [ $# = 3 ]
then if [ ! -e $2 ]
     then echo "ERROR: '$2' file not found";
          exit 1;
     fi
     FLEXOPTS="-Cfer";
     FILE1=$2;
     FILE2=$3;
fi
