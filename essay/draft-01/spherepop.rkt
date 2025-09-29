#lang racket

;; Spherepop Calculus: Evaluator Skeleton with Macros
(provide sphere pop merge choice rotate lit-unit lit-bool lit-nat if add choice-n bind fold-or fold-and)

;; Core Syntax (minimal)
(define-syntax-rule (sphere (x t) body) (Sphere x t body))
(define-syntax-rule (pop f u) (Pop f u))
(define-syntax-rule (merge t u) (Merge t u))
(define-syntax-rule (choice p t u) (Choice p t u))
(define-syntax-rule (rotate k t) (Rotate k t))
(define-syntax-rule (lit-unit) (LitUnit))
(define-syntax-rule (lit-bool b) (LitBool b))
(define-syntax-rule (lit-nat n) (LitNat n))
(define-syntax-rule (if b t u) (If b t u))
(define-syntax-rule (add t u) (Add t u))

;; Extended Syntax (macros)
(define-syntax-rule (choice-n (p1 t1) (p2 t2) ...)
  (choice p1 t1 (choice-n (p2 t2) ...)))
(define-syntax-rule (choice-n (p t)) t)

(define-syntax-rule (bind (x <- t) u)
  (pop (sphere (x Any) u) t))

(define-syntax (fold-or stx)
  (syntax-case stx ()
    [(_ (merge t1 t2)) #'(if t1 (lit-bool #t) (fold-or t2))]
    [(_ b) #'b]))

(define-syntax (fold-and stx)
  (syntax-case stx ()
    [(_ (merge t1 t2)) #'(if t1 (fold-and t2) (lit-bool #f))]
    [(_ b) #'b]))

;; Evaluator Skeleton (to be implemented)
(define (eval term)
  (error "Evaluator not implemented"))
