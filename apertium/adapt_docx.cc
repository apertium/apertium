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
#include <stack>
#include <vector>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#include "utf8.h"
#include <utf8_fwrap.h>
#include <icu.h>
#else
#include "utf8/utf8.h"
#include "unicode/uchar.h"
#endif

using namespace std;

void endProgram(char *name)
{
	cout << basename(name) << ": makes some changes to .xml files from .docx components" << endl;
	cout << "- moves word boundaries to 'legal' positions" << endl;
	cout << "Gets input file name from stdin (or from -f), writes to stdout" << endl;
	cout << "USAGE: " << basename(name) << " [-n] [-p] [-f filename]" << endl;
	cout << "Options:" << endl;
#if HAVE_GETOPT_LONG
	cout << "  -n, --name:    writes \"< file name=\"filename\">\" to output" << endl;
	cout << "  -f, --file:    gets file name as parameter" << endl;
	cout << "  -p, --pretty:  outputs xml with a pretty format" << endl;
#else
	cout << "  -n:    writes \"< file name=\"filename\">\" to output" << endl;
	cout << "  -f:    gets file name as parameter" << endl;
	cout << "  -p:    outputs xml with a pretty format" << endl;
#endif
	exit(EXIT_FAILURE);
}

// Iterator for an xml tree
// Finds elements with given name and type
// Can search in embedded elements (if _goDown) or only at root level (siblings)
class Walker {
public:
	Walker(xmlNode *_root, string _name, bool _goDown, xmlElementType _type = xmlElementType::XML_ELEMENT_NODE);
	xmlNode *getCurrent() { return current; }
	xmlNode *findNext();
	static vector<xmlNode*> findAll(xmlNode *root, const char *name, bool goDown, xmlElementType _type = xmlElementType::XML_ELEMENT_NODE);
private:
	bool match(xmlNode *node);
	xmlNode *root, *current;
	stack<xmlNode*> higher;
	string name;
	bool goDown;
	xmlElementType type;
};

Walker::Walker(xmlNode *_root, string _name, bool _goDown, xmlElementType _type)
{
	root = _root;
	name = _name;
	type = _type;
	goDown = _goDown;
	if (root)
		higher.push(root);
	if (match(root))
		current = root;
	else
		current = findNext();
}

bool Walker::match(xmlNode *node)
{
	return node && (node->type == type) && (xmlStrcmp(node->name, (const xmlChar*)name.c_str()) == 0);
}

xmlNode *Walker::findNext()
{
	while (!higher.empty()) {
		xmlNode *node;
		xmlNode *top = higher.top();
		//string name = (char*)top->name;
		//cout << name << " " << endl;
		if (goDown && top->children) {
			node = top->children;
			higher.push(node);
			if (match(node)) {
				current = node;
				break;
			}
			continue;
		}
		while (!higher.empty() && !higher.top()->next)
			higher.pop();
		if (higher.empty()) {
			current = NULL;
			break;
		}
		node = higher.top()->next;
		higher.pop();
		higher.push(node);
		if (match(node)) {
			current = node;
			break;
		}
	}
	return current;
}

/*static*/
vector<xmlNode*> Walker::findAll(xmlNode *root, const char *name, bool goDown, xmlElementType type)
{
	vector<xmlNode*> found;
	if (root) {
		Walker walker(root, name, goDown, type);
		while (walker.getCurrent()) {
			found.push_back(walker.getCurrent());
			walker.findNext();
		}
	}
	return found;
}

// Text span with same properties
// A TextRun can have a "t" tag, whose content is a text fragment
// A TextRun can be the last run in its group; text cannot be moved
// between a last run and next ones.
class TextRun {
public:
	TextRun(xmlNode *_root);
	string getText() { return text; }
	void setText(string _text);
	bool hasText() { return containsText; }
	void setLast(bool _is) { last = _is; }
	bool isLast() { return last; }
	void remove();
private:
	xmlNode *root;
	xmlNode *text_0;
	string text;
	bool containsText;
	bool last;
};

TextRun::TextRun(xmlNode *_root)
{
	root = _root;
	text_0 = NULL;
	last = false;
	vector<xmlNode*> tt = Walker::findAll(root->children, "t", false);
	if (tt.size() == 1) {
		text_0 = tt[0];
		containsText = true;
		for (auto it = tt.begin(); it != tt.end(); it++) {
			Walker texts((*it)->children, "text", false, xmlElementType::XML_TEXT_NODE);
			while (texts.getCurrent()) {
				xmlNode *t = texts.getCurrent();
				if (xmlNodeGetSpacePreserve((xmlNode*)*it) == 1)
					text.append((const char*)t->content);
				else {
					string c((const char*)t->content);
					int newBegin = 0;
					int newEnd = c.length() - 1;
					while (newBegin <= newEnd && c[newBegin] == ' ')
						++newBegin;
					while (newEnd > newBegin && c[newEnd] == ' ')
						--newEnd;
					text.append(c.substr(newBegin, newEnd - newBegin + 1));
				}
				texts.findNext();
			}
		}
	}
	else
		containsText = false;
}

void TextRun::setText(string _text)
{
	xmlReplaceNode(text_0->children, xmlNewText((const xmlChar*)_text.c_str()));
	xmlNodeSetSpacePreserve(text_0, _text.front() == ' ' || _text.back() == ' ' ? 1 : 0);
}

void TextRun::remove()
{
	xmlUnlinkNode(root);
}

// A list of text runs
// Only runs than contain text are included in the list
class Paragraph {
public:
	Paragraph(xmlNode *_root);
	vector<TextRun*> getTextRuns() { return runs; };
private:
	xmlNode *root;
	vector<TextRun*> runs;
};

Paragraph::Paragraph(xmlNode *_root)
{
	root = _root;
	vector<xmlNode*> nodes = Walker::findAll(root->children, "r", false);
	for (auto it = nodes.begin(); it != nodes.end(); it++) {
		TextRun *r = new TextRun(*it);
		if (r->hasText())
			runs.push_back(r);
		else
			delete r;
	}
	if (!runs.empty())
		runs.back()->setLast(true);
}

// Show text of paragraphs (for debuggins purposes)
void showParagraphs(xmlNode *root)
{
	vector<xmlNode*> pp = Walker::findAll(root, "p", true);
	cout << pp.size() << " paragraphs found" << endl;
	for (auto it1 = pp.begin(); it1 != pp.end(); it1++) {
		Paragraph p(*it1);
		bool first = true;
		vector<TextRun*> rr = p.getTextRuns();
		for (auto it2 = rr.begin(); it2 != rr.end(); it2++) {
			if (!first)
				cout << "|";
			cout << (*it2)->getText();
			//(*it2)->setText(" " + (*it2)->getText() + " ");
			first = false;
		}
		cout << endl;
	}
}

// Find a suitable boundary at begin position or to the right
// No boundary is allowed in groups "l·l" and "alpha-alpha"
// TODO: adapt to source language
size_t findBoundary(string _text, size_t begin)
{
	size_t length = _text.length();
	if (begin == 0 || begin >= length)
		return begin;
	const unsigned char *text = (unsigned char *) _text.c_str();
	utf8::iterator<const unsigned char*> textBegin(text, text, text + length);
	utf8::iterator<const unsigned char*> textEnd(text + length, text, text + length);
	utf8::iterator<const unsigned char*> it(text + begin, text, text + length);
	utf8::iterator<const unsigned char*> minBoundary = it;
	uint32_t cur = *it, cur_1 = 0, cur_2 = 0;
	utf8::iterator<const unsigned char*> it2 = it;
	cur_1 = *(--it2);
	if (it2 != textBegin)
		cur_2 = *(--it2);
	bool advance, expectL, expectAlpha;
	while (true) {
		advance = expectL = expectAlpha = false;
		if ((u_isalpha(cur_1) && u_isalpha(cur)))					// ?a|a
			advance = true;
		else if (u_isalpha(cur_2) && cur_1 == '-')					// a-|?
			advance = u_isalpha(cur);
		else if (u_isalpha(cur_1) && cur == '-')					// ?a|-
			expectAlpha = true;
		else if ((cur_2 == 'L' || cur_2 == 'l') && cur_1 == 0xB7)	// l·|?
			advance = (cur == 'L' || cur == 'l');
		else if ((cur_1 == 'L' || cur_1 == 'l') && cur == 0xB7)		// ?l|·
			expectL = true;
		if (advance || expectAlpha || expectL)
		{
			cur_2 = cur_1;
			cur_1 = cur;
			if (++it == textEnd)
				break;
			else
				cur = *it;
			if (expectAlpha && !u_isalpha(cur)) {
				--it;
				break;
			}
			if (expectL && !(cur == 'L' || cur == 'l')) {
				--it;
				break;
			}
		}
		else
			break;
	}
	return begin + it.base() - minBoundary.base();
}

// Adjusts the boundaries between adjacent text runs
// Eventually removes text runs that have become empty
void adjust(vector<TextRun*> runs, vector<size_t> lengths, string join)
{
	if (runs.size() == 1)
		return;
	bool changes = false;
	size_t left = 0, right = 1;
	size_t endLeft = lengths[0];
	while (right < runs.size()) {
		if (left == right) {
			++right;
			continue;
		}
		if (lengths[left] == 0) {
			++left;
			endLeft += lengths[left];
			continue;
		}
		size_t boundary = findBoundary(join, endLeft);
		size_t toMove = boundary - endLeft;
		if (toMove > 0) {
			changes = true;
			while (toMove > 0 && right < runs.size()) {
				if (toMove >= lengths[right]) {
					lengths[left] += lengths[right];
					endLeft += lengths[right];
					toMove -= lengths[right];
					lengths[right] = 0;
					++right;
				}
				else {
					lengths[left] += toMove;
					endLeft += toMove;
					lengths[right] -= toMove;
					toMove = 0;
				}
			}
		}
		++left;
		endLeft += lengths[left];
	}
	if (changes) {
		int begin = 0;
		for (size_t i = 0; i < runs.size(); ++i) {
			if (lengths[i] == 0)
				runs[i]->remove();
			else {
				runs[i]->setText(join.substr(begin, lengths[i]));
				begin += lengths[i];
			}
		}
	}
}

// Process the document
// For each paragraph, finds text runs and adjusts them
void process(xmlNode *root)
{
	//showParagraphs(root);
	vector<xmlNode*> paragraphs = Walker::findAll(root, "p", true);
	for (auto itp = paragraphs.begin(); itp != paragraphs.end(); itp++) {
		Paragraph paragraph(*itp);
		vector<TextRun*> allRuns = paragraph.getTextRuns();
		string joinedText;
		vector<size_t> lengths;
		vector<TextRun*> adjustRuns;
		for (auto itr = allRuns.begin(); itr != allRuns.end(); itr++) {
			TextRun *run = *itr;
			adjustRuns.push_back(run);
			lengths.push_back(run->getText().length());
			joinedText.append(run->getText());
			if (run->isLast()) {
				adjust(adjustRuns, lengths, joinedText);
				adjustRuns.clear();
				lengths.clear();
				joinedText.clear();
			}
		}
	}
	//showParagraphs(root);
}

inline const unsigned char* printNl2spc(const unsigned char *p) {
	while (*p && *p != '\x0A' && *p != '\x0D')
		cout << *(p++);
	cout << ' ';
	while (*p == '\x0A' || *p == '\x0D')
		++p;
	return p;
}


// Opens a document, processes it and sends it to stdout
// Can output the name (<file name="XXX"/>
// The result can be pretty
void process(string fileName, bool outputsName, bool pretty)
{
	xmlDoc *document = xmlReadFile(fileName.c_str(), NULL, XML_PARSE_NOENT);
	if (document == NULL) {
		cerr << "error: could not parse file \"" << fileName << "\"" << endl;
		return;
	}
	process(xmlDocGetRootElement(document));
	xmlChar *buffer;
	int sizeBuffer;
	xmlDocDumpFormatMemory(document, &buffer, &sizeBuffer, pretty ? 1: 0);
	if (outputsName)
		cout << "<file name=\"" << fileName << "\"/> ";
	const unsigned char *p = buffer;
	p = printNl2spc(p);
	if (pretty)
		cout << p;
	else
		do
			p = printNl2spc(p);
		while (*p);
	cout << endl;
        xmlFree(buffer);
	xmlFreeDoc(document);
}

int main(int argc, char *argv[])
{

	bool outputsName = false;
	bool pretty = false;
	bool argF = false;
	string name;

#if HAVE_GETOPT_LONG
	static struct option long_options[] =
	{
	  {"name",   0, 0, 'n'},
	  {"pretty", 0, 0, 'n'},
	  {"file",   0, 0, 'n'},
	  {"help",   0, 0, 'h'}
	};
#endif

	while (true)
	{
#if HAVE_GETOPT_LONG
		int option_index;
		int c = getopt_long(argc, argv, "npf:h", long_options, &option_index);
#else
		int c = getopt(argc, argv, "npf:h");
#endif

		if (c == -1)
		{
			break;
		}

		switch (c)
		{
		case 'n':
			outputsName = true;
			break;
		case 'p':
			pretty = true;
			break;
		case 'f':
			argF = true;
			name = optarg;
			break;
		case 'h':
		default:
			endProgram(argv[0]);
			break;
		}
	}
	if (argF) {
		// cerr << "out: " << outputsName << ", name: " << name << ", pretty: " << pretty << endl;
		process(name, outputsName, pretty);
	}
	else {
		while (cin >> name) {
			// cerr << "out: " << outputsName << ", name: " << name << ", pretty: " << pretty << endl;
			process(name, outputsName, pretty);
		}
	}
	cout << '\0' << flush;
}

/* vim: set noet ts=4 sw=4: */
