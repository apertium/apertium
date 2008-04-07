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
    echo " translation      LANG1-LANG2"
    echo " format           one of: txt (default), txtu, html, htmlu, rtf, rtfu"
    echo " infile           input file (stdin by default)"
    echo " outfile          output file (stdout by default)"
    exit 1;
esac

#Parámetros obligatorios
PREFIJO=$2    #Dirección traducción Ejm.- es-ca
FORMATADOR=$3 #Fuente a traducir Ejm.- txt

DATOS=$1

#Parametro opcional, de no estar, lee de la entrada estandar (stdin)
FICHERO=$4    #Fichero con el texto a traducir

PATH=.:/usr/local/bin:$PATH
AUTOMORF=$DATOS/$PREFIJO.automorf.bin
AUTOBIL=$DATOS/$PREFIJO.autobil.bin
#AUTOBIL=$DATOS/$PREFIJO.lextorbil.bin
AUTOGEN=$DATOS/$PREFIJO.autogen.bin
AUTOPGEN=$DATOS/$PREFIJO.autopgen.bin

DEP="dep"

TURL="cat" #No hace nada, se introduce para no tener
           #que cambiar la línea de montaje, pues en algunos
	   #casos se usa como ultimo eslabón de la cadena el
	   #programa turl o ext-turl.
REF=
	      
case "$FORMATADOR" in 
	txt)
		FORMATADOR="txt"
		GENERADOR="lt-proc -g"		
		;;
	txtu)
		FORMATADOR="txt"
		GENERADOR="lt-proc -n"
		;;
	rtf)
		FORMATADOR="rtf"
		GENERADOR="lt-proc -g"		
		;;
	rtfu)
		FORMATADOR="rtf"		
		GENERADOR="lt-proc -n"
		;;
	html)
		FORMATADOR="html"
		GENERADOR="lt-proc -g"	
		;;
	htmlu)
		FORMATADOR="html"
		GENERADOR="lt-proc -n"		
		;;
	*) # Por defecto asumimos txt
		FORMATADOR="txt"
		GENERADOR="lt-proc -g"
		;;	
esac

if [ -z $REF ]
then 
        REF=$FORMATADOR
fi

$APERTIUM_PATH/apertium-des$FORMATADOR $FICHERO | \
$LTTOOLBOX_PATH/lt-proc $AUTOMORF | \
$APERTIUM_PATH/apertium-tagger -g $DATOS/$PREFIJO.prob | \
$APERTIUM_PATH/apertium-pretransfer | \
#$APERTIUM_PATH/apertium-lextor -l $DATOS/$PREFIJO.lextor $DATOS/$PREFIJO.lextormono.bin 3 3 | \
$APERTIUM_PATH/apertium-transfer $DATOS/trules-$PREFIJO.xml $DATOS/trules-$PREFIJO.bin $AUTOBIL | \
$LTTOOLBOX_PATH/$GENERADOR $AUTOGEN  | \
$LTTOOLBOX_PATH/lt-proc -p $AUTOPGEN | \
if [ x$SALIDA = x ]
then $APERTIUM_PATH/apertium-re$FORMATADOR 
else
  $APERTIUM_PATH/apertium-re$FORMATADOR >$SALIDA
fi
