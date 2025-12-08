#lang racket

(require rackunit
         "../core.rkt")

(define s default-collapse-strategy)

(define a (make-region "a" '(1)))
(define b (make-region "b" '(2)))

(define ta (make-atom a))
(define tb (make-atom b))

(check-true (region? a))
(check-equal? (region-label (eval-term s ta)) "a")

(define r (eval-term s (make-merge ta tb)))
(check-true (region? r))
