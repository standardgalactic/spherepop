#lang info
(define collection "spherepop-lib")
(define deps '("base" "typed-racket-lib" "rackunit-lib"))
(define build-deps '("scribble-lib"))
(define scribblings '(("scribblings/spherepop.scrbl" () (library))))

(define pkg-desc "Spherepop: a geometric merge–collapse calculus")
(define version "0.1.0")
