# The Work That Vanishes

## Chapter 2: Putting the Corner Back

In chapter 1, Ines turned a product into a square with a corner missing. Now she wants to go the other way. She is making a quilt with 35 rows of 35 squares, and she wants to know how many squares that is.

35 × 35 is a square, not a rectangle. Can the missing-corner trick help?

### Step 1: Run the trick backward

In chapter 1, the rectangle 8 × 12 was a 10 by 10 square with a 2 by 2 corner missing. That means:

*square = rectangle + corner.*

Ines turned that around. A 35 by 35 square is a rectangle plus a corner. Which rectangle? Take 5 off one side and add 5 to the other: 30 × 40. And the corner is 5 × 5.

Here is her notebook:

```
( 35 × 35 )
redraw  →  ( ( 30 × 40 ) + ( 5 × 5 ) )
pop ( 30 × 40 )  →  1,200
pop ( 5 × 5 )  →  25
( 1,200 + 25 )
pop  →  1,225
```

So 35 × 35 is **1,225**. The hard part, 35 × 35, became an easy rectangle and a small corner.

### Step 2: Choose the corner you like

Ines can pick how far to step away from 35. She stepped 5 each way because 30 × 40 is easy. Other squares want other steps.

**49 × 49.** Step 1 each way: 48 × 50 is easy, and the corner is 1 × 1.

```
( 49 × 49 )
redraw  →  ( ( 48 × 50 ) + ( 1 × 1 ) )
pop ( 48 × 50 )  →  2,400
pop ( 1 × 1 )  →  1
( 2,400 + 1 )
pop  →  2,401
```

**98 × 98.** Step 2 each way: 96 × 100 is easy, and the corner is 2 × 2. That gives 9,600 + 4 = **9,604**.

The rule is always the same. For any number, step away by some amount *d* in both directions, multiply the two ends, and put back the corner *d* × *d*.

### Step 3: A pattern, and the reason for it

Ines tried the squares of numbers ending in 5, stepping 5 each way:

- 15 × 15: 10 × 20 + 25 = 225
- 25 × 25: 20 × 30 + 25 = 625
- 35 × 35: 30 × 40 + 25 = 1,225
- 45 × 45: 40 × 50 + 25 = 2,025
- 55 × 55: 50 × 60 + 25 = 3,025

She noticed something. Every answer ends in 25. And the front part is a product of two neighbors: 1 × 2, 2 × 3, 3 × 4, 4 × 5, 5 × 6, followed by 25.

That is a pattern from five examples, which is evidence. Here is the reason. 30 × 40 is 100 times 3 × 4, and 40 × 50 is 100 times 4 × 5. The rectangle always has a tens digit times the next one, times 100, and the corner is always 5 × 5 = 25.

### Step 4: Do not forget the corner

Dani tried 35 × 35 and said, "30 times 40 is 1,200, so that is the answer."

It is not. 1,200 is a useful wrong answer, and the error is exactly the corner that Dani left out. The answer is 25 more than Dani's.

The corner is the part of the work that does not disappear. The other parts, the long sums, are what the trick saves you from.

### Split, Balance, Correct

This chapter used the same three moves as the series strategy. We **split** the numbers around a friendly middle, **balanced** them into a square, and **corrected** for the corner that the balancing changed.

### What did we keep?

Write the answers in your notebook.

- **What stayed the same when Ines redrew the circles?** (The value of the square. It was 35 × 35 before and after.)
- **What changed?** (The pops. A hard multiplication became two easy ones and an addition.)

### What makes this trustworthy?

The rule is exact: a square is always the rectangle plus the corner, because the rectangle plus the corner puts every missing square back. You can test it on a small square you know. Try 6 × 6 with a step of 2: 4 × 8 + 2 × 2 = 32 + 4 = 36. It matches.

The pattern for numbers ending in 5 is a second kind of statement. Five examples made Ines suspect it, and the rectangle-plus-corner reason is what makes her sure.

### Practice

1. Find **45 × 45** with a step of 5.

2. Find **99 × 99** with a step of 1.

3. Find **75 × 75** with a step of 5.

4. Find **52 × 52**. Which step would you choose, and why?

5. Dani says "35 × 35 is 30 × 40, which is 1,200." What is missing, and how much is it?

### Answers

1. ( ( 40 × 50 ) + ( 5 × 5 ) ) pops to 2,000 + 25 = **2,025**.
2. ( ( 98 × 100 ) + ( 1 × 1 ) ) pops to 9,800 + 1 = **9,801**.
3. ( ( 70 × 80 ) + ( 5 × 5 ) ) pops to 5,600 + 25 = **5,625**.
4. A step of 2 gives ( ( 50 × 54 ) + ( 2 × 2 ) ) = 2,700 + 4 = **2,704**. A step of 2 makes one end 50, which is easy to multiply by.
5. The corner is missing. It is 5 × 5 = **25**, so the answer should be 1,225.

---

*Bridge to the rest of the series:* In chapter 1 the corner was taken away to turn a rectangle into a square. Here it is put back. In A Useful Wrong Answer, Dani's 1,200 is an estimate with a named error, and the error is the corner.
