-- spherepop.hs
{-# LANGUAGE DeriveFunctor #-}
{-# LANGUAGE TupleSections #-}
{-# LANGUAGE LambdaCase #-}

-- module Spherepop where

import qualified Data.Map.Strict as M
import Data.List (groupBy, sortOn)
import Data.Function (on)

--------------------------------------------------------------------------------
-- Types
--------------------------------------------------------------------------------

data Type
  = TyUnit
  | TyBool
  | TyNat
  | TyArr Type Type
  | TyTensor Type Type
  deriving (Eq, Ord, Show)

--------------------------------------------------------------------------------
-- Terms
--------------------------------------------------------------------------------

type Name = String

data Term
  = Var Name
  | Sphere Name Type Term
  | Pop Term Term
  | Merge Term Term
  | Choice Double Term Term
  | Rotate Int Term
  | LitUnit
  | LitBool Bool
  | If Term Term Term
  | LitNat Integer
  | Add Term Term
  deriving (Eq, Ord, Show)

--------------------------------------------------------------------------------
-- Values
--------------------------------------------------------------------------------

data Value
  = VUnit
  | VBool Bool
  | VNat Integer
  | VLam Name Term Env
  | VPair Value Value
  deriving (Eq, Ord, Show)

--------------------------------------------------------------------------------
-- Contexts / Environments
--------------------------------------------------------------------------------

type Ctx = M.Map Name Type
type Env = M.Map Name Value

lookupType :: Ctx -> Name -> Either String Type
lookupType g x = maybe (Left $ "Unbound variable: " ++ x) Right (M.lookup x g)

lookupVal :: Env -> Name -> Either String Value
lookupVal e x = maybe (Left $ "Unbound value: " ++ x) Right (M.lookup x e)

--------------------------------------------------------------------------------
-- Type checking
--------------------------------------------------------------------------------

typeOf :: Ctx -> Term -> Either String Type
typeOf g = \case
  Var x -> lookupType g x
  Sphere x a t -> TyArr a <$> typeOf (M.insert x a g) t
  Pop f u -> do
    tf <- typeOf g f
    tu <- typeOf g u
    case tf of
      TyArr a b | a == tu -> pure b
      TyArr a _ -> Left $ "Type mismatch: expected " ++ show a ++ ", got " ++ show tu
      _ -> Left "Attempted to Pop a non-function"
  Merge t u -> TyTensor <$> typeOf g t <*> typeOf g u
  Choice p t u -> do
    if p < 0 || p > 1 then Left "Choice probability must be in [0,1]" else pure ()
    at <- typeOf g t
    au <- typeOf g u
    if at == au then pure at else Left "Choice branches must match"
  Rotate _ t -> do
    tt <- typeOf g t
    if isHomBoolTensor tt then pure tt else Left "Rotate: expected Bool tensor"
  LitUnit -> pure TyUnit
  LitBool _ -> pure TyBool
  If b t u -> do
    tb <- typeOf g b
    if tb /= TyBool then Left "If: condition must be Bool" else pure ()
    tt <- typeOf g t
    tu <- typeOf g u
    if tt == tu then pure tt else Left "If branches must match"
  LitNat _ -> pure TyNat
  Add a b -> do
    ta <- typeOf g a
    tb <- typeOf g b
    if ta == TyNat && tb == TyNat then pure TyNat else Left "Add expects Nat + Nat"

--------------------------------------------------------------------------------
-- Probabilistic distributions
--------------------------------------------------------------------------------

newtype Dist a = Dist { unDist :: [(Double, a)] }
  deriving (Eq, Show, Functor)

delta :: a -> Dist a
delta x = Dist [(1.0, x)]

mix :: Double -> Dist a -> Dist a -> Dist a
mix p (Dist xs) (Dist ys) =
  Dist (map (\(w,a) -> (p*w,a)) xs ++ map (\(w,b) -> ((1-p)*w,b)) ys)

bindD :: Dist a -> (a -> Dist b) -> Dist b
bindD (Dist xs) f =
  Dist [ (w*w', b) | (w,a) <- xs, (w',b) <- unDist (f a) ]

normalize :: (Ord a, Show a) => Dist a -> Dist a
normalize (Dist xs) =
  let ys = map (\(w,a) -> (w, a, show a)) xs
      grouped = groupBy ((==) `on` (\(_,_,s)->s)) (sortOn (\(_,_,s)->s) ys)
      summed  = [ (sum [w | (w,_,_) <- g], a) | g@((_,a,_):_) <- grouped ]
      tot     = sum (map fst summed)
  in if tot == 0 then Dist [] else Dist [ (w/tot, a) | (w,a) <- summed ]

--------------------------------------------------------------------------------
-- Evaluation
--------------------------------------------------------------------------------

type Eval a = Either String (Dist a)

eval :: Env -> Term -> Eval Value
eval env = \case
  Var x -> delta <$> lookupVal env x
  Sphere x _ t -> pure (delta (VLam x t env))
  Pop f u -> do
    df <- eval env f
    du <- eval env u
    pure . normalize $ bindD df (\case
      VLam x body clo -> bindD du (\v -> case eval (M.insert x v clo) body of
                                           Right d -> d
                                           Left err -> Dist [(1.0, VBool False)]) -- dummy error value
      nonFun -> Dist [(1.0, VBool False)])
  Merge t u -> do
    dt <- eval env t
    du <- eval env u
    pure . normalize $ bindD dt (\v -> bindD du (\w -> delta (VPair v w)))
  Choice p t u -> do
    dt <- eval env t
    du <- eval env u
    pure . normalize $ mix p dt du
  Rotate k t -> do
    dt <- eval env t
    pure . normalize $ bindD dt (\v ->
      if allBoolLeaves v
      then case rebuildRight (rotateRight k (flattenPairs v)) of
             Right v' -> delta v'
             Left _   -> v `deltaAs` "rotation failure"
      else v `deltaAs` "not Bool tensor")
  LitUnit -> pure (delta VUnit)
  LitBool b -> pure (delta (VBool b))
  If b t u -> do
    db <- eval env b
    pure . normalize $ bindD db (\case
      VBool True  -> forceEval env t
      VBool False -> forceEval env u
      _           -> delta (VBool False))
  LitNat n -> pure (delta (VNat n))
  Add a b -> do
    da <- eval env a
    db <- eval env b
    pure . normalize $ bindD da (\va ->
      bindD db (\vb -> case (va,vb) of
                         (VNat x, VNat y) -> delta (VNat (x+y))
                         _ -> delta (VNat 0)))

-- Helpers for eval
forceEval :: Env -> Term -> Dist Value
forceEval env t = case eval env t of
  Right d -> d
  Left _  -> Dist []

deltaAs :: Value -> String -> Dist Value
deltaAs v _ = delta v

--------------------------------------------------------------------------------
-- Observables and helpers
--------------------------------------------------------------------------------

anyDoomV :: Value -> Either String Bool
anyDoomV = \case
  VBool b   -> Right b
  VPair v w -> (||) <$> anyDoomV v <*> anyDoomV w
  other     -> Left $ "anyDoom: expected Bool tensor, got " ++ show other

probAnyDoom :: Dist Value -> Either String Double
probAnyDoom (Dist xs) = do
  bs <- mapM (\(w,v) -> (\b -> (w,b)) <$> anyDoomV v) xs
  let good = sum [ w | (w, True) <- bs ]
      tot  = sum [ w | (w, _) <- bs ]
  pure (if tot == 0 then 0 else good / tot)

isHomBoolTensor :: Type -> Bool
isHomBoolTensor ty = let xs = flattenTyTensor ty in not (null xs) && all (== TyBool) xs
  where
    flattenTyTensor = \case
      TyTensor a b -> flattenTyTensor a ++ flattenTyTensor b
      t            -> [t]

flattenPairs :: Value -> [Value]
flattenPairs = \case
  VPair v w -> flattenPairs v ++ flattenPairs w
  v         -> [v]

rebuildRight :: [Value] -> Either String Value
rebuildRight []     = Left "empty"
rebuildRight [v]    = Right v
rebuildRight (v:vs) = VPair v <$> rebuildRight vs

rotateRight :: Int -> [a] -> [a]
rotateRight _ [] = []
rotateRight k xs =
  let n = length xs
      r = ((k `mod` n) + n) `mod` n
  in drop (n-r) xs ++ take (n-r) xs

allBoolLeaves :: Value -> Bool
allBoolLeaves = \case
  VBool _   -> True
  VPair v w -> allBoolLeaves v && allBoolLeaves w
  _         -> False

--------------------------------------------------------------------------------
-- Pretty printing and demos
--------------------------------------------------------------------------------

showDist :: Show a => Dist a -> String
showDist (Dist xs) =
  unlines [ show (fromIntegral (round (1000 * w)) / 10.0 :: Double) ++ "% ⟼ " ++ show a
          | (w,a) <- xs ]

-- Evaluate a closed term to its distribution (or an error)
evalDist :: Term -> Either String (Dist Value)
evalDist = eval M.empty

-- Print distribution + Pr(anyDoom) for a given term expected to be Bool ⊗ ... ⊗ Bool
runAnyDoomOn :: Term -> IO ()
runAnyDoomOn t = do
  putStrLn ("anyDoom over " ++ show t ++ ":")
  case evalDist t of
    Left err -> putStrLn ("  ⟂  " ++ err)
    Right d  -> do
      putStr (showDist d)
      case probAnyDoom d of
        Left e   -> putStrLn ("  ⟂  " ++ e)
        Right pr -> do
          putStrLn ("Pr(anyDoom) = " ++ show pr)


run :: Term -> IO ()
run t = do
  putStrLn "Type:"
  print (typeOf M.empty t)
  putStrLn "\nEval (distribution):"
  case eval M.empty t of
    Left err -> putStrLn ("  ⟂  " ++ err)
    Right d  -> putStrLn (showDist d)


-- Convenience demo
runAnyDoom :: IO ()
runAnyDoom = do
  -- closed-form check for the demo (0.2 and 0.5)
  runAnyDoomOn demoDoomTensor
  putStrLn ("Closed form  = " ++ show (1 - (1 - 0.2) * (1 - 0.5)))

-- Non-IO helper:
probAnyDoomTerm :: Term -> Either String Double
probAnyDoomTerm t = do
  d <- evalDist t
  probAnyDoom d


-- Demo terms
idNat :: Term
idNat = Sphere "x" TyNat (Var "x")

demoId :: Term
demoId = Pop idNat (LitNat 3)

coin :: Double -> Term
coin p = Choice p (LitNat 1) (LitNat 0)

doomCoin :: Double -> Term
doomCoin p = Choice p (LitBool True) (LitBool False)

demoMergeCoins :: Term
demoMergeCoins = Merge (coin 0.3) (coin 0.7)

demoDoomTensor :: Term
demoDoomTensor = Merge (doomCoin 0.2) (doomCoin 0.5)

demoMerge3 :: Term
demoMerge3 = Merge (doomCoin 0.2) (Merge (doomCoin 0.5) (doomCoin 0.7))

-- Run the rotation demo on demoMerge3
runRotate :: IO ()
runRotate = do
  putStrLn "Rotation k=1 over Bool tensor:"
  case evalDist (Rotate 1 demoMerge3) of
    Left err -> putStrLn ("  ⟂  " ++ err)
    Right d  -> putStrLn (showDist d)

