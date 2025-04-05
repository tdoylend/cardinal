# Design Notes - The Basics

Cardinal is a new programming language I am designing. Assuming everything
goes according to plan, it will be the daily driver for most of my work at my
current company (where we write software tools for around half a dozen fields)
and also for most of my hobby game and app development.

Cardinal is heavily based on Wren and copies most of its syntax and concepts
with some modifications here and there. Forking an existing language might
seem unoriginal. This is correct; I do not think originality is a virtue in
programming language design unless there are very clear benefits to the 
language's usability as a result.

Wren has been field-tested: I have written a reasonably complex multiplayer
game in it as well as a spec-compliant JSON parser and Mustache renderer.
This assured me it was a language I could enjoy writing big programs
in. I also taught the language to a few people to make sure it was easy for
newcomers to pick up; Wren passed both tests well. So I felt confident that
it would make a good base, and in fact I nearly used it as-is.

However, Wren does lack a few features that I would like - for example, better
error messages (Wren's are quite terse) and explicit field declarations (versus
implicit). The latter makes Cardinal *incompatible* with Wren; Wren code will
generally not run in Cardinal without some changes. So: in for a
penny, in for a pound! Why not investigate the
other features of the language and either redesign them or find good reasons
not to?

This set of articles is my thought process on the design of a language I hope
to use in production for the next few years, hopefully decades. I am not a 
language design expert - no Niklaus Wirth here, nor Bob Nystrom - just someone
who wants a good language to get stuff done in and has a fairly sharp sense
of what they want to that end. Hopefully the next person can get more right by
looking at what I've done wrong.
