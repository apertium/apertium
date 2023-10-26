/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*  From hunalign; for license see ../AUTHORS and ../COPYING.hunalign     *
*                                                                        *
*************************************************************************/
#include <apertium/tmx_arguments_parser.h>
#include <lttoolbox/string_utils.h>
#include <iostream>
#include <stdlib.h>
#include <lttoolbox/i18n.h>

// Could be better.
bool alphabetic( char c)
{
  return ((c>='a')&&(c<='z')) || ((c>='A')&&(c<='Z')) || (c=='_');
}

bool Arguments::read( int argc, char **argv )
{
  for ( int i=1; i<argc; ++i )
  {
    std::string p = argv[i];
    if (p.empty() || p[0]!='-')
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81260", {"arg"}, {p.c_str()}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
      return false;
    }
    p.erase(0,1);

    if (p.empty())
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81270", {}, {}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
      return false;
    }

    size_t j;

    for (j = 0 ; j<p.size(); ++j )
    {
      if (! alphabetic(p[j]) )
      {
        if (p[j]=='=')
          p.erase(j,1);
        break;
      }
    }

    ArgName name = p.substr(0,j);
    std::string val = p.substr(j, p.size()-j);
    int num = atoi(val.c_str());

    AnyData anyData(val);
    if ( (num!=0) || (val=="0") )
    {
      anyData.dInt = num;
      anyData.kind = AnyData::Int;
    }
    operator[](name) = anyData;

  }

  return true;
}

bool Arguments::read( int argc, char **argv, std::vector<const char*>& remains )
{
  remains.clear();

  for ( int i=1; i<argc; ++i )
  {
    std::string p = argv[i];
    if (p.empty() || p[0]!='-')
    {
      remains.push_back(argv[i]);
      continue;
    }

    p.erase(0,1);

    if (p.empty())
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81270", {}, {}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
      return false;
    }

    size_t j;
    for (j = 0; j<p.size(); ++j )
    {
      if (! alphabetic(p[j]) )
      {
        if (p[j]=='=')
          p.erase(j,1);
        break;
      }
    }

    ArgName name = p.substr(0,j);
    std::string val = p.substr(j, p.size()-j);
    int num = atoi(val.c_str());

    AnyData anyData(val);
    if ( (num!=0) || (val=="0") )
    {
      anyData.dInt = num;
      anyData.kind = AnyData::Int;
    }
    operator[](name) = anyData;

  }

  return true;
}

bool Arguments::getNumericParam( const std::string& name, int& num )
{
  const_iterator it=find(name);
  if (it==end())
  {
    // std::cerr << "Argument -" << name << " missing.\n";
    return false;
  }

  if (it->second.kind != AnyData::Int)
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81280", {"arg"}, {name.c_str()}, false);
    throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
  }

  num = it->second.dInt;
  erase(name);
  return true;
}

bool Arguments::getSwitchConst( const ArgName& name, bool& sw ) const
{
  const_iterator it=find(name);
  if (it==end())
  {
    sw = false;
    return true;
  }
  else if (! it->second.dString.empty())
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81290", {"arg"}, {name.c_str()}, false);
    return false;
  }
  else
  {
    sw = true;
    return true;
  }
}

bool Arguments::getSwitch( const ArgName& name, bool& sw )
{
  bool ok = getSwitchConst(name, sw);
  if (ok)
    erase(name);

  return ok;
}

bool Arguments::getSwitchCompact( const ArgName& name )
{
  bool sw(false);
  bool ok = getSwitchConst(name, sw);
  if (ok)
  {
    erase(name);
    return sw;
  }
  else
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81300", {"arg"}, {name.c_str()}, false);
    throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
  }
}

void Arguments::checkEmptyArgs() const
{
  if (!empty())
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81310", {}, {}, false);

    for ( Arguments::const_iterator it=begin(); it!=end(); ++it )
    {
      std::cerr << "-" << it->first;
      if (!it->second.dString.empty())
        std::cerr << "=" << it->second.dString;
      std::cerr << " ";
    }
    std::cerr << std::endl;

    throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
  }
}
