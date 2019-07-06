#include <apertium/pretransfer.h>

void readAndWriteUntil(FILE *input, FILE *output, int const charcode)
{
  int mychar;

  while((mychar = fgetwc_unlocked(input)) != charcode)
  {
    if(feof(input))
    {
      wcerr << L"ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }
    fputwc_unlocked(mychar, output);
    if(mychar == L'\\')
    {
      mychar = fgetwc(input);
      fputwc(mychar, output);
    }
  }
}

void procWord(FILE *input, FILE *output, bool surface_forms, bool compound_sep)
{
  int mychar;
  wstring buffer = L"";

  bool buffer_mode = false;
  bool in_tag = false;
  bool queuing = false;

  if(surface_forms)
  {
    while((mychar = fgetwc_unlocked(input)) != L'/') ;
  }

  while((mychar = fgetwc_unlocked(input)) != L'$')
  {
    if(feof(input))
    {
      wcerr << L"ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }

    switch(mychar)
    {
    case L'<':
      in_tag = true;
      if(!buffer_mode)
      {
        buffer_mode = true;
      }
      break;

    case L'>':
      in_tag = false;
      break;

    case L'#':
      if(buffer_mode)
      {
        buffer_mode = false;
        queuing = true;
      }
      break;
    }

    if(buffer_mode)
    {
      if((mychar != L'+' || (mychar == L'+' && in_tag == true)) &&
         (mychar != L'~' || (mychar == L'~' && in_tag == true)))
      {
        buffer += static_cast<wchar_t>(mychar);
      }
      else if(in_tag == false && mychar == L'+')
      {
        buffer.append(L"$ ^");
      }
      else if(in_tag == false && mychar == L'~' and compound_sep == true)
      {
        buffer.append(L"$^");
      }
    }
    else
    {
      if(mychar == L'+' && queuing == true)
      {
        buffer.append(L"$ ^");
        buffer_mode = true;
      }
      else
      {
        fputwc_unlocked(mychar, output);
      }
    }

  }
  fputws_unlocked(buffer.c_str(), output);
}

void processStream(FILE *input, FILE *output, bool null_flush, bool surface_forms, bool compound_sep)
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
      case L'[':
        fputwc_unlocked(L'[', output);
        readAndWriteUntil(input, output, L']');
        fputwc_unlocked(L']', output);
        break;

      case L'\\':
        fputwc_unlocked(mychar, output);
        fputwc_unlocked(fgetwc_unlocked(input), output);
        break;

      case L'^':
        fputwc_unlocked(mychar, output);
        procWord(input, output, surface_forms, compound_sep);
        fputwc_unlocked(L'$', output);
        break;

      case L'\0':
        fputwc_unlocked(mychar, output);

        if(null_flush)
        {
          fflush(output);
        }
        break;

      default:
        fputwc_unlocked(mychar, output);
        break;
    }
  }
}
