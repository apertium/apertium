if [ $# != 2 ]
then if [ $# != 3 ]
     then echo "USAGE: $(basename $0) -[aAmM] <input_file> <output_file>";
          echo "  -a: apertium standard mode";
          echo "  -A: apertium optimized mode (default mode)";
          echo "  -m: matxin standard mode";
          echo "  -M: matxin optimized mode"; 
          exit 1;
     elif [ $1 != "-a" ] && [ $1 != "-A" ] && [ $1 != "-m" ] && [ $1 != "-M" ]
     then echo "USAGE: $(basename $0) -[AaMm] <input file> <output_file>";
          echo "  -a: apertium standard mode";
          echo "  -A: apertium optimized mode (default mode)";
          echo "  -m: matxin standard mode";
          echo "  -M: matxin optimized mode"; 
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

MODE="apertium" # default mode

if [ $# = 3 ]
then if [ ! -e $2 ]
     then echo "ERROR: '$2' file not found";
          exit 1;
     fi

     if [ $1 = "-a" ]
     then FLEXOPTS="";
          MODE="apertium";
     elif [ $1 = "-m" ]
     then FLEXOPTS="";
          MODE="matxin";
     elif [ $1 = "-M" ]
     then FLEXOPTS="-Cfer";
          MODE="matxin";
     fi

     FILE1=$2;
     FILE2=$3;
fi
