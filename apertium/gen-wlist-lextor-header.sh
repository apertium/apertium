
if [ $# != 2 ]
then echo "USAGE: $(basename $0) <input_file> <output_file>" 1>&2
     echo "where <input_file> is a lextor monolingual dictionary (.dix) file" 1>&2
     echo "generated with apertium-gen-lextormono" 1>&2
     exit 1
fi

if [ ! -e $1 ]
then echo "ERROR: '$1' file not found" 1>&2
     exit 1
fi


$LTTOOLBOX_PATH/lt-expand $1 | grep -v "__REGEXP__" |\
awk 'BEGIN{FS=":"}{if(index($2,"__")>0) print $1}' |\
sort | uniq > $2 #|\
#awk '{ #Only lemma and first tag; rest of tags, if present, are ignored
#  if (index($0,">")>0)
#    print substr($0,1,index($0,">"));
#  else
#    print $0;
#}' > $2

exit 0
