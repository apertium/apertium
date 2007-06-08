
if [ $# != 3 ]
then echo "USAGE: $(basename $0) <n> <input_file> <output_file>" 1>&2
     echo "where <n> is the desired number of stopwords" 1>&2
     echo "      <input_file> contains a large preprocessed corpus" 1>&2
     echo "      <output_file> is the file to which the list of stopwords is written" 1>&2
     exit 1
fi

N=$1
INFILE=$2
OUTFILE=$3

if [ ! -e $INFILE  ]
then echo "ERROR: '$INFILE' file not found" 1>&2
     exit 1
fi

cat $INFILE |\
sed -re "s/(\^[0-9·ÀÁÂÄÇÈÉÊËÌÍÎÏÑÒÓÔÖÙÚÛÜàáâäçèéêëìíîïñòóôöùúûüABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz <>ç+.,;:_'#*%()?¿!¡-]+\\$)/\1\n/g" |\
sed -re "s/^[ \t]+//g" |\
sed -re "s/[ \t]+$//g" |\
sed -re "s/^\^//g" |\
sed -re "s/\\\$$//g" |\
awk '{if (length($0)>0) print tolower($0)}' |\
awk '{ #Only lemma and first tag; rest of tags, if present, are ignored
  if (index($0,">")>0)
    print substr($0,1,index($0,">"));
  else
    print $0;
}' |\
sort | uniq -c | sort -n -r |\
head -n $N |\
awk 'BEGIN{FS=" "}
{
  c="";
  for(i=2; i<=NF; i++) {
    if (length(c)>0)
      c= c " "
    c = c $i  
  }
  print c;
}' > $OUTFILE

exit 0
