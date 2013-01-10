# Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

PATH="${APERTIUM_PATH}:${PATH}"
PAIR=""
INPUT_FILE="/dev/stdin"
OUTPUT_FILE="/dev/stdout"

[ -z "$TMPDIR" ] && TMPDIR=/tmp

message ()
{
  echo "USAGE: $(basename $0) [-d datadir] [-f format] [-u] <direction> [in [out]]"
  echo " -d datadir       directory of linguistic data"
  echo " -f format        one of: txt (default), html, rtf, odt, docx, wxml, xlsx, pptx,"
  echo "                  xpresstag, html-noent, latex, latex-raw";
  echo " -a               display ambiguity"
  echo " -u               don't display marks '*' for unknown words" 
  echo " -m memory.tmx    use a translation memory to recycle translations"
  echo " -o direction     translation direction using the translation memory,"
  echo "                  by default 'direction' is used instead"
  echo " -l               lists the available translation directions and exits"
  echo " direction        typically, LANG1-LANG2, but see modes.xml in language data"
  echo " in               input file (stdin by default)"
  echo " out              output file (stdout by default)"
  exit 1;
}

list_directions ()
{
         for i in $DATOS/modes/*.mode;
         do echo "  " $(basename $i) |awk '{gsub(".mode", ""); print;}'
         done;
}

locale_utf8 ()
{
  export LC_CTYPE=$(locale -a|grep -i "utf[.]*8"|head -1);
  if [ LC_CTYPE = "" ]
  then echo "Error: Install an UTF-8 locale in your system";
       exit 1;
  fi
}

locale_latin1 ()
{
  export LC_CTYPE=$(locale -a|grep -i -e "8859-1" -e "@euro"|head -1);
  if [ LC_CTYPE = "" ]
  then echo "Error: Install a Latin-1 locale in your system";
       exit 1;
  fi
}

test_zip ()
{
 if [ "$(which zip)" = "" ]
  then echo "Error: Install 'zip' command in your system";
       exit 1;
  fi
  
  if [ "$(which unzip)" = "" ]
  then echo "Error: Install 'unzip' command in your system";
       exit 1;
  fi 
}

test_gawk ()
{
  GAWK=$(which gawk)
  if [ "$GAWK" = "" ]
  then echo "Error: Install 'gawk' in your system"
       exit 1
  fi
}


translate_latex()
{
  test_gawk

  if [ "$FICHERO" = ""  -o "$FICHERO" = /dev/stdin ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi

  if [ "$(file -b --mime-encoding $FICHERO)" == "utf-8" ]
  then locale_latin1
  else locale_utf8
  fi
  
  $APERTIUM_PATH/apertium-prelatex "$FICHERO" | \
  $APERTIUM_PATH/apertium-utils-fixlatex | \
  $APERTIUM_PATH/apertium-deslatex | \
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-relatex| \
  awk '{gsub("</CONTENTS-noeos>", "</CONTENTS>"); print;}' | \
  $APERTIUM_PATH/apertium-postlatex >"$SALIDA"
  
  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO"
  fi
}


translate_latex_raw()
{
  test_gawk
  
  if [ "$FICHERO" = "" -o "$FICHERO" = /dev/stdin ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi

  if [ "$(file -b --mime-encoding $FICHERO)" = "utf-8" ]
  then locale_latin1
  else locale_utf8
  fi

  $APERTIUM_PATH/apertium-prelatex "$FICHERO" | \
  $APERTIUM_PATH/apertium-utils-fixlatex | \
  $APERTIUM_PATH/apertium-deslatex | \
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;  
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-relatex| \
  awk '{gsub("</CONTENTS-noeos>", "</CONTENTS>"); print;}' | \
  $APERTIUM_PATH/apertium-postlatex-raw >"$SALIDA"  
}


translate_odt ()
{
  INPUT_TMPDIR=$(mktemp -d $TMPDIR/apertium.XXXXXXXX)

  locale_utf8
  test_zip

  if [ "$FICHERO" = "" ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
  
  unzip -q -o -d $INPUT_TMPDIR "$FICHERO"
  find $INPUT_TMPDIR | grep "content\\.xml\\|styles\\.xml" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-desodt |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ];
  then cat;
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-reodt|\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd $INPUT_TMPDIR
  rm -Rf ObjectReplacements
  zip -q -r - . >$OTRASALIDA
  cd $VUELVE
  rm -Rf $INPUT_TMPDIR

  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO";
  fi
  
  cat $OTRASALIDA >"$SALIDA"
  rm -Rf $OTRASALIDA
  rm -Rf $TMCOMPFILE
}

translate_docx ()
{
  INPUT_TMPDIR=$(mktemp -d $TMPDIR/apertium.XXXXXXXX)

  locale_utf8
  test_zip
  
  if [ "$FICHERO" = "" ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
  
  if [ "$UWORDS" = "no" ]
  then OPCIONU="-u";
  else OPCIONU="";
  fi
  
  unzip -q -o -d $INPUT_TMPDIR "$FICHERO"
  
  for i in $(find $INPUT_TMPDIR|grep "xlsx$");
  do LOCALTEMP=$(mktemp "$TMPDIR/apertium.XXXXXXXX");
     $APERTIUM_PATH/apertium -f xlsx -d $DIRECTORY $OPCIONU $PREFIJO <$i >$LOCALTEMP;
     cp $LOCALTEMP $i;
     rm $LOCALTEMP;
  done;
  
  find $INPUT_TMPDIR | grep "xml" |\
  grep -v -i \\\(settings\\\|theme\\\|styles\\\|font\\\|rels\\\|docProps\\\) |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-deswxml |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;  
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-rewxml|\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd $INPUT_TMPDIR
  zip -q -r - . >$OTRASALIDA
  cd $VUELVE
  rm -Rf $INPUT_TMPDIR

  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO";
  fi
  
  cat $OTRASALIDA >"$SALIDA"
  rm -Rf $OTRASALIDA
  rm -Rf $TMCOMPFILE
}

translate_pptx ()
{
  INPUT_TMPDIR=$(mktemp -d $TMPDIR/apertium.XXXXXXXX)

  locale_utf8
  test_zip
  
  if [ "$FICHERO" = "" ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
  
  if [ "$UWORDS" = "no" ]
  then OPCIONU="-u";
  else OPCIONU="";
  fi
  
  unzip -q -o -d $INPUT_TMPDIR "$FICHERO"
  
  for i in $(find $INPUT_TMPDIR|grep "xlsx$");
  do LOCALTEMP=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
     $APERTIUM_PATH/apertium -f xlsx -d $DIRECTORY $OPCIONU $PREFIJO <$i >$LOCALTEMP;
     cp $LOCALTEMP $i
     rm $LOCALTEMP
  done;
  
  find $INPUT_TMPDIR | grep "xml$" |\
  grep "slides\/slide" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-despptx |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;  
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-repptx |\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd $INPUT_TMPDIR
  zip -q -r - . >$OTRASALIDA
  cd $VUELVE
  rm -Rf $INPUT_TMPDIR

  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO";
  fi
  
  cat $OTRASALIDA >"$SALIDA"
  rm -Rf $OTRASALIDA
  rm -Rf $TMCOMPFILE
}


translate_xlsx ()
{
  INPUT_TMPDIR=$(mktemp -d $TMPDIR/apertium.XXXXXXXX)

  locale_utf8
  test_zip
  
  if [ "$FICHERO" = "" ]
  then FICHERO=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
       cat > "$FICHERO"
       BORRAFICHERO="true"
  fi
  OTRASALIDA=$(mktemp "$TMPDIR/apertium.XXXXXXXX")
  
  unzip -q -o -d $INPUT_TMPDIR "$FICHERO"
  find $INPUT_TMPDIR | grep "sharedStrings.xml" |\
  awk '{printf "<file name=\"" $0 "\"/>"; PART = $0; while(getline < PART) printf(" %s", $0); printf("\n");}' |\
  $APERTIUM_PATH/apertium-desxlsx |\
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;  
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | \
  if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | \
  $APERTIUM_PATH/apertium-rexlsx |\
  awk '{punto = index($0, "/>") + 3; cabeza = substr($0, 1, punto-1); cola = substr($0, punto); n1 = substr(cabeza, index(cabeza, "\"")+1); name = substr(n1, 1, index(n1, "\"")-1); gsub("[?]> ", "?>\n", cola); print cola > name;}'
  VUELVE=$(pwd)
  cd $INPUT_TMPDIR
  zip -q -r - . >$OTRASALIDA
  cd $VUELVE
  rm -Rf $INPUT_TMPDIR

  if [ "$BORRAFICHERO" = "true" ]
  then rm -Rf "$FICHERO";
  fi

  cat $OTRASALIDA >"$SALIDA"
  rm -Rf $OTRASALIDA
  rm -Rf $TMCOMPFILE
}

translate_htmlnoent ()
{
  $APERTIUM_PATH/apertium-deshtml "$FICHERO" | \
  if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
  then cat;  
  else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
  fi | if [ ! -x $DATOS/modes/$PREFIJO.mode ]
  then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
  fi | if [ "$FORMATADOR" = "none" ]
  then cat >"$SALIDA";
  else $APERTIUM_PATH/apertium-rehtml-noent >"$SALIDA";
  fi

  rm -Rf $TMCOMPFILE
}




ARGS=$(getopt "uahlf:d:m:o:" $*)
set -- $ARGS
for i
do
  case "$i" in 
    -f) shift; FORMAT=$1; shift;;
    -d) shift; DIRECTORY=$1; shift;;
    -m) shift; TRANSLATION_MEMORY_FILE=$1; shift;;
    -o) shift; TRANSLATION_MEMORY_DIRECTION=$1 shift;;
    -u) UWORDS="no"; shift;;
    -a) OPTION_TAGGER="-m"; shift;;
    -l) DATOS=$DEFAULT_DIRECTORY; list_directions; exit 0;;
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
if [ x$TRANSLATION_MEMORY_DIRECTION = x ]; then TRANSLATION_MEMORY_DIRECTION=$PAIR; fi

PREFIJO=$PAIR;
FORMATADOR=$FORMAT;
DATOS=$DIRECTORY;
FICHERO=$INPUT_FILE;
SALIDA=$OUTPUT_FILE;
TMCOMPFILE=$(mktemp "$TMPDIR/apertium.XXXXXXXX")

if [ "$TRANSLATION_MEMORY_FILE" != "" ]
then $APERTIUM_PATH/lt-tmxcomp $TRANSLATION_MEMORY_DIRECTION $TRANSLATION_MEMORY_FILE $TMCOMPFILE >/dev/null
     if [ "$?" != "0" ]
     then echo "Error: Cannot compile TM '" $TRANSLATION_MEMORY_FILE "'";
          echo"   hint: use -o parameter";
          message;
     fi
fi

if [ ! -d $DATOS/modes ];
then echo "Error: Directory '$DATOS/modes' does not exist."
     message
fi

if [ ! -e $DATOS/modes/$PREFIJO.mode ];
  then echo -n "Error: Mode $PREFIJO does not exist";
       c=$(find $DATOS/modes|wc -l)
       if [ "$c" -le 1 ];
       then echo ".";
       else echo ". Try one of:";
         list_directions;
#         for i in $DATOS/modes/*.mode;
#         do echo "  " $(basename $i) |awk '{gsub(".mode", ""); print;}'
#         done;
       fi
       exit 1;
fi

#Parametro opcional, de no estar, lee de la entrada estandar (stdin)

case "$FORMATADOR" in 
        none)
        	if [ "$UWORDS" = "no" ]
        	then OPTION="-n";
        	else OPTION="-g";
        	fi
	        ;;
	txt|rtf|html|xpresstag)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		;;
        rtf)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		MILOCALE=$(locale -a|grep -i -v "utf\|^C$\|^POSIX$"|head -1);
		if [ "$MILOCALE" = "" ]
		then echo "Error: Install a ISO-8859-1 compatible locale in your system";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
        
        odt)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_odt
		exit 0
		;;
        latex)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_latex
		exit 0
		;;
        latex-raw)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_latex_raw
		exit 0
		;;
		
		
	docx)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_docx
		exit 0
		;;
	xlsx)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_xlsx
		exit 0
		;;
	pptx)
		if [ "$UWORDS" = "no" ]; then OPTION="-n"; 
		else OPTION="-g";
		fi;
		translate_pptx
		exit 0
		;;
	html-noent)
		if [ "$UWORDS" = "no" ]; then OPTION="-n";
		else OPTION="-g";
		fi;
		translate_htmlnoent
		exit 0
		;;
		
	wxml)
		if [ "$UWORDS" = "no" ]; then OPTION="-n";
		else OPTION="-g";
		fi;
	        locale_utf8
	        ;;
	        	
	txtu)
	        FORMATADOR="txt";
		OPTION="-n"
		;;
	htmlu) 
		FORMATADOR="html";
		OPTION="-n";
		;;
	xpresstagu)
		FORMATADOR="xpresstag";
		OPTION="-n";
		;;
	rtfu)
		FORMATADOR="rtf";
		OPTION="-n";
		MILOCALE=$(locale -a|grep -i -v "utf\|^C$\|^POSIX$"|head -1);
		if [ "$MILOCALE" = "" ]
		then echo "Error: Install a ISO-8859-1 compatible locale in your system";
	             exit 1;
	        fi
	        export LC_CTYPE=$MILOCALE
		;;
 
        odtu) 
                OPTION="-n"
                translate_odt
                exit 0
                ;;
                
        docxu)
                OPTION="-n"
                translate_docx
                exit 0
                ;;

        xlsxu)
                OPTION="-n"
                translate_xlsx
                exit 0
                ;;

        pptxu)
                OPTION="-n"
                translate_pptx
                exit 0
                ;;
        
        wxmlu)
		OPTION="-n";
	        locale_utf8
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

if [ "$FORMATADOR" = "none" ]
then cat "$FICHERO";
else $APERTIUM_PATH/apertium-des$FORMATADOR "$FICHERO"; fi| \
if [ "$TRANSLATION_MEMORY_FILE" = "" ]; 
then cat;  
else $APERTIUM_PATH/lt-tmxproc $TMCOMPFILE;
fi | if [ ! -x $DATOS/modes/$PREFIJO.mode ]
then sh $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
else $DATOS/modes/$PREFIJO.mode $OPTION $OPTION_TAGGER
fi | if [ "$FORMATADOR" = "none" ]
then cat >"$SALIDA";
else $APERTIUM_PATH/apertium-re$FORMATADOR >"$SALIDA";
fi

rm -Rf $TMCOMPFILE
