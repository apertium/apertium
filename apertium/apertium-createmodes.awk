#!/usr/bin/awk -f


{
  if($0 ~ /----.+----/)
  {
    OUTPUTFILE = substr($0, 5, length($0)-8);
  }
  else if(OUTPUTFILE != 0)
  {
    print $0 >OUTPUTFILE;
    system("chmod a+x " PARAM "/modes/" OUTPUTFILE);
  }
}