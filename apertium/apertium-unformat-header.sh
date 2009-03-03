PAIR=""
INPUT_FILE="/dev/stdin"
OUTPUT_FILE="/dev/stdout"

message ()
{
  echo "USAGE: $(basename $0) [-f format] [in [out]]"
  echo " -f format        one of: txt (default), html, rtf, odt, docx, wxml, xlsx, pptx"
  echo " in               input file (stdin by default)"
  echo " out              output file (stdout by default)"
  exit 1;
}

locale_utf8 ()
{
  export LC_CTYPE=$(locale -a|grep -i "utf[.]*8"|head -1);
  if [[ LC_CTYPE = "" ]]
  then echo "Error: Install an UTF-8 locale in your system";
       exit 1;
  fi
}

test_zip ()
{
 if [[ $(which zip) = "" ]]
  then echo "Error: Install 'zip' command in your system";
       exit 1;
  fi
  
  if [[ $(which unzip) = "" ]]
  then echo "Error: Install 'unzip' command in your system";
       exit 1;
  fi 
}

unformat_odt ()
{
  INPUT_TMPDIR=$(mktemp -d /tmp/apertium.XXXXXXXX)

  locale_utf8
  test_zip
  
  unzip -q -o -d $INPUT_TMPDIR $FICHERO
  find $INPUT_TMPDIR | grep content\\\.xml |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-desodt >$SALIDA
  rm -Rf $INPUT_TMPDIR
}

unformat_docx ()
{
  INPUT_TMPDIR=$(mktemp -d /tmp/apertium.XXXXXXXX)

  locale_utf8
  test_zip
  
  unzip -q -o -d $INPUT_TMPDIR $FICHERO
  
  for i in $(find $INPUT_TMPDIR|grep "xlsx$");
  do LOCALTEMP=$(mktemp /tmp/apertium.XXXXXXXX)
     $APERTIUM_PATH/apertium -f xlsx -d $DIRECTORY $OPCIONU $PREFIJO <$i >$LOCALTEMP;
     cp $LOCALTEMP $i;
     rm $LOCALTEMP;
  done;
  
  find $INPUT_TMPDIR | grep "xml" |\
  grep -v -i \\\(settings\\\|theme\\\|styles\\\|font\\\|rels\\\) |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-deswxml >$SALIDA
  rm -Rf $INPUT_TMPDIR
}

unformat_pptx ()
{
  INPUT_TMPDIR=$(mktemp -d /tmp/apertium.XXXXXXXX)

  locale_utf8
  test_zip
    
  unzip -q -o -d $INPUT_TMPDIR $FICHERO
  
  for i in $(find $INPUT_TMPDIR|grep "xlsx$");
  do LOCALTEMP=$(mktemp /tmp/apertium.XXXXXXXX)
     $APERTIUM_PATH/apertium -f xlsx -d $DIRECTORY $OPCIONU $PREFIJO <$i >$LOCALTEMP
     cp $LOCALTEMP $i
     rm $LOCALTEMP
  done;
  
  find $INPUT_TMPDIR | grep "xml$" |\
  grep "slides\/slide" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-despptx >$SALIDA
  rm -Rf $INPUT_TMPDIR
}


unformat_xlsx ()
{
  INPUT_TMPDIR=$(mkdir -d)

  locale_utf8
  test_zip
  
  unzip -q -o -d $INPUT_TMPDIR $FICHERO
  find $INPUT_TMPDIR | grep "sharedStrings.xml" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-desxlsx >$SALIDA
  rm -Rf $INPUT_TMPDIR

}


ARGS=$(getopt "f:" $*)
set -- $ARGS
for i
do
  case "$i" in 
    -f) shift; FORMAT=$1; shift;;
    --) shift; break;;
  esac
done

case "$#" in 
     2)
       OUTPUT_FILE=$2; 
       INPUT_FILE=$1;
       if [ ! -e $INPUT_FILE ];
       then echo "Error: file '$INPUT_FILE' not found."
            message;
       fi
       ;;
     1)
       INPUT_FILE=$1;
       if [ ! -e $INPUT_FILE ];
       then echo "Error: file '$INPUT_FILE' not found."
            message;
       fi
       ;;
     0)
       ;;
     *)
       message 
       ;;
esac    

if [ x$FORMAT = x ]; then FORMAT="txt"; fi

FORMATADOR=$FORMAT;
FICHERO=$INPUT_FILE;
SALIDA=$OUTPUT_FILE;


case "$FORMATADOR" in 
        rtf)
		MILOCALE=$(locale -a|grep -i -v "utf\|^C$\|^POSIX$"|head -1);
		if [[ $MILOCALE = "" ]]
		then echo "Error: Install a ISO-8859-1 compatible locale in your system";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
        
        odt)
		unformat_odt
		exit 0
		;;
	docx)
		unformat_docx
		exit 0
		;;
	xlsx)
		unformat_xlsx
		exit 0
		;;
	pptx)
		unformat_pptx
		exit 0
		;;
		
	wxml)
	        locale_utf8
	        ;;
	*)
	        ;;
	        	
esac

$APERTIUM_PATH/apertium-des$FORMATADOR $FICHERO >$SALIDA
