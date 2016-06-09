#!/usr/bin/awk -f

# Parse output from modes2bash.xsl

BEGIN {
  FS="^ *# *"
  guesswarned=0
}

NF==2 && /\.mode$/ {
  filename = $2
  if(filename ~ /NAMEME/ && !guesswarned) {
    print "apertium-createmodes.awk: At least one program in a gendebug=\"yes\" mode needs a debug-suff attribute; couldn't guess what suffix to use for the debug mode." > "/dev/stderr"
    guesswarned=1
  }
  if(seen[filename]) {
    print "apertium-createmodes.awk: "filename" seen twice" > "/dev/stderr"
    filename = 0
  }
  else {
    print "" > filename
    seen[filename] = 1
  }
  next
}

filename {
  print $0 >> filename
  close(filename)
}
