data Type = TyUnit | TyTensor Type Type deriving Show
main = print (TyTensor TyUnit TyUnit)
