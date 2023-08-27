PATH="${APERTIUM_PATH}:${PATH}"

if [[ $# != 1 ]]; then
    echo "USAGE: $(basename "$0") <input_file>"
    exit 1
fi

FILE1=$1

if [[ ! -e $FILE1 ]]; then
    icuformat "$APERTIUM_DATADIR"/apertium.dat "apertium" "APER1000" "file_name" "$1";
    exit 1
fi

