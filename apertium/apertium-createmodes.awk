#!/usr/bin/awk -f

# Parse output from modes2bash.xsl

BEGIN {
  FS="^ *# *"
}

NF==2 && /\.mode$/ {
  filename = $2
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
