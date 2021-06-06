#include <apertium/pretransfer.h>

#include <lttoolbox/lt_locale.h>
#include <apertium/apertium_config.h>
#include <apertium/unlocked_cstdio.h>
#include <apertium/string_utils.h>

#include <iostream>
#include <string>

UString storeAndWriteWblank(InputFile& input, UFILE* output)
{
  int mychar;
  UString content = "[[";

  while(true)
  {
    mychar = fgetwc_unlocked(input);
    if(feof(input))
    {
      cerr << "ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }
    
    content += mychar;
    u_fputc(mychar, output);
    
    if(mychar == '\\')
    {
      mychar = fgetwc(input);
      content += mychar;
      u_fputc(mychar, output);
    }
    else if(mychar == ']')
    {
      mychar = fgetwc(input);
      
      if(mychar == ']')
      {
        content += mychar;
        u_fputc(mychar, output);
        break;
      }
    }
  }
  
  return content;
}

void readAndWriteUntil(InputFile& input, UFILE* output, int const charcode)
{
  int mychar;

  while((mychar = fgetwc_unlocked(input)) != charcode)
  {
    if(feof(input))
    {
      cerr << "ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }
    u_fputc(mychar, output);
    if(mychar == '\\')
    {
      mychar = fgetwc(input);
      u_fputc(mychar, output);
    }
  }
}

void procWord(InputFile& input, UFILE* output, bool surface_forms, bool compound_sep, UString wblank = "")
{
  int mychar;
  UString buffer = "";

  bool buffer_mode = false;
  bool in_tag = false;
  bool queuing = false;

  if(surface_forms)
  {
    while((mychar = fgetwc_unlocked(input)) != '/') ;
  }

  while((mychar = fgetwc_unlocked(input)) != '$')
  {
    if(feof(input))
    {
      cerr << "ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }

    switch(mychar)
    {
    case '<':
      in_tag = true;
      if(!buffer_mode)
      {
        buffer_mode = true;
      }
      break;

    case '>':
      in_tag = false;
      break;

    case '#':
      if(buffer_mode)
      {
        buffer_mode = false;
        queuing = true;
      }
      break;
    }

    if(buffer_mode)
    {
      if((mychar != '+' || (mychar == '+' && in_tag == true)) &&
         (mychar != '~' || (mychar == '~' && in_tag == true)))
      {
        buffer += static_cast<wchar_t>(mychar);
      }
      else if(in_tag == false && mychar == '+')
      {
        buffer.append("$ ");
        buffer.append(wblank);
        buffer.append("^");
      }
      else if(in_tag == false && mychar == '~' and compound_sep == true)
      {
        buffer.append("$");
        buffer.append(wblank);
        buffer.append("^");
      }
    }
    else
    {
      if(mychar == '+' && queuing == true)
      {
        buffer.append("$ ");
        buffer.append(wblank);
        buffer.append("^");
        buffer_mode = true;
      }
      else
      {
        u_fputc(mychar, output);
      }
    }

  }
  write(buffer, output);
}

void processStream(InputFile& input, UFILE* output, bool null_flush, bool surface_forms, bool compound_sep)
{
  while(true)
  {
    int mychar = fgetwc_unlocked(input);
    if(feof(input))
    {
      break;
    }
    switch(mychar)
    {
      case '[':
        u_fputc('[', output);
        mychar = fgetwc_unlocked(input);
        
        if(mychar == '[')
        {
          u_fputc('[', output);
          UString wblank = storeAndWriteWblank(input, output);
          mychar = fgetwc_unlocked(input);
          
          if(mychar == '^')
          {
            u_fputc(mychar, output);
            procWord(input, output, surface_forms, compound_sep, wblank);
            u_fputc('$', output);
          }
          else
          {
            cerr << "ERROR: Wordbound blank isn't immediately followed by the Lexical Unit." << endl;
            exit(EXIT_FAILURE);
          }
        }
        else
        {
          ungetwc(mychar, input);
          readAndWriteUntil(input, output, ']');
          u_fputc(']', output);
        }
        break;

      case '\\':
        u_fputc(mychar, output);
        u_fputc(fgetwc_unlocked(input), output);
        break;

      case '^':
        u_fputc(mychar, output);
        procWord(input, output, surface_forms, compound_sep);
        u_fputc('$', output);
        break;

      case '\0':
        u_fputc(mychar, output);

        if(null_flush)
        {
          fflush(output);
        }
        break;

      default:
        u_fputc(mychar, output);
        break;
    }
  }
}
