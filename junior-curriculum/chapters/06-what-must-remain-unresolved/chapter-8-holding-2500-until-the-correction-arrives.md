# What Must Remain Unresolved

## Chapter 8: Holding 2,500 Until the Correction Arrives

Tomas's friend Aiko gave him a puzzle. "I am thinking of two numbers that multiply to something. Their middle is 50. Each is the same distance from 50, one below and one above. I will not tell you how far, but it is somewhere from 1 to 5."

Tomas knew the case file. If the distance is d, the product is a square minus a corner.

### Step 1: The honest note

Tomas started the trace and got stuck.

```
( product )
redraw  →  ( ( 50 × 50 ) − ( d × d ) )
pop ( 50 × 50 )  →  2,500
( 2,500 − ( d × d ) )
```

The circle ( d × d ) is **not ready**, because d is an unknown. Tomas cannot pop it. That is allowed. He keeps the circle as his note to self:

```
( 2,500 − ( d × d ) )
```

This is the most he can say, and it is true. A guess like "2,496" would not be a note. It would be a bet.

### Step 2: What can be said while the note is open?

Tomas can still say useful things. He lists what d could be.

| d | corner ( d × d ) | product |
|---|---|---|
| 1 | 1 | 2,499 |
| 2 | 4 | 2,496 |
| 3 | 9 | 2,491 |
| 4 | 16 | 2,484 |
| 5 | 25 | 2,475 |

So the product is somewhere from **2,475 to 2,499**. He also knows it is **less than 2,500**, because the corner is always more than nothing.

### Step 3: Label each claim

Tomas labels the claims he could make.

- "The product is less than 2,500." **Has a reason.** The corner is at least 1, so something is always taken away.
- "The product is between 2,475 and 2,499." **Has a reason.** It comes from d being 1 to 5.
- "The product is 2,500." **Refuted.** It would need d to be 0, and Aiko said d is at least 1.
- "The product is 2,496." **Only seen so far.** It is true if d is 2, as in 48 × 52, but nothing says d is 2.

### Step 4: The correction arrives

Aiko reveals: "The distance is 2." Now the circle is ready.

```
( 2,500 − ( 2 × 2 ) )
pop ( 2 × 2 )  →  4
pop ( 2,500 − 4 )  →  2,496
```

The note is paid, the number is **2,496**, and it is the same as 48 × 52. The trace shows exactly when the stuck state ended, and why.

### What did we keep?

Write the answers in your notebook.

- **What did Tomas's open note keep?** (Everything true about the problem: the middle, and the shape of the answer.)
- **What did it leave out?** (The one thing nobody had told him, the distance.)

### What makes this trustworthy?

An open note is honest. It claims only what the clues support, and every claim has a label. When the missing piece arrived, Tomas did not have to start again. He finished the same trace.

### Practice

1. The middle is 50 and d is from 1 to 3. What is the range for the product?

2. The middle is 50, and d is even and at most 5. What are the possible products?

3. Label each claim with *has a reason*, *only seen so far*, or *refuted*, when d is unknown but from 1 to 5: (a) "The product is more than 2,400." (b) "The product is 2,491." (c) "The product is more than 2,500."

4. The middle is 30 and d is from 1 to 2. Write the note and find the range.

5. Why is writing "( 2,500 − ( d × d ) )" better than writing "about 2,490"?

### Answers

1. d = 1 gives 2,499, d = 2 gives 2,496, and d = 3 gives 2,491. The range is **2,491 to 2,499**.
2. d = 2 gives **2,496**, and d = 4 gives **2,484**.
3. (a) **Has a reason**: the smallest product is 2,475. (b) **Only seen so far**: it is true when d is 3, but nothing says so. (c) **Refuted**: a corner is always taken away.
4. The note is ( 900 − ( d × d ) ). d = 1 gives 899, and d = 2 gives 896. The range is **896 to 899**.
5. The note says exactly what is known and can be finished later. "About 2,490" is one guess inside that range, and it throws away the structure that says why the product is where it is.

---

*Bridge to the rest of the series:* In A Useful Wrong Answer, the debt on an estimate was known. Here it is not yet known, and the notebook keeps the debt open. In Reversing the Calculation, a clue about the distance is what finds the pair.
