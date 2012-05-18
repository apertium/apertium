INPUT_FILE=/dev/stdin
OUTPUT_FILE=/dev/stdout

cat $INPUT_FILE | \
gawk '
function is_inline_tag(str,                      aux, val)
{
  for(val in INLINETAGS)
  {
    aux = INLINETAGS[val] "<CONTENTS>";
    if(gsub(aux, aux, str) == 1)
    {
      return 1;
    }
  }
  
  return 0;
}

BEGIN{
  RS="</CONTENTS>";  
  
  INLINETAGS[1]="<textit/>";
  INLINETAGS[2]="<textbf/>";
  INLINETAGS[3]="<emph/>";
}
{
  MYRECORD[++nline] = $0;  
}
END{
  for(i=1; i < nline; i++)
  {
    if(gsub("<CONTENTS>", "<CONTENTS>", MYRECORD[i]) == 1)
    {
      if(is_inline_tag(MYRECORD[i]))
      {
        printf("%s</CONTENTS-noeos>", MYRECORD[i]);
      }
      else
      {
        printf("%s</CONTENTS>", MYRECORD[i]);
      }
    } 
    else
    {
      printf("%s</CONTENTS>", MYRECORD[i]);
    }
  }
  
  printf("%s", MYRECORD[nline]);
}' > $OUTPUT_FILE

