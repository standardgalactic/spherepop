module Spherepop.Parallel
  ( stepParallel
  ) where

import Spherepop.Core

-- A very small placeholder for parallel-style evaluation.
-- In a fuller implementation, this would detect independent
-- subterms and reduce them in parallel. For now, we simply
-- perform one evaluation step.

stepParallel :: CollapseStrategy a -> Term a -> Term a
stepParallel strat t =
  case t of
    Atom _    -> t
    Pop sub   -> case sub of
                   Atom r  -> Atom (collapse strat r)
                   _       -> Pop (stepParallel strat sub)
    Merge l r -> case (l, r) of
                   (Atom rl, Atom rr) -> Atom (merge strat rl rr)
                   _                  -> Merge (stepParallel strat l)
                                              (stepParallel strat r)
