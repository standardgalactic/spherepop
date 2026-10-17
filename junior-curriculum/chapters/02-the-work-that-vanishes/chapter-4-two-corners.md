# The Work That Vanishes

## Chapter 4: Two Corners

Ines was tiling a floor that was 96 tiles long and 94 tiles wide. The numbers were not close to each other's middle the way 48 and 52 are, but both were close to 100.

"Both are a little under 100," said Ines. "Maybe I can use 100 as my friendly number."

### Step 1: Start with the big square

Picture a square 100 tiles by 100 tiles. That is 10,000 tiles. Ines's floor is smaller, 96 by 94. So it is the big square with a strip taken off one side and another strip off the other.

- The strip off the length is 4 wide and 100 long: **4 × 100 = 400** tiles.
- The strip off the width is 6 wide and 100 long: **6 × 100 = 600** tiles.

If Ines takes away both strips, she has removed 10,000 − 400 − 600 = 9,000 tiles' worth.

But wait. The two strips overlap in a small corner, 4 by 6. Ines took that corner away twice. She has to put it back once: 4 × 6 = **24**.

```
( 96 × 94 )
redraw  →  ( ( 100 − 4 ) × ( 100 − 6 ) )
redraw  →  ( ( ( 10,000 − 400 ) − 600 ) + ( 4 × 6 ) )
pop  →  ( ( 9,600 − 600 ) + 24 )
pop  →  ( 9,000 + 24 )
pop  →  9,024
```

The floor takes **9,024** tiles.

### Step 2: A shortcut hiding in the middle

Look at 9,000 again. It is 100 times 90, and 90 is 96 − 6. It is also 94 − 4. Ines found that:

- Take one number and subtract the *other* number's shortfall: 96 − 6 = 90 (or 94 − 4 = 90).
- Multiply by 100: 9,000.
- Add the product of the two shortfalls: 4 × 6 = 24.

```
( ( 96 − 6 ) × 100 ) + ( 4 × 6 )   →   9,000 + 24   →   9,024
```

It works because 100 × 100 − 400 − 600 is just 100 × ( 100 − 4 − 6 ).

### Step 3: Split, Balance, Correct again

Ines used the series strategy in a new way.

- **Split:** 96 as 100 − 4, and 94 as 100 − 6.
- **Balance:** make a big, friendly square, 100 × 100.
- **Correct:** take off the two strips, then put back the corner that was taken off twice.

Before, the correction was a single corner that was missing. Now there are two strips and a corner that was double-counted.

### What did we keep?

Write the answers in your notebook.

- **What did the redraw keep?** (The value, 96 × 94.)
- **What did we need to correct?** (Two strips, and the corner they share.)

### What makes this trustworthy?

The reason is the picture: a big square, two strips, and an overlap that was taken away twice. Every piece of that picture is counted.

The check is another route. Ines found 96 × 94 by taking 96 × 94 = 96 × 100 − 96 × 6 = 9,600 − 576 = 9,024. ✓

### Practice

1. Find 97 × 95 by splitting around 100.

2. Find 98 × 93.

3. Ines forgot to put the corner back for 97 × 95 and wrote 9,200. How far off is she, and why?

4. Find 102 × 104. This time both numbers are over 100. The strips are added, not taken off. What happens to the corner?

5. Find 96 × 94 a third way, using the middle trick: 95 × 95 with a corner.

### Answers

1. Shortfalls are 3 and 5. ( ( 97 − 5 ) × 100 ) + ( 3 × 5 ) = 9,200 + 15 = **9,215**.
2. Shortfalls are 2 and 7. ( ( 98 − 7 ) × 100 ) + ( 2 × 7 ) = 9,100 + 14 = **9,114**.
3. She is off by **15**, which is the corner 3 × 5 that she forgot to put back.
4. Overs are 2 and 4. ( ( 102 + 4 ) × 100 ) + ( 2 × 4 ) = 10,600 + 8 = **10,608**. Here the corner is added because it is a new piece outside the big square.
5. 96 × 94 is 95 × 95 with a 1 by 1 corner missing: 9,025 − 1 = **9,024**. ✓

---

*Bridge to the rest of the series:* In chapter 3, one corner explained the loss from stepping away from a square. Here two strips and a shared corner appear. In A Useful Wrong Answer, the strips are the named errors, and the shared corner is the second-order error that is easy to forget.
