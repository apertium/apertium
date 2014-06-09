#! /bin/sh

# If the user specified a --prefix, take that, otherwise /usr/local/
# is the default.
PREFIX=/usr/local
prefixnext=false
for i in "$@"; do
    case $i in
        --prefix=*)		# equals separated:
	    PREFIX="${i#*=}"
	    ;;
        --prefix)		# space separated:
	    prefixnext=true
	    ;;
        *)
	    $prefixnext && PREFIX="$i" && prefixnext=false
	    ;;
    esac
done

# Set the paths needed by libtool/pkg-config/aclocal etc. By inferring
# them based on --prefix , users don't have to edit ~/.bashrc. We only
# append, so if a user has some other preference, that will override.
PATH="${PATH}:/usr/local/bin"
export PATH
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${PREFIX}/lib"
export LD_LIBRARY_PATH
PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${PREFIX}/share/pkgconfig:${PREFIX}/lib/pkgconfig"
export PKG_CONFIG_PATH
ACLOCAL_PATH="${ACLOCAL_PATH}:${PREFIX}/share/aclocal"
export ACLOCAL_PATH


# Pass on all args to configure
autoreconf -fi && ./configure "$@"
