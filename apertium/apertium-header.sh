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

function translate_odt
{
  INPUT_TMPDIR=/tmp/$$odtdir
  INPUT_TMPFILE=/tmp/$$odtfile

  export LC_CTYPE=$(locale -a|grep -i "utf[.]*8"|head -1);

  if [[ LC_CTYPE == "" ]]
  then echo "Error: Install an UTF-8 locale in your system";
       exit 1;
  fi

  if [[ $(which zip) == "" ]]
  then echo "Error: Install 'zip' command in your system";
       exit 1;
  fi
  
  if [[ $(which unzip) == "" ]]
  then echo "Error: Install 'unzip' command in your system";
       exit 1;
  fi
  
  if [[ $FICHERO == "" ]]
  then FICHERO=/tmp/$$odtorig
       cat > $FICHERO
       BORRAFICHERO="true"
  fi
  OTRASALIDA=/tmp/$$odtsalida.zip
  
  unzip -q -o -d $INPUT_TMPDIR $FICHERO
  find $INPUT_TMPDIR | grep content\\.xml |\
  awk '{printf "<file name=\"" $0 "\"/>" >> MYFILENAME; PART = $0; while(getline < PART) printf(" %s", $0) >> MYFILENAME; printf("\n") >> MYFILENAME;}' MYFILENAME=$INPUT_TMPFILE;
  apertium-desodt $INPUT_TMPFILE |\
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION
  else $DATOS/modes/$PREFIJO.mode $OPTION
  fi | \
  apertium-reodt|\
  awk '{punto = index($0, "<?"); cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("\?> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd $INPUT_TMPDIR
  zip -q -r $OTRASALIDA .
  cd $VUELVE
  rm -Rf $INPUT_TMPDIR $INPUT_TMPFILE

  if [[ $BORRAFICHERO == "true" ]]
  then rm -Rf $FICHERO;
  fi
  if [[ $SALIDA == "" ]]
  then cat $OTRASALIDA;
       rm -Rf $OTRASALIDA
  else mv $OTRASALIDA $SALIDA
  fi
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
        rtf)
		if [[ $UWORDS == "no" ]]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		MILOCALE=$(locale -a|grep -i -v "utf\|^C$\|^POSIX$"|head -1);
		if [[ $MILOCALE == "" ]]
		then echo "Error: Install a ISO-8859-1 compatible locale in your system";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
        
        odt)
		if [[ $UWORDS == "no" ]]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_odt
		exit 0
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
		FORMATADOR="rtf";
		OPTION="-n";
		MILOCALE=$(locale -a|grep -i -v "utf\|^C$\|^POSIX$"|head -1);
		if [[ $MILOCALE == "" ]]
		then echo "Error: Install a ISO-8859-1 compatible locale in your system";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
 
        odtu) 
                OPTION="-n";
                translate_odt
                exit 0
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
