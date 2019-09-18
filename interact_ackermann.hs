data Nat = S Nat | Z

one = S Z

ackermann :: Nat -> Nat -> Nat
ackermann Z n = S n
ackermann (S m) n = ackermann' n m

ackermann' :: Nat -> Nat -> Nat
ackermann' Z m = ackermann m one
ackermann' (S n) m = ackermann m (ackermann' n m)

nat2int :: Int -> Nat -> Int
nat2int i (S x) = i `seq` nat2int (i + 1) x
nat2int i _ = i

int2nat :: Int -> Nat
int2nat 0 = Z
int2nat x = S (int2nat (x - 1))

main :: IO ()
main = print (nat2int 0 (ackermann (int2nat 3) (int2nat 12)))
