#lang scribble/manual

@title{Spherepop: A Geometric Merge--Collapse Calculus}
@author{Flyxion}

@section{Overview}

Spherepop is a small calculus where values are @emph{regions} and
computation proceeds by two primitive operations:

@itemlist[
  @item{@bold{merge}: union of regions followed by collapse}
  @item{@bold{collapse}: abstraction that removes interior detail}
]

This library provides a reference implementation, a tiny surface
syntax, and a typed core suitable for experimentation in Racket.

@section{Core Concepts}

@include-section["core.scrbl"]
