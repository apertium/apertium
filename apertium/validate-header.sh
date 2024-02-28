PATH="${APERTIUM_PATH}:${PATH}"

if [[ $# != 1 ]]; then
    echo "USAGE: $(basename "$0") <input_file>"
    exit 1
fi

FILE1=$1

if [[ ! -e $FILE1 ]]; then
    formatmsg "$APERTIUM_DATADIR"/apertium.dat "apertium" "APR80000" "file_name" "$1";
    exit 1
fi

