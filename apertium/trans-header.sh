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

