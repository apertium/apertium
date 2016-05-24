if [[ $# != 1 ]]; then
    echo "USAGE: $(basename "$0") <input_file>"
    exit 1
fi

FILE1=$1

if [[ ! -e $FILE1 ]]; then
    echo "ERROR: '$1' file not found"
    exit 1
fi

