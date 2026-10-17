# The Work That Vanishes

## Chapter 1: The Missing Corner

> **Circle reminder.** A circle holds a piece that belongs together, and circles can sit inside circles. A circle is *ready* when everything needed for its calculation is known and its operation is allowed by the current rules. To **pop** a ready circle, replace it with its value and leave everything outside it alone. To **redraw**, move circles around or split a number, as long as the value stays the same. The full rules are in the introduction.

Ines was making a quilt, and she wanted to know how many squares of cloth she needed. The quilt had 8 rows with 12 squares in each row. Counting every square would take a while, and she did not feel like multiplying 8 by 12 in her head. So she drew it on grid paper instead, and something strange happened.

### Step 1: Cut and slide

Ines drew a rectangle 12 squares long and 8 squares tall. Then she cut off a strip 2 squares wide from the long end. That strip is 2 wide and 8 tall.

She turned the strip on its side, so that it was 8 long and 2 tall, and laid it along the top of what was left.

What she had now was almost a perfect square. The big piece was 10 wide and 8 tall. The strip added a band 2 tall on top, but the band was only 8 long, while the piece underneath was 10 wide. So the band left a small hole in one corner, 2 by 2.

Ines had a 10 by 10 square with a 2 by 2 corner missing.

That is easy to count:

10 times 10 is 100. Take away the missing corner, which is 2 times 2, or 4.

100 minus 4 is 96.

So 8 times 12 is 96. Check it any way you like. It works.

### In circles

Ines's cut-and-slide can be written with circles. The cutting and sliding is a **redraw**: it changes where the circles are, but not the value.

```
( 8 × 12 )
redraw  →  ( ( 10 × 10 ) − ( 2 × 2 ) )
pop ( 10 × 10 )  →  100
pop ( 2 × 2 )  →  4
( 100 − 4 )
pop  →  96
```

All the thinking happens in the redraw. After that, every circle is easy to pop: the big square is one circle, and the missing corner is another.

### Step 2: Does it work for bigger numbers?

Ines wondered if she had found a one-time accident or a real trick. So she tried a bigger quilt.

**18 rows of 22.** The middle number between 18 and 22 is 20. Both numbers are 2 away from it. So Ines pictured a 20 by 20 square with a 2 by 2 corner missing.

20 times 20 is 400. Take away 4.

18 times 22 is 396.

**48 rows of 52.** The middle number between 48 and 52 is 50. Both are 2 away again.

50 times 50 is 2500. Take away 4.

48 times 52 is 2496.

Look at what happened. The numbers got much bigger, but the corner stayed exactly the same size. The part that looked hard, multiplying 48 by 52, turned into a square you can do in your head and a corner you already knew. That is why this chapter is called the work that vanishes: the middle pieces that would have taken so long to add up cancel out before you ever have to count them.

**Predict first:** What would 98 times 102 be? Before you calculate anything, decide how big the corner will be. (It is 2 times 2 again.)

### Step 3: When another route is easier

Now Ines tried 8 rows of 13.

The middle number between 8 and 13 is 10 and a half. That is not a whole number, and her grid paper does not have half squares. Both 8 and 13 are 2 and a half away from the middle, so the corner is no longer a tidy 2 by 2.

So the trick, as she first used it, is not the easiest tool here. She asked what she already knew.

She already knew that 8 times 12 is 96. And 8 rows of 13 is just one more column than 8 rows of 12. One more column is 8 more squares.

96 plus 8 is 104.

So 8 times 13 is 104.

This is a different kind of fix. Instead of finding a perfect square, Ines started from a nearby answer she trusted and added the leftover piece.

*For brave readers:* if you are happy working with halves, the square trick does still work. 10 and a half times 10 and a half is 110 and a quarter. The corner is 2 and a half times 2 and a half, which is 6 and a quarter. 110 and a quarter minus 6 and a quarter is 104. Same answer, so both roads lead to the same place.

### Split, Balance, Correct

This chapter used the same three moves as the series strategy. We **split** the numbers around a friendly middle, **balanced** them into a square, and **corrected** for the corner that the balancing changed.

### What did we keep?

Write the answers in your notebook.

- When Ines cut and slid the strip, **what stayed the same?** (The total number of squares. We only moved them around.)
- **What did the size of the missing corner depend on?** (Only how far the two numbers were from the middle. It did not matter how big the numbers were.)

### What makes this correction trustworthy?

The square-minus-corner trick is an **exact** rule. Every square in the rectangle is still there, just in a new shape, and the corner counts exactly the squares that are missing. There is no guessing in it.

Every pair of numbers has a middle, so the rule can always be used. What changes is how easy it is. It is most convenient when the middle and the corner are easy to work out. For 8 and 13 the middle is 10 and a half, so adding one more column to 8 × 12 is easier.

### Practice

1. Use the trick to find 9 times 11.

2. Use the trick to find 28 times 32.

3. Use the trick to find 98 times 102.

4. Which of these can use the trick with a whole-number middle? For the ones that can, find the answer.
   - 6 times 14
   - 7 times 12
   - 15 times 25

5. Use Ines's other route to find 18 times 23. (Hint: start with 18 times 22, which you already know.)

6. Write **99 × 101** with circles, using 100 as the middle, and pop every circle.

### Answers

1. The middle is 10, and the corner is 1 times 1. 100 minus 1 is **99**.
2. The middle is 30, and the corner is 2 times 2. 900 minus 4 is **896**.
3. The middle is 100, and the corner is 2 times 2. 10,000 minus 4 is **9,996**.
4. 6 times 14: middle is 10, corner is 4 times 4 = 16, so 100 minus 16 is **84**. 7 times 12: the middle would be 9 and a half, so the whole-number trick does not apply. 15 times 25: middle is 20, corner is 5 times 5 = 25, so 400 minus 25 is **375**.
5. 18 times 22 is 396, and one more column of 18 gives 396 plus 18, which is **414**.
6. ( 99 × 101 ) redraws as ( ( 100 × 100 ) − ( 1 × 1 ) ). Pop the two circles to get 10,000 and 1, and then ( 10,000 − 1 ) pops to **9,999**.

---

*Bridge from A Useful Wrong Answer:* In the measuring chapter, Mara named one small mistake and counted how many times it happened. Ines does something similar here. She takes an easy answer, a square, names the leftover piece, a corner, and takes it away. In both stories, nothing is guessed at: the correction can be calculated.
