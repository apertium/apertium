
if [ $# != 4 ]
then echo "USAGE: $(basename $0) <dada_dir> <translation_dir> <input_file> <output_file>" 1>&2
     echo "where <data_dir> is the path to the linguistic data to use" 1>&2 
     echo "      <translation_dir> is the translation direction to use" 1>&2
     echo "      <input_file> contains a large corpus in raw format" 1>&2
     echo "      <output_file> is the file to which the preprocessed corpus is written" 1>&2
     exit 1
fi

DATA_DIR=$1
TRANSLATION_DIR=$2
INFILE=$3
OUTFILE=$4

if [ ! -e $INFILE  ]
then echo "ERROR: '$INFILE' file not found" 1>&2
     exit 1
fi

if [ ! -e $DATA_DIR/$TRANSLATION_DIR.automorf.bin  ]
then echo "ERROR: '$DATA_DIR/$TRANSLATION_DIR.automorf.bin' file not found" 1>&2
     exit 1
fi

if [ ! -e $DATA_DIR/$TRANSLATION_DIR.prob  ]
then echo "ERROR: '$DATA_DIR/$TRANSLATION_DIR.prob' file not found" 1>&2
     exit 1
fi


cat $INFILE | $APERTIUM_PATH/apertium-destxt |\
$LTTOOLBOX_PATH/lt-proc -a $DATA_DIR/$TRANSLATION_DIR.automorf.bin |\
$APERTIUM_PATH/apertium-tagger -g $DATA_DIR/$TRANSLATION_DIR.prob |\
$APERTIUM_PATH/apertium-pretransfer |\
$APERTIUM_PATH/apertium-retxt |\
awk 'BEGIN{FS="\\$"} #Discards characters not belonging to apertium words
{
  c="";
  for (j=1; j<=NF; j++) {
    w=$j;
    w=substr(w,index(w,"^"));
            
    if ((length(w)>0) && (index(w,"^")>0)) {                    
      if (length(c)>0)
        c = c " ";                                  
      c = c w "$";
    }
  }
  
  print c;
}' >  $OUTFILE

exit 0
