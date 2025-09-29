#lang racket
(provide run run-any-doom run-demos
         demo-id demo-merge-coins demo-doom-tensor demo-merge-3)

(require racket/match)

;; -----------------------------------------------------------------------------
;; Types
;; -----------------------------------------------------------------------------
(struct TyUnit () #:transparent)
(struct TyBool () #:transparent)
(struct TyNat  () #:transparent)
(struct TyArr (dom cod) #:transparent)
(struct TyTensor (left right) #:transparent)

;; -----------------------------------------------------------------------------
;; Terms
;; -----------------------------------------------------------------------------
(define (Var x) `(Var ,x))
(define (Sphere x ty t) `(Sphere ,x ,ty ,t))
(define (Pop f u) `(Pop ,f ,u))
(define (Merge t u) `(Merge ,t ,u))
(define (Choice p t u) `(Choice ,p ,t ,u))
(define (Rotate k t) `(Rotate ,k ,t))
(define (LitUnit) `(LitUnit))
(define (LitBool b) `(LitBool ,b))
(define (If b t u) `(If ,b ,t ,u))
(define (LitNat n) `(LitNat ,n))
(define (Add a b) `(Add ,a ,b))

;; -----------------------------------------------------------------------------
;; Values
;; -----------------------------------------------------------------------------
(struct VUnit   () #:transparent)
(struct VBool   (b) #:transparent)
(struct VNat    (n) #:transparent)
(struct VLam    (x body env) #:transparent)
(struct VPair   (v w) #:transparent)

;; -----------------------------------------------------------------------------
;; Contexts / Environments
;; -----------------------------------------------------------------------------
(define (lookup-type ctx x)
  (hash-ref ctx x (λ () (error (format "Unbound variable: ~a" x)))))

(define (lookup-val env x)
  (hash-ref env x (λ () (error (format "Unbound value: ~a" x)))))

;; -----------------------------------------------------------------------------
;; Type checking
;; -----------------------------------------------------------------------------
(define (type-of ctx term)
  (match term
    [`(Var ,x) (lookup-type ctx x)]
    [`(Sphere ,x ,a ,t)
     (TyArr a (type-of (hash-set ctx x a) t))]
    [`(Pop ,f ,u)
     (define tf (type-of ctx f))
     (define tu (type-of ctx u))
     (match tf
       [(TyArr a b) (if (equal? a tu) b
                        (error (format "Type mismatch: expected ~a, got ~a" a tu)))]
       [_ (error "Attempted to Pop a non-function")])]
    [`(Merge ,t ,u) (TyTensor (type-of ctx t) (type-of ctx u))]
    [`(Choice ,p ,t ,u)
     (unless (<= 0 p 1) (error "Choice probability must be in [0,1]"))
     (define at (type-of ctx t))
     (define au (type-of ctx u))
     (if (equal? at au) at (error "Choice branches must match"))]
    [`(Rotate ,_ ,t)
     (define tt (type-of ctx t))
     (if (is-hom-bool-tensor? tt) tt (error "Rotate: expected Bool tensor"))]
    [`(LitUnit) (TyUnit)]
    [`(LitBool ,_) (TyBool)]
    [`(If ,b ,t ,u)
     (define tb (type-of ctx b))
     (unless (equal? tb (TyBool)) (error "If: condition must be Bool"))
     (define tt (type-of ctx t))
     (define tu (type-of ctx u))
     (if (equal? tt tu) tt (error "If branches must match"))]
    [`(LitNat ,_) (TyNat)]
    [`(Add ,a ,b)
     (define ta (type-of ctx a))
     (define tb (type-of ctx b))
     (if (and (equal? ta (TyNat)) (equal? tb (TyNat))) (TyNat)
         (error "Add expects Nat + Nat"))]))

;; -----------------------------------------------------------------------------
;; Probabilistic distributions
;; -----------------------------------------------------------------------------
(struct Dist (pairs) #:transparent)

(define (delta x) (Dist (list (cons 1.0 x))))

(define (mix p d1 d2)
  (Dist (append (map (λ (wa) (cons (* p (car wa)) (cdr wa))) (Dist-pairs d1))
                (map (λ (wb) (cons (* (- 1 p) (car wb)) (cdr wb))) (Dist-pairs d2)))))

;; FIXED bindD with explicit destructuring
(define (bindD d f)
  (Dist
   (for*/list ([wa (in-list (Dist-pairs d))]
               [w2b (in-list (Dist-pairs (f (cdr wa))))])
     (match-define (cons w a) wa)
     (match-define (cons w2 b) w2b)
     (cons (* w w2) b))))

(define (normalize d)
  ;; group values by their string representation
  (define groups
    (foldl
     (λ (w-a acc)
       (define key (format "~a" (cdr w-a)))
       (hash-update acc key (λ (lst) (cons w-a lst)) '()))
     (hash) (Dist-pairs d)))
  ;; sum the weights for each group
  (define summed
    (for/list ([(k g) (in-hash groups)])
      (cons (for/sum ([w-a (in-list g)]) (car w-a))
            (cdr (car g)))))
  (define tot (for/sum ([w-a (in-list summed)]) (car w-a)))
  (Dist (if (= tot 0) '()
            (map (λ (w-a) (cons (/ (car w-a) tot) (cdr w-a))) summed))))


;; -----------------------------------------------------------------------------
;; Evaluation
;; -----------------------------------------------------------------------------
(define (eval env term)
  (match term
    [`(Var ,x) (delta (lookup-val env x))]
    [`(Sphere ,x ,_ ,t) (delta (VLam x t env))]
    [`(Pop ,f ,u)
     (normalize
      (bindD (eval env f)
             (λ (fval)
               (bindD (eval env u)
                      (λ (uval)
                        (match fval
                          [(VLam x body clo) (eval (hash-set clo x uval) body)]
                          [_ (delta (VBool #f))]))))))]
    [`(Merge ,t ,u)
     (normalize (bindD (eval env t)
                       (λ (v) (bindD (eval env u) (λ (w) (delta (VPair v w)))))))]
    [`(Choice ,p ,t ,u) (normalize (mix p (eval env t) (eval env u)))]
    [`(Rotate ,k ,t)
     (normalize
      (bindD (eval env t)
             (λ (v)
               (if (all-bool-leaves? v)
                   (delta (rebuild-right (rotate-right k (flatten-pairs v))))
                   (delta (VBool #f))))))]
    [`(LitUnit) (delta (VUnit))]
    [`(LitBool ,b) (delta (VBool b))]
    [`(If ,b ,t ,u)
     (normalize
      (bindD (eval env b)
             (λ (v)
               (match v
                 [(VBool #t) (eval env t)]
                 [(VBool #f) (eval env u)]
                 [_ (delta (VBool #f))]))))]
    [`(LitNat ,n) (delta (VNat n))]
    [`(Add ,a ,b)
     (normalize
      (bindD (eval env a)
             (λ (va)
               (bindD (eval env b)
                      (λ (vb)
                        (match* (va vb)
                          [((VNat x) (VNat y)) (delta (VNat (+ x y)))]
                          [(_ _) (delta (VNat 0))]))))))]))

;; -----------------------------------------------------------------------------
;; Helpers
;; -----------------------------------------------------------------------------
(define (is-hom-bool-tensor? ty)
  (define xs (flatten-ty-tensor ty))
  (and (not (null? xs)) (andmap (λ (t) (equal? t (TyBool))) xs)))

(define (flatten-ty-tensor ty)
  (match ty
    [(TyTensor a b) (append (flatten-ty-tensor a) (flatten-ty-tensor b))]
    [t (list t)]))

(define (all-bool-leaves? v)
  (match v
    [(VBool _) #t]
    [(VPair v w) (and (all-bool-leaves? v) (all-bool-leaves? w))]
    [_ #f]))

(define (flatten-pairs v)
  (match v
    [(VPair v w) (append (flatten-pairs v) (flatten-pairs w))]
    [v (list v)]))

(define (rebuild-right vs)
  (match vs
    ['() (error "empty")]
    [(list v) v]
    [(cons v vs) (VPair v (rebuild-right vs))]))

(define (rotate-right k xs)
  (if (null? xs) '()
      (let* ([n (length xs)]
             [r (modulo (+ (modulo k n) n) n)])
        (append (drop xs (- n r)) (take xs (- n r))))))

(define (any-doom-v v)
  (match v
    [(VBool b) b]
    [(VPair v w) (or (any-doom-v v) (any-doom-v w))]
    [_ (error (format "anyDoom: expected Bool tensor, got ~a" v))]))

(define (prob-any-doom d)
  (define bs (map (λ (w-v) (cons (car w-v) (any-doom-v (cdr w-v)))) (Dist-pairs d)))
  (define good (for/sum ([w-b (in-list bs)] #:when (cdr w-b)) (car w-b)))
  (define tot  (for/sum ([w-b (in-list bs)]) (car w-b)))
  (if (= tot 0) 0 (/ good tot)))

;; -----------------------------------------------------------------------------
;; Pretty printing & demos
;; -----------------------------------------------------------------------------
(define (show-dist d)
  (string-join
   (map (λ (w-v)
          (format "~a% ⟼ ~a"
                  (/ (round (* 1000 (car w-v))) 10.0)
                  (cdr w-v)))
        (Dist-pairs d))
   "\n"))

(define (run term)
  (printf "Type:\n~a\n\nEval (distribution):\n" (type-of (hash) term))
  (displayln (show-dist (eval (hash) term))))

(define (run-any-doom term)
  (printf "anyDoom over ~a:\n" term)
  (define d (eval (hash) term))
  (displayln (show-dist d))
  (printf "Pr(anyDoom) = ~a\n" (prob-any-doom d)))

;; Demo terms
(define id-nat (Sphere "x" (TyNat) (Var "x")))
(define demo-id (Pop id-nat (LitNat 3)))
(define (coin p) (Choice p (LitNat 1) (LitNat 0)))
(define (doom-coin p) (Choice p (LitBool #t) (LitBool #f)))
(define demo-merge-coins (Merge (coin 0.3) (coin 0.7)))
(define demo-doom-tensor (Merge (doom-coin 0.2) (doom-coin 0.5)))
(define demo-merge-3 (Merge (doom-coin 0.2) (Merge (doom-coin 0.5) (doom-coin 0.7))))

(define (run-demos)
  (printf "Running demo-doom-tensor:\n")
  (run demo-doom-tensor)
  (printf "\nRunning run-any-doom:\n")
  (run-any-doom demo-doom-tensor)
  (printf "\nRunning rotate demo:\n")
  (run (Rotate 1 demo-merge-3)))
