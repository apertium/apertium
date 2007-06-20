case $# in
  2)
    DATOS=$1
    PREFIJO=$2
    FORMATADOR=txt
    ;;
  3)
    DATOS=$1
    PREFIJO=$2
    FORMATADOR=$3
    ;;
  4)
    DATOS=$1
    PREFIJO=$2
    FORMATADOR=$3
    FICHERO=$4
    ;;
  5)  
    DATOS=$1
    PREFIJO=$2
    FORMATADOR=$3
    FICHERO=$4
    SALIDA=$5
    ;;
  *)
    echo "USAGE: $(basename $0) <datadir> <translation> [format [infile [outfile]]]"
    echo " datadir          Directory of linguistic data"
    echo " translation      tipically, LANG1-LANG2, but see modes.xml in language data"
    echo " format           one of: txt (default), txtu, html, htmlu, rtf, rtfu"
    echo " infile           input file (stdin by default)"
    echo " outfile          output file (stdout by default)"
    exit 1;
esac

#Parámetros obligatorios
PREFIJO=$2    #Dirección traducción Ejm.- es-ca
FORMATADOR=$3 #Fuente a traducir Ejm.- txt

DATOS=$1

if [ ! -d $DATOS/modes ];
  then echo "Error: Directory '$DATOS/modes' does not exist."
       exit 1;
fi

if [ ! -e $DATOS/modes/$PREFIJO ];
  then echo -n "Error: Mode $PREFIJO does not exist";
       c=$(find $DATOS/modes|wc -l)
       if((c <= 1));
       then echo ".";
       else echo ". Try one of:";
         for i in $DATOS/modes/*;
         do echo "  " $(basename $i);
         done;
       fi
       exit 1;
fi

#Parametro opcional, de no estar, lee de la entrada estandar (stdin)
FICHERO=$4    #Fichero con el texto a traducir

PATH=.:/usr/local/bin:$PATH
	      
case "$FORMATADOR" in 
	txt)
		FORMATADOR="txt"
		OPTION="-g"		
		;;
	txtu)
		FORMATADOR="txt"
		OPTION="-n"
		;;
	rtf)
		FORMATADOR="rtf"
		OPTION="-g"		
		;;
	rtfu)
		FORMATADOR="rtf"		
		OPTION="-n"
		;;
	html)
		FORMATADOR="html"
		OPTION="-g"	
		;;
	htmlu)
		FORMATADOR="html"
		OPTION="-n"		
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
if [ ! -x $DATOS/modes/$PREFIJO ]
then sh $DATOS/modes/$PREFIJO $OPTION
else $DATOS/modes/$PREFIJO $OPTION
fi | \
if [ x$SALIDA = x ]
then $APERTIUM_PATH/apertium-re$FORMATADOR 
else
  $APERTIUM_PATH/apertium-re$FORMATADOR >$SALIDA
fi
