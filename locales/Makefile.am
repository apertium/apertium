apertium.dat: root.txt en.txt
	genrb -d . root.txt en.txt
	echo root.res en.res > package_list.txt
	pkgdata -p apertium --mode archive -d . package_list.txt

localesdir = $(LOCALES_DIR)
dist_locales_DATA = apertium.dat
