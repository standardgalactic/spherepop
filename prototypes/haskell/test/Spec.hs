module Main (main) where

import Test.Hspec
import Spherepop.Core

main :: IO ()
main = hspec $ do
  describe "Spherepop.Core" $ do
    it "merges regions" $ do
      let r1 = Region "a" [1 :: Int]
          r2 = Region "b" [2 :: Int]
          out = merge defaultCollapse r1 r2
      regionLabel out `shouldBe` "a+b"
