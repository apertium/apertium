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
 * Generic reformater for mnemonic interface files and man pages
 * after an Apertium translation.
 *
 * This source file permit to generate both apertium-remnemo and
 * apertium-reman executables.
 *
 */

#include <stdio.h>
#include <ctype.h>


int main (int argc, char **argv)
{
    int lastchar;   // last character read on input. Must be an int to test EOF


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

    // while input was not read completly
    while (lastchar != EOF)
    {
        // if '.' found
        // skip completly ".[]" strings but output the '.' in the other cases
        if (lastchar == '.')
        {
            lastchar = getchar ();

            if (lastchar == '[')
            {
                lastchar = getchar ();

                if (lastchar != ']')
                    putchar ('.');
            }
            else
                putchar ('.');
        }
        // other characters
        else
        {
            // there may be plenty of more complicated solutions
            // but this simple one has my preference
            switch (lastchar)
            {
                // skip [ and ]
                case '[' :
                case ']' : break;

                // skip \ but always output the next character
                case '\\': lastchar = getchar ();
                           putchar (lastchar);
                           break;

                // output other characters
                default  : putchar (lastchar);
                           break;
            }

            // read the next character
            lastchar = getchar ();
        }
    }

    // end of file
    return (0);
}
