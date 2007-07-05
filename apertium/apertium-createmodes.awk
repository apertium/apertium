#!/usr/bin/awk -f


{
  if($0 ~ /----.+----/)
  {
    HEAD = substr($0, 5, length($0)-8);
    split(HEAD, ARR, ":");
    NAME = substr(ARR[1], 5, length(ARR[1]));
  }
  else if(HEAD != 0)
  {
    if(ARR[3] == "yes")
    {
      print $0 >"../"NAME".mode";
    } 
    else
    {
      print $0 >NAME".mode";
    }
  }
}
