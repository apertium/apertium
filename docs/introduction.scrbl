#lang scribble/manual

@title{Introduction}

This documentation describes the Apertium platform, one of
the open-source machine translation systems which originated
within the project ``Open-Source Machine Translation for the
Languages of Spain'' (``Traducción automática de código
abierto para las lenguas del estado español''). It is a
shallow-transfer machine translation system, initially
designed for the translation between related language pairs,
although some of its components have been also used in the
deep-transfer architecture
(@hyperlink["https://github.com/matxin"]{Matxin}) that has
been developed in the same project for the pair
Spanish-Basque.

Existing machine translation systems available at present
for the pairs @tt{es}--@tt{ca} and @tt{es}--@tt{gl} are
mostly commercial or use proprietary technologies, which
makes them very hard to adapt to new usages; furthermore,
they use different technologies across language pairs, which
makes it very difficult to integrate them in a single
multilingual content management system.

One of the main novelties of the architecture described here
is that it has been released under open-source licenses (in
most cases, GNU GPL; some data still have a Creative Commons
license) and is distributed free of charge. This means that
anyone having the necessary computational and linguistic
skills will be able to adapt or enhance the platform or the
language-pair data to create a new machine translation
system, even for other pairs of related languages. The
licenses chosen make these improvements immediately
available to everyone. We therefore expect that the
introduction of this of open-source machine translation
architecture will solve some of the mentioned problems
(having different technologies for different pairs,
closed-source architectures being hard to adapt to new uses,
etc.) and promote the exchange of existing linguistic data
through the use of the XML-based formats defined in this
documentation. On the other hand, we think that it will help
shift the current business model from a license-centered one
to a services-centered one.

It is worth mentioning that ``Open-Source Machine Translation
for the Languages of Spain'' was the first large open-source
machine translation project funded by the central Spanish
Government, although the adoption of open-source software by
the Spanish governments is not new.