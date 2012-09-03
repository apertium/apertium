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
 * Deformater for mnemonic interface files before Apertium translation.
 *
 * Files to be used are made of lines with the following format :
 *
 * Mnemonic             "printf_formated_string"
 *
 * The mnemonic starts at colomn 1 and only the printf_formated_string
 * must be translated by Apertium.
 * Comments starting by a # at column 1 are also allowed.
 *
 */

#include <stdio.h>
#include <ctype.h>


/*
 *  returns 1 if a \ needs to be inserted before the character
 *  0 otherwise
 */
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


/*
 *  main function
 */
int main (int argc, char **argv)
{
    int lastchar;  // last character read on input. Must be an int to test EOF


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

    lastchar = getchar ();
    putchar ('[');

    while (lastchar != EOF)
    {
        // reading and copying mnemonic inside [... ]
        // it works also with several blank lines
        while (! isblank (lastchar) && lastchar != EOF)
        {
            // no char to escape may be in a mnemonic, but it is more sure
            // to test that and it can be usefull for comment lines
            if (char_to_escape (lastchar))
                putchar ('\\');

            putchar (lastchar);
            lastchar = getchar ();
        }

        // reading and copying spaces before the printf formatted string
        while (isblank (lastchar))
        {
            putchar (lastchar);
            lastchar = getchar ();
        }

        // normally the printf formated string begins with a "
        // but there can be comments here instead
        if (lastchar == '"')
        {
            putchar (lastchar);
            lastchar = getchar ();
        }

        // if the printf formatted string starts by \n \r or another
        // this special character will not be translated
        while (lastchar == '\\')
        {
            putchar (lastchar);
            putchar (lastchar);     // 2 '\' instead of one needed
            lastchar = getchar ();
            putchar (lastchar);
            lastchar = getchar ();
        }

        // now, the end of the line will be translated
        putchar (']');

        while (lastchar != '"' && lastchar != '\n' && lastchar != EOF)
        {
            // except for \? sequences
            if (lastchar == '\\')
            {
                putchar ('[');

                do
                {
                    putchar (lastchar);
                    putchar (lastchar);     // 2 '\' instead of one needed
                    lastchar = getchar ();
                    putchar (lastchar);
                    lastchar = getchar ();
                }
                while (lastchar == '\\');

                putchar (']');
            }

            // and % sequences
            else if (lastchar == '%')
            {
                putchar ('[');

                do
                {
                    putchar (lastchar);
                    lastchar = getchar ();
                }
                while (isdigit (lastchar) || lastchar == '.');

                putchar (lastchar);
                putchar (']');
                lastchar = getchar ();
            }
            // but the rest is translated
            else
            {
                // special char must be escaped
                if (char_to_escape (lastchar))
                    putchar ('\\');

                putchar (lastchar);
                lastchar = getchar ();
            }
        }

        // to say each printf formatted string must be translated
        // without taking into account previous printf formatted strings
        if (lastchar == '"')
            printf (".[]");

        // end of printf formatted string reached
        putchar ('[');

        while (lastchar != '\n' && lastchar != EOF)
        {
            putchar (lastchar);
            lastchar = getchar ();
        }
    }

    // end of file
    putchar (']');
    return (0);
}
