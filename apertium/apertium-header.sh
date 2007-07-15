PAIR=""
INPUT_FILE=""
OUTPUT_FILE=""

function message
{
  echo "USAGE: $(basename $0) [-d datadir] [-f format] [-u] <translation> [in [out]]"
  echo " -d datadir       directory of linguistic data"
  echo " -f format        one of: txt (default), html, rtf"
  echo " -u               don't display marks '*' for unknown words" 
  echo " translation      typically, LANG1-LANG2, but see modes.xml in language data"
  echo " in               input file (stdin by default)"
  echo " out              output file (stdout by default)"
  exit 1;
}

ARGS=$(getopt "uhf:d:" $*)
set -- $ARGS
for i
do
  case "$i" in
    -f) shift; FORMAT=$1; shift;;
    -d) shift; DIRECTORY=$1; shift;;
    -u) UWORDS="no"; shift;;
    -h) message;;
    --) shift; break;;
  esac
done

case "$#" in 
     3)
       OUTPUT_FILE=$3; 
       INPUT_FILE=$2;
       PAIR=$1;
       if [ ! -e $INPUT_FILE ];
       then echo "Error: file '$INPUT_FILE' not found."
            message;
       fi
       ;;
     2)
       INPUT_FILE=$2;
       PAIR=$1;
       if [ ! -e $INPUT_FILE ];
       then echo "Error: file '$INPUT_FILE' not found."
            message;
       fi
       ;;
     1)
       PAIR=$1
       ;;
     *)
       message 
       ;;
esac    

if [ x$FORMAT = x ]; then FORMAT="txt"; fi
if [ x$DIRECTORY = x ]; then DIRECTORY=$DEFAULT_DIRECTORY; fi

PREFIJO=$PAIR;
FORMATADOR=$FORMAT;
DATOS=$DIRECTORY;
FICHERO=$INPUT_FILE;
SALIDA=$OUTPUT_FILE;

if [ ! -d $DATOS/modes ];
then echo "Error: Directory '$DATOS/modes' does not exist."
     message
fi

if [ ! -e $DATOS/modes/$PREFIJO.mode ];
  then echo -n "Error: Mode $PREFIJO does not exist";
       c=$(find $DATOS/modes|wc -l)
       if((c <= 1));
       then echo ".";
       else echo ". Try one of:";
         for i in $DATOS/modes/*;
         do echo "  " $(basename $i) |awk '{gsub(".mode", ""); print;}'
         done;
       fi
       exit 1;
fi

#Parametro opcional, de no estar, lee de la entrada estandar (stdin)

case "$FORMATADOR" in 
	txt|rtf|html)
		if [[ $UWORDS == "no" ]]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		;;
	txtu)
	        FORMATADOR="txt";
		OPTION="-n"
		;;
	htmlu) 
		FORMATADOR="html";
		OPTION="-n";
		;;
	rtfu)
		FORMATADOR="rtfu";
		OPTION="-n";
		;;
	*) # Por defecto asumimos txt
		FORMATADOR="txt"
		OPTION="-g"
		;;	
esac

if [ -z $REF ]
then 
        REF=$FORMATADOR
fi

$APERTIUM_PATH/apertium-des$FORMATADOR $FICHERO | \
if [ ! -x $DATOS/modes/$PREFIJO.mode ]
then sh $DATOS/modes/$PREFIJO.mode $OPTION
else $DATOS/modes/$PREFIJO.mode $OPTION
fi | \
if [ x$SALIDA = x ]
then $APERTIUM_PATH/apertium-re$FORMATADOR 
else
  $APERTIUM_PATH/apertium-re$FORMATADOR >$SALIDA
fi
