
{
  if($0 ~ /----.+----/)
  {
    HEAD = substr($0, 5, length($0)-8);
    split(HEAD, ARR, ":");
    NAME = substr(ARR[1], 5, length(ARR[1]));
  }
  else if(HEAD != 0)
  {
    myfilename = NAME ".mode";
    if(ARR[3] == "yes")
    {
      myfilename = "../" myfilename;
    }
    # fool code because a bug in mawk
    printf $0 "\n"  >> myfilename;
    close(myfilename);
  }
}
