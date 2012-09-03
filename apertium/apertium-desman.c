/*
 * Copyleft Bernard Chardonneau (2012) for initial version
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
 *
 * Deformater for man files before Apertium translation.
 *
 */

#include <stdio.h>
#include <ctype.h>


int char_to_escape (int caract)
{
    switch (caract)
    {
        case '[' :
        case ']' :
        case '<' :
        case '>' :
        case '{' :
        case '}' :
        case '\\':
        case '/' :
        case '^' :
        case '$' :
        case '@' : return 1;
                   // no break necessary here

        default  : return 0;
    }
}


int main (int argc, char **argv)
{
    int lastchar;  // last character read on input. Must be an int to test EOF
    int debl;      // to memorise if we are at the beginning of a line
    int in_tag;    // to memorise if we are inside a [...] or not


    // input from a file if needed
    if (--argc >= 1)
    {
        if (! freopen (argv [1], "r", stdin))
        {
            fprintf (stderr, "Cannot access to file %s\n", argv [1]);
            return (1);
        }
    }

    // output into a file if needed
    if (argc >= 2)
    {
        if (! freopen (argv [2], "w", stdout))
        {
            fprintf (stderr, "Cannot write result to file %s\n", argv [2]);
            return (2);
        }
    }

    // beginning
    lastchar = getchar ();
    debl = 1;
    in_tag = 0;

    // while input was not read completly
    while (lastchar != EOF)
    {
        // if we are at the beginning of a line starting by a '.'
        if (debl && (lastchar == '.'))
        {
            // the . followed by a keyvord will not be translated
            if (! in_tag)
            {
                in_tag = 1;
                putchar ('[');
            }

            do
            {
                putchar (lastchar);
                lastchar = getchar ();
            }
            while (isalpha (lastchar));

            putchar (']');
            in_tag = 0;
            debl = 0;
        }
        // else if we are at the end of a line
        else if (lastchar == '\n')
        {
            // we do the same as apertium-destxt
            if (! in_tag)
            {
                in_tag = 1;
                putchar ('[');
            }

            do
            {
                putchar (lastchar);
                lastchar = getchar ();
            }
            while (lastchar == '\n');

            debl = 1;
        }
        // else if we find a -
        else if (lastchar == '-')
        {
            // text the next character
            lastchar = getchar ();

            // if the - is followed by what looks like a command option
            if (isalnum (lastchar))
            {
                // the option will not be translated
                if (! in_tag)
                {
                    in_tag = 1;
                    putchar ('[');
                }

                putchar ('-');

                do
                {
                    putchar (lastchar);
                    lastchar = getchar ();
                }
                while (isalnum (lastchar));

                putchar (']');
                in_tag = 0;
            }
            else
            {
                // else isolated - are processed like other characters
                if (in_tag)
                {
                    putchar (']');
                    in_tag = 0;
                }

                putchar ('-');
            }

            debl = 0;
        }
        // else (text to be translated)
        else
        {
            if (in_tag)
            {
                putchar (']');
                in_tag = 0;
            }

            // special char must be escaped
            if (char_to_escape (lastchar))
                putchar ('\\');

            putchar (lastchar);
            lastchar = getchar ();
            debl = 0;
        }
    }

    // close the last [ oppened
    if (in_tag)
        putchar (']');

    // end of file
    return (0);
}
