#lang racket

(require syntax/parse/define
         racket/list
         "core.rkt")

(provide sp
         sp*)

;; A tiny s-expression based syntax for Spherepop terms.
;; Example:
;;   (sp ((a) (merge b c) (pop d)))
;; expands into nested mrg/pop/atom forms.

(define-syntax-parser sp
  [(_ expr)
   #'(sp* expr)] )

(define-syntax-parser sp*
  [(_ (pop t))
   #'(make-pop (sp* t))]
  [(_ (merge t1 t2))
   #'(make-merge (sp* t1) (sp* t2))]
  [(_ (atom lbl payload))
   #'(make-atom (make-region (symbol->string 'lbl) payload))]
  [(_ (t ...))
   ;; left-associative merge of subterms
   #'(let loop ([ts (list (sp* t) ...)])
       (cond
         [(null? ts) (error 'sp "empty merge")]
         [(null? (cdr ts)) (car ts)]
         [else (make-merge (car ts)
                           (loop (cdr ts)))]))]
  [(_ x)
   #'(make-atom (make-region (symbol->string 'x) '()))])
