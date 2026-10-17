# The Work That Vanishes

## Chapter 7: Climbing from One Square to the Next

Ines knew that 25 × 25 is 625. Her teacher asked her for 26 × 26.

"I do not need to start over," said Ines. "I already have the 25 by 25 square. I just need to build the next one around it."

### Step 1: Add an L-shaped border

A 26 by 26 square is a 25 by 25 square with an L-shaped border added on two sides.

- One side of the border is 25 long.
- The other side is 25 long.
- The corner is 1 square.

So the border has 25 + 25 + 1 = **51** squares. In circles:

```
( 26 × 26 )
redraw  →  ( ( 25 × 25 ) + ( ( 25 + 25 ) + 1 ) )
pop  →  ( 625 + 51 )
pop  →  676
```

26 × 26 is **676**.

Notice the border: 25 + 25 + 1 is also 25 + 26. It is the two squares' sides added together.

### Step 2: The rule for any square

To get from one square to the next, add the old side and the new side.

```
( 41 × 41 )  =  ( 40 × 40 ) + ( 40 + 41 )  =  1,600 + 81  =  1,681
( 51 × 51 )  =  ( 50 × 50 ) + ( 50 + 51 )  =  2,500 + 101 =  2,601
( 100 × 100 ) =  ( 99 × 99 ) + ( 99 + 100 )  so  ( 99 × 99 ) = 10,000 − 199 = 9,801
```

The last line runs the rule backward. If you know 100 × 100, you can step down to 99 × 99 by taking the border away.

Did 1,681 look familiar? It was the number that broke Kofi's prime recipe in The Order of Discovery. That recipe took n × n, added n, then added 41. At n = 40 it was 1,600 + 40 + 41 = 1,681. That is exactly the border from 40 × 40 to 41 × 41, and it makes 41 × 41. A pattern that looked like a prime recipe was secretly a square.

### Step 3: Why the border is always odd

The border from n × n to the next square is n + ( n + 1 ), which is 2n + 1. That is always an odd number. So the squares go up by the odd numbers, 3, 5, 7, 9, 11 and so on, which is the pattern from Kofi's notebook.

```
1,  4,  9,  16,  25,  36
  +3  +5   +7   +9   +11
```

### What did we keep?

Write the answers in your notebook.

- **What stayed when we built the next square?** (The old square, whole, inside the new one.)
- **What did we add?** (An L-shaped border, which is the only new work.)

### What makes this trustworthy?

The reason is the picture: a bigger square is the smaller square plus a border, and every square in the border is counted once. The check is to compare with another way, such as the missing corner: 26 × 26 = ( 25 × 27 ) + 1 = 675 + 1 = 676. ✓

### Practice

1. Find 31 × 31 from 30 × 30.

2. Find 61 × 61 from 60 × 60.

3. Find 20 × 20 from 19 × 19 = 361.

4. Find 49 × 49 from 50 × 50 by stepping down.

5. Check 26 × 26 using the missing corner.

### Answers

1. 900 + ( 30 + 31 ) = 900 + 61 = **961**.
2. 3,600 + ( 60 + 61 ) = 3,600 + 121 = **3,721**.
3. 361 + ( 19 + 20 ) = 361 + 39 = **400**.
4. 2,500 − ( 49 + 50 ) = 2,500 − 99 = **2,401**.
5. ( 25 × 27 ) is 675, and adding the corner 1 × 1 gives **676**. ✓

---

*Bridge to the rest of the series:* In chapter 2, a square was a rectangle plus a corner. Here one square is the next smaller square plus a border. In The Order of Discovery, the border is the reason the odd numbers add up to squares, and it also explains why a prime recipe quietly turned into a square.

*Next in this book:* Chapter 8: The Cross Pieces That Cancel.
