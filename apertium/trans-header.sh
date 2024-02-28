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
    formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "trans_desc" "basename" "$(basename "$0")"
    exit 1;
esac

