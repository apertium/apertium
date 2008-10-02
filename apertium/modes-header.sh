
if [ $# -lt 1 ]
then echo "USAGE: $(basename $0) <modes file> ";
     exit 1;
fi

FLEXOPTS=""
FILE1=$1;

if [ ! -e $1 ] 
then echo "ERROR: '$1' file not found";
     exit 1;
fi

DIRNAME=$(dirname $1);
FULLDIRNAME=$(cd $DIRNAME; pwd);

rm -Rf *.mode

if [ ! -d $FULLDIRNAME/modes ]
then mkdir $FULLDIRNAME/modes
else rm -Rf $FULLDIRNAME/modes && mkdir $FULLDIRNAME/modes
fi

FILE1=$FULLDIRNAME/$(basename $1)
cd $FULLDIRNAME/modes

if [ $# -eq 2 ]; then
	PREFIX=$2;
	FULLDIRNAME=$APERTIUMDIR"/"$PREFIX;
fi

