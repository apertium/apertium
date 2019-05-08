/*
 * Copyright (C) 2019 Joan Moratinos Jaume
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <string>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#include <utf8_fwrap.h>
#endif

using namespace std;

void endProgram(char *name)
{
  cout << basename(name) << ": makes some changes to .xml files from .docx components" << endl;
  cout << "- moves word boundaries to 'legal' positions" << endl;
  cout << "Reads file name from stdin, writes to stdout" << endl;
  cout << "USAGE: " << basename(name) << " [-n]" << endl;
  cout << "Options:" << endl;
#if HAVE_GETOPT_LONG
  cout << "  -n, --name:    writes file name to output" << endl;
#else
  cout << "  -n, --name:    writes file name to output" << endl;
#endif
  exit(EXIT_FAILURE);
}

void extreuText(xmlNode *node, string &text) {
	if (node->type == XML_TEXT_NODE)
		text.append((const char*)node->content);
	else {
		while (node) {
			xmlNode *child  = node->children;
			while (child) {
				extreuText(child, text);
				child = child->next;
			}
			node = node->next;
		}
	}
}

string extreuText(xmlNode *node)
{
	string text;
	extreuText(node, text);
	return text;
}

void processP(xmlNode *p)
{
	cout << p->name << " ((" << extreuText(p) << ")) ";
}

void process(xmlNode *root)
{
	cout << root->name << " ";
	for (xmlNode *node=root->children; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE) {
			if (xmlStrcmp(node->name, (const xmlChar*) "p") == 0)
				processP(node);
			else
				process(node);
		}
	}
}

void process(string name)
{
	xmlDoc *doc = xmlReadFile(name.c_str(), NULL, 0);
	if (doc == NULL) {
    cerr << "error: could not parse file \"" << name << "\"" << endl;
    return;
  }
	process(xmlDocGetRootElement(doc));
	xmlChar *buffer;
	int sizeBuffer;
	xmlDocDumpFormatMemory(doc, &buffer, &sizeBuffer, 1);
	cout << (const char*) buffer << endl;
	xmlFree(buffer);
	xmlFreeDoc(doc);
}

#define MAX_LEN_NAME 200

int main(int argc, char *argv[])
{

	bool outputName = false;
	string name;

#if HAVE_GETOPT_LONG
  static struct option long_options[]=
    {
      {"name", 0, 0, 'n'},
      {"help", 0, 0, 'h'}
    };
#endif

  while(true)
  {
#if HAVE_GETOPT_LONG
    int option_index;
    int c = getopt_long(argc, argv, "n", long_options, &option_index);
#else
    int c = getopt(argc, argv, "n");
#endif

    if(c == -1)
    {
      break;
    }

    switch(c)
    {
    case 'n':
      outputName = true;
      break;
    case 'h':
    default:
      endProgram(argv[0]);
      break;
    }
  }
	cin >> name;
	if (outputName)
		cout << "<file name=\"" << name << "\"> ";
	process(name);
}

/* vim: set noet ts=2 sw=2: */
