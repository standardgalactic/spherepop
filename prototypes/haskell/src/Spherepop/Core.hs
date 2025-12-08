module Spherepop.Core
  ( Region(..)
  , Term(..)
  , CollapseStrategy
  , defaultCollapse
  , merge
  , collapse
  , evalTerm
  ) where

import Data.List (foldl')

data Region a = Region
  { regionLabel   :: String
  , regionPayload :: [a]
  } deriving (Eq, Show)

type CollapseStrategy a = Region a -> Region a

defaultCollapse :: CollapseStrategy a
defaultCollapse = id

merge :: CollapseStrategy a -> Region a -> Region a -> Region a
merge strat r1 r2 =
  collapse strat $ Region
    { regionLabel   = regionLabel r1 ++ "+" ++ regionLabel r2
    , regionPayload = regionPayload r1 ++ regionPayload r2
    }

collapse :: CollapseStrategy a -> Region a -> Region a
collapse strat = strat

data Term a
  = Atom (Region a)
  | Pop  (Term a)
  | Merge (Term a) (Term a)
  deriving (Eq, Show)

evalTerm :: CollapseStrategy a -> Term a -> Region a
evalTerm strat t =
  case t of
    Atom r      -> r
    Pop sub     -> collapse strat (evalTerm strat sub)
    Merge l r   -> merge strat (evalTerm strat l) (evalTerm strat r)
