#include <apertium/pretransfer.h>

#include <lttoolbox/lt_locale.h>
#include <apertium/apertium_config.h>
#include <apertium/unlocked_cstdio.h>
#include <lttoolbox/string_utils.h>

#include <iostream>
#include <string>
#include <i18n.h>

UString storeAndWriteWblank(InputFile& input, UFILE* output)
{
  int mychar;
  UString content = "[["_u;

  while(true)
  {
    mychar = input.get();
    if(input.eof())
    {
      I18n(APER_I18N_DATA, "apertium").error("APER1095", {}, {}, true);
    }

    content += mychar;
    u_fputc(mychar, output);

    if(mychar == '\\')
    {
      mychar = input.get();
      content += mychar;
      u_fputc(mychar, output);
    }
    else if(mychar == ']')
    {
      mychar = input.get();

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

  while((mychar = input.get()) != charcode)
  {
    if(input.eof())
    {
      I18n(APER_I18N_DATA, "apertium").error("APER1095", {}, {}, true);
    }
    u_fputc(mychar, output);
    if(mychar == '\\')
    {
      mychar = input.get();
      u_fputc(mychar, output);
    }
  }
}

void procWord(InputFile& input, UFILE* output, bool surface_forms, bool compound_sep, UString wblank = ""_u)
{
  int mychar;
  UString buffer;

  bool buffer_mode = false;
  bool in_tag = false;
  bool queuing = false;

  if(surface_forms)
  {
    while((mychar = input.get()) != '/') ;
  }

  while((mychar = input.get()) != '$')
  {
    if(input.eof())
    {
      I18n(APER_I18N_DATA, "apertium").error("APER1095", {}, {}, true);
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
        buffer += mychar;
      }
      else if(in_tag == false && mychar == '+')
      {
        buffer.append("$ "_u);
        buffer.append(wblank);
        buffer.append("^"_u);
      }
      else if(in_tag == false && mychar == '~' and compound_sep == true)
      {
        buffer.append("$"_u);
        buffer.append(wblank);
        buffer.append("^"_u);
      }
    }
    else
    {
      if(mychar == '+' && queuing == true)
      {
        buffer.append("$ "_u);
        buffer.append(wblank);
        buffer.append("^"_u);
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
    int mychar = input.get();
    if(input.eof())
    {
      break;
    }
    switch(mychar)
    {
      case '[':
        u_fputc('[', output);
        mychar = input.get();

        if(mychar == '[')
        {
          u_fputc('[', output);
          UString wblank = storeAndWriteWblank(input, output);
          mychar = input.get();

          if(mychar == '^')
          {
            u_fputc(mychar, output);
            procWord(input, output, surface_forms, compound_sep, wblank);
            u_fputc('$', output);
          }
          else
          {
            I18n(APER_I18N_DATA, "apertium").error("APER1096", {}, {}, true);
          }
        }
        else
        {
          input.unget(mychar);
          readAndWriteUntil(input, output, ']');
          u_fputc(']', output);
        }
        break;

      case '\\':
        u_fputc(mychar, output);
        u_fputc(input.get(), output);
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
          u_fflush(output);
        }
        break;

      default:
        u_fputc(mychar, output);
        break;
    }
  }
}
