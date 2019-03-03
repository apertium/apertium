#lang scribble/manual

@(require racket/date)

@title[#:version "3.0"]{Documentation of the Open-Source Shallow-Transfer
 Machine-Translation Platform @emph{Apertium}}

@author["Mikel L. Forcada" "Boyan Ivanov Bonev" "Sergio Ortiz Rojas"
        "Juan Antonio Pérez Ortiz" "Gema Ramírez Sánchez"
        "Felipe Sánchez Martínez" "Carme Armentano-Oller" "Marco A. Montava"
        "Francis M. Tyers" "Mireia Ginestí Rosell (editor)"
        "(version 2.0)"
        "Ilnar Salimzianov (this document)"]

@smaller{Copyright © 2007 Grup Transducens, Universitat
 d'Alacant. Permission is granted to copy, distribute and/or
 modify this document under the terms of the GNU Free
 Documentation License, Version 1.2 or any later version
 published by the Free Software Foundation; with no Invariant
 Sections, no Front-Cover Texts, and no Back-Cover Texts. A
 copy of the license can be found in \url{
  http://www.gnu.org/copyleft/fdl.html}.}

Version 2.0 of the official Apertium documentation can be
found
@hyperlink["http://xixona.dlsi.ua.es/~fran/apertium2-documentation.pdf"]{
 here}.

In addition, there is a lot of information on the
@hyperlink["http://wiki.apertium.org"]{wiki} of the project.

The goal of tihs document is two-fold:

@itemlist[

 @item{describe what has changed in Apertium since the above
  mentioned documentation has been published (especially as a
  result of contributions made by the Google Summer of
  Code students), and}

 @item{consolidate the most important information spread
  across many different wiki pages into a single document
  (with pointers to more information on the wiki and
  elsewhere).}

 ]

 @table-of-contents[]

@include-section["introduction.scrbl"]

@include-section["../../apertium-lex-tools/docs/index.scrbl"]

@include-section["../../apertium-separable/docs/index.scrbl"]