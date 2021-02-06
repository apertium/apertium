/*
 * Copyright (C) 2021 Daniel Swanson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <lttoolbox/lt_locale.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "string_utils.h"
#include "utf_converter.h"
#include <libgen.h>
#include <getopt.h>
#include <libxml/xmlreader.h>
#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <set>

using namespace Apertium;
using namespace std;
using namespace std::filesystem;

// TODO TODO TODO - properly get this from Makefile.am
const char* APERTIUMDIR = "/usr/local/share/apertium";

void endProgram(char *name)
{
  cout << basename(name) << ": " << endl;
  cout << "USAGE: " << basename(name) << " [-fvh] modes.xml [install_path]" << endl;
  cout << "  -f, --full:      treat install_path as absolute (defaults to " << APERTIUMDIR << "/{install_path}" << endl;
  cout << "  -v, --verbose:   print more detailed messages" << endl;
  cout << "  -h, --help:      display this help" << endl;
  exit(EXIT_FAILURE);
}

struct program {
  string command;
  vector<string> debug_suffix;
  // <argument, is_file>
  vector<pair<string, bool>> arguments;
};

struct pipeline {
  string name;
  bool install;
  bool debug;
  vector<program> steps;
};

bool xmlEq(const xmlChar* a, const char* b)
{
  return !xmlStrcmp(a, (const xmlChar*) b);
}

string xml2str(const xmlAttr* a)
{
  //return StringUtils::trim(UtfConverter::fromUtf8((const char*) a->children->content));
  return string((const char*) a->children->content);
}

void read_program(xmlNode* node, pipeline& pipe)
{
  program step;
  for(xmlAttr* a = node->properties; a != NULL; a = a->next) {
    if(xmlEq(a->name, "name")) {
      step.command = xml2str(a);
    } else if(xmlEq(a->name, "debug-suff")) {
      step.debug_suffix.push_back(xml2str(a));
    }
  }
  for(xmlNode* arg = node->children; arg != NULL; arg = arg->next) {
    if(arg->type != XML_ELEMENT_NODE) continue;
    for(xmlAttr* a = arg->properties; a != NULL; a = a->next) {
      if(xmlEq(a->name, "name")) {
	bool is_file = xmlEq(a->name, "file");
	step.arguments.push_back(make_pair(xml2str(a), is_file));
	break;
      }
    }
  }
  pipe.steps.push_back(step);
}

void read_modes(xmlNode* doc, map<string, pipeline>& modes)
{
  for(xmlNode* mode = doc->children; mode != NULL; mode = mode->next) {
    if(mode->type != XML_ELEMENT_NODE) continue;
    pipeline pipe;
    for(xmlAttr* a = mode->properties; a != NULL; a = a->next) {
      if(xmlEq(a->name, "name")) {
	pipe.name = xml2str(a);
      } else if(xmlEq(a->name, "install")) {
	pipe.install = xmlEq(a->children->content, "yes");
      } else if(xmlEq(a->name, "gendebug")) {
	pipe.debug = xmlEq(a->children->content, "yes");
      }
    }
    for(xmlNode* pipe_x = mode->children; pipe_x != NULL; pipe_x = pipe_x->next) {
      if(pipe_x->type != XML_ELEMENT_NODE) continue;
      for(xmlNode* prog = pipe_x->children; prog != NULL; prog = prog->next) {
	if(prog->type != XML_ELEMENT_NODE) continue;
	read_program(prog, pipe);
      }
    }
    modes[pipe.name] = pipe;
  }
}

bool startswith(const string& cmd, const string& prog)
{
  return (cmd.size() >= prog.size()) && (cmd.compare(0, prog.size(), prog) == 0);
}

void set_debug_suffixes(pipeline& prog)
{
  for(auto& cmd : prog.steps) {
    if(!cmd.debug_suffix.empty()) continue;
    const string& c = cmd.command;
    if(startswith(c, "cg-proc")) {
      cmd.debug_suffix.push_back("-disam");
    } else if(startswith(c, "apertium-tagger")) {
      cmd.debug_suffix.push_back("-tagger");
    } else if(startswith(c, "apertium-pretransfer")) {
      cmd.debug_suffix.push_back("-pretransfer");
    } else if(startswith(c, "lrx-proc")) {
      cmd.debug_suffix.push_back("-lex");
      cmd.debug_suffix.push_back("-lextor");
    } else if(startswith(c, "apertium-transfer")) {
      if(c.rfind("-n") != string::npos) {
	cmd.debug_suffix.push_back("-transfer2");
      } else {
	cmd.debug_suffix.push_back("-chunker");
	cmd.debug_suffix.push_back("-transfer");
      }
    } else if(startswith(c, "apertium-interchunk")) {
      cmd.debug_suffix.push_back("-interchunk");
    } else if(startswith(c, "apertium-postchunk")) {
      cmd.debug_suffix.push_back("-postchunk");
    } else if(c.rfind("$1") != string::npos) {
      cmd.debug_suffix.push_back("-dgen");
      cmd.debug_suffix.push_back("-generador");
    } else if(startswith(c, "lt-proc")) {
      if(c.rfind("-b") != string::npos) {
	cmd.debug_suffix.push_back("-biltrans");
      } else if(c.rfind("-p") != string::npos) {
	cmd.debug_suffix.push_back("-pgen");
      } else {
	cmd.debug_suffix.push_back("morph");
	if(c.rfind("-g") == string::npos) {
	  cmd.debug_suffix.push_back("anmor");
	}
      }
    } else if(startswith(c, "hfst-proc")) {
      cmd.debug_suffix.push_back("-morph");
      if(c.rfind("-g") == string::npos) {
	cmd.debug_suffix.push_back("anmor");
      }
    } else if(startswith(c, "lsx-proc")) {
      cmd.debug_suffix.push_back("-autoseq");
    } else if(startswith(c, "rtx-proc")) {
      cmd.debug_suffix.push_back("-transfer");
    } else if(startswith(c, "apertium-anaphora")) {
      cmd.debug_suffix.push_back("-anaph");
    } else {
      cmd.debug_suffix.push_back("-NAMEME");
    }
  }
}

void set_trace_opt(pipeline& mode)
{
  string& cmd = mode.steps.back().command;
  if(startswith(cmd, "cg-proc") || startswith(cmd, "lrx-proc") ||
     startswith(cmd, "apertium-transfer") ||
     startswith(cmd, "apertium-interchunk") ||
     startswith(cmd, "apertium-postchunk")) {
    cmd += " -t";
  } else if(startswith(cmd, "rtx-proc")) {
    cmd += " -r";
  } else if(startswith(cmd, "apertium-anaphora")) {
    cmd += " -d";
  }
}

void gen_debug_modes(map<string, pipeline>& modes)
{
  vector<string> todo;
  for(auto& m : modes) {
    if(m.second.debug) {
      todo.push_back(m.first);
      set_debug_suffixes(m.second);
    }
  }
  for(auto& mode_name : todo) {
    pipeline& mode = modes[mode_name];
    for(size_t i = 0; i < mode.steps.size(); i++) {
      for(auto& suff : mode.steps[i].debug_suffix) {
	pipeline debug;
	debug.name = mode_name + suff;
	debug.install = false;
	debug.debug = false;
	if(modes.find(debug.name) == modes.end()) {
	  debug.steps.assign(mode.steps.begin(),mode.steps.begin()+i+1);
	  set_trace_opt(debug);
	  modes[debug.name] = debug;
	}
	pipeline untrimmed;
	untrimmed.name = "@" + debug.name;
	untrimmed.install = false;
	untrimmed.debug = false;
	if(modes.find(untrimmed.name) == modes.end()) {
	  untrimmed.steps = debug.steps;
	  for(auto& step : untrimmed.steps) {
	    for(auto& str : step.arguments) {
	      size_t loc = str.first.rfind(".automorf.");
	      if(loc != string::npos) {
		str.first.replace(loc, 10, ".automorf-untrimmed.");
	      }
	    }
	  }
	  set_trace_opt(untrimmed);
	  modes[untrimmed.name] = untrimmed;
	}
      }
    }
  }
}

void gen_mode(pipeline& mode, path& file_dir, path& write_dir)
{
  path modefile = write_dir / (mode.name + ".mode");
  ofstream f;
  f.open(modefile);
  for(size_t i = 0; i < mode.steps.size(); i++) {
    if(i != 0) {
      f << " | ";
    }
    program& p = mode.steps[i];
    f << p.command;
    for(auto& arg : p.arguments) {
      if(arg.second) {
	f << " '" << absolute(file_dir / arg.first).c_str() << "'";
      } else {
	f << " " << arg.first;
      }
    }
  }
  f.close();
}

void gen_modes(map<string, pipeline>& modes, path& install_dir, path& dev_dir)
{
  bool installing = (install_dir != dev_dir);
  path file_dir;
  path write_dir;
  if(installing) {
    file_dir = install_dir;
    write_dir = install_dir;
  } else {
    file_dir = dev_dir;
    write_dir = dev_dir / "modes";
  }
  for(auto& mode : modes) {
    if(installing && !mode.second.install) continue;
    gen_mode(mode.second, file_dir, write_dir);
  }
}

int main(int argc, char* argv[])
{
  LtLocale::tryToSetLocale();

#if HAVE_GETOPT_LONG
  static struct option long_options[] =
    {
     {"full",       0, 0, 'f'},
     {"verbose",    0, 0, 'v'},
     {"help",       0, 0, 'h'}
    };
#endif

  bool full = false;
  bool verbose = false;

  while(true) {
#if HAVE_GETOPT_LONG
    int option_index;
    int c = getopt_long(argc, argv, "fvh", long_options, &option_index);
#else
    int c = getopt(argc, argv, "fvh");
#endif

    if(c == -1) {
      break;
    }

    switch(c) {
    case 'f':
      full = true;
      break;

    case 'v':
      verbose = true;
      break;

    case 'h':
    default:
      endProgram(argv[0]);
      break;
    }
  }

  if((argc - optind > 2) || (argc - optind == 0)) {
    endProgram(argv[0]);
  }

  path apertium_path = APERTIUMDIR;
  path xml_path = argv[optind];
  path dev_dir = xml_path.parent_path();
  path install_dir;
  if(argc - optind == 1) {
    install_dir = dev_dir;
  } else {
    if(full) {
      install_dir = argv[optind+1];
    } else {
      install_dir = apertium_path / path(argv[optind+1]);
    }
    if(install_dir == dev_dir) {
      cerr << basename(argv[0]) << " ERROR: Installation prefix is the same directory as modes.xml; give a different INSTALLDIR." << endl;
      exit(EXIT_FAILURE);
    }
  }

  if(!exists(dev_dir / "modes")) {
    create_directory(dev_dir / "modes");
  }

  xmlDoc* doc = xmlReadFile(xml_path.c_str(), NULL, 0);
  if(doc == NULL) {
    cerr << "Error: Could not parse file '" << xml_path << "'." << endl;
    exit(EXIT_FAILURE);
  }

  map<string, pipeline> modes;
  read_modes(xmlDocGetRootElement(doc), modes);
  gen_debug_modes(modes);
  gen_modes(modes, install_dir, dev_dir);
}
