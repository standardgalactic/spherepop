#lang racket

;; Core untyped Spherepop implementation (small and readable).
;; A region is represented abstractly as a struct with a label and
;; an arbitrary payload. Merge and collapse are parameterized by a
;; collapse-strategy function.

(provide region?
         make-region
         region-label
         region-payload
         collapse-strategy?
         default-collapse-strategy
         merge
         collapse
         eval-term
         term?
         make-atom
         make-pop
         make-merge)

(struct region (label payload) #:transparent)

(define (make-region label payload)
  (region label payload))

(define (region-label r) (region-label r))
(define (region-payload r) (region-payload r))

;; A collapse-strategy maps a region to a simplified region.
(define collapse-strategy? (-> region? region?))

(define (default-collapse-strategy r)
  ;; For now, simply normalise the payload into a list.
  (match r
    [(region lbl p)
     (region lbl (if (list? p) p (list p)))]))

;; Merge is union followed by collapse under a strategy.
(define (merge s r1 r2)
  (define union-region
    (match (list r1 r2)
      [(list (region l1 p1) (region l2 p2))
       (region (string-append l1 "+" l2)
               (append (if (list? p1) p1 (list p1))
                       (if (list? p2) p2 (list p2))))]))
  (collapse s union-region))

(define (collapse s r)
  (s r))

;; ------------------------------------------------------------------
;; Tiny term language for experimentation.

(struct atom (region) #:transparent)
(struct pop (sub) #:transparent)
(struct mrg (left right) #:transparent)

(define (make-atom r) (atom r))
(define (make-pop t) (pop t))
(define (make-merge t1 t2) (mrg t1 t2))

(define (term? t)
  (or (atom? t) (pop? t) (mrg? t)))

(define (eval-term s t)
  (match t
    [(atom r) r]
    [(pop sub)
     (collapse s (eval-term s sub))]
    [(mrg l r)
     (merge s (eval-term s l)
              (eval-term s r))]))
