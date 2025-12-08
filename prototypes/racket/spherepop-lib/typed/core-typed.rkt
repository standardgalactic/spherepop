#lang typed/racket

(require/typed racket/match [match (All (A) (-> Any * A))])
(provide Region
         region
         region-label
         region-payload
         CollapseStrategy
         default-collapse-strategy
         merge
         collapse
         Term
         make-atom
         make-pop
         make-merge
         eval-term)

(struct: Region ([label : String]
                 [payload : (Listof Any)]))

(define-type CollapseStrategy (-> Region Region))

(: default-collapse-strategy (-> Region Region))
(define (default-collapse-strategy r) r)

(: merge (-> CollapseStrategy Region Region Region))
(define (merge s r1 r2)
  (define union-payload
    (append (Region-payload r1)
            (Region-payload r2)))
  (collapse s (Region (string-append (Region-label r1) "+" (Region-label r2))
                      union-payload)))

(: collapse (-> CollapseStrategy Region Region))
(define (collapse s r) (s r))

;; Term language

(define-type Term
  (U Atom Pop Mrg))

(struct: Atom ([r : Region]))
(struct: Pop  ([t : Term]))
(struct: Mrg  ([l : Term] [r : Term]))

(: make-atom (-> Region Term))
(define (make-atom r) (Atom r))

(: make-pop (-> Term Term))
(define (make-pop t) (Pop t))

(: make-merge (-> Term Term Term))
(define (make-merge l r) (Mrg l r))

(: eval-term (-> CollapseStrategy Term Region))
(define (eval-term s t)
  (match t
    [(Atom r) r]
    [(Pop sub) (collapse s (eval-term s sub))]
    [(Mrg l r) (merge s (eval-term s l)
                         (eval-term s r))]))
