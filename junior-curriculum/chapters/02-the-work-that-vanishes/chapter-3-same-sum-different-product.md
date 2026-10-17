# The Work That Vanishes

## Chapter 3: Same Sum, Different Product

Ines had a rope that was 24 metres long. She wanted to lay it on the ground in a rectangle for a vegetable garden, with the rope going all the way around the edge.

"Every rectangle I make uses the same rope," said Ines. "Does every one hold the same amount of garden?"

### Step 1: Try some rectangles

The rope goes around all four sides, so one long side plus one short side uses half the rope. That is 12 metres. Ines listed the rectangles whose two sides add up to 12:

| Sides | Garden area |
|---|---|
| 6 by 6 | 36 |
| 5 by 7 | 35 |
| 4 by 8 | 32 |
| 3 by 9 | 27 |
| 2 by 10 | 20 |
| 1 by 11 | 11 |

The same rope, but very different gardens. The square holds the most.

### Step 2: The missing-corner trick explains it

Ines noticed that 36 is the biggest, and each other area is 36 minus something:

- 35 = 36 − 1
- 32 = 36 − 4
- 27 = 36 − 9
- 20 = 36 − 16
- 11 = 36 − 25

The things taken away are 1, 4, 9, 16, 25. Those are 1 × 1, 2 × 2, 3 × 3, 4 × 4 and 5 × 5. The missing corner again!

Here is why, with the 4 by 8 rectangle. It is 2 less than 6 on one side and 2 more than 6 on the other:

```
( 4 × 8 )
redraw  →  ( ( 6 − 2 ) × ( 6 + 2 ) )
redraw  →  ( ( 6 × 6 ) − ( 2 × 2 ) )
pop  →  ( 36 − 4 )
pop  →  32
```

Whenever the two sides add up to 12, you can write them as 6 − d and 6 + d for some d. Stepping away from the square always costs a corner of d × d.

### Step 3: What that means for gardeners

The further you step from a square, the more garden you lose. A step of 1 loses 1 square metre, a step of 2 loses 4, a step of 3 loses 9. The loss grows faster and faster.

This is the reason the square is best. It is also why a long thin garden, even with the same rope, holds so little.

Try it with a different rope. If the two sides must add up to 20, the square has sides 10 and 10, and holds 100. A 7 by 13 garden is 3 steps away from the square, so it loses 3 × 3 = 9, and holds 91. Check: 7 × 13 = 91. ✓

### Step 4: A warning

This works when the sides can be any number. If a fence has to use whole metres and the sides must add up to 11, there is no square. Sides 5 and 6 are each half a step from 5.5. The square 5.5 × 5.5 is 30.25, and 5 × 6 is 30, which is 0.25 less. The corner is smaller than 1, but it is still there: it is 0.5 × 0.5 = 0.25.

### What did we keep?

Write the answers in your notebook.

- **What was the same for every rectangle?** (The rope, so the sum of the two sides.)
- **What changed?** (The corner we lose, d × d, so the area.)

### What makes this trustworthy?

The reason is the same as in chapter 1: a rectangle is a square with a corner missing. Here we found it by trying six rectangles, which is evidence. The redraw ( 6 − d ) × ( 6 + d ) = ( 6 × 6 ) − ( d × d ) is the reason, and it works for any d.

### Practice

1. The two sides add up to 30. What is the area of the square? What is the area of a 12 by 18 rectangle? How far is it from the square?

2. Redraw ( 7 × 13 ) as a square with a corner missing and pop every circle.

3. The sides add up to 16. Which rectangle with whole-number sides holds the most?

4. Ines says a 2 by 10 rectangle holds 36 − 16. Where does the 16 come from?

5. Without multiplying 9 × 11 directly, use the sum 20 to say what it is.

### Answers

1. The square is 15 by 15, area **225**. A 12 by 18 is 3 steps from 15, so it loses 3 × 3 = 9, giving 216. Check: 12 × 18 = 216. ✓
2. ( ( 10 − 3 ) × ( 10 + 3 ) ) → ( ( 10 × 10 ) − ( 3 × 3 ) ) → ( 100 − 9 ) → **91**.
3. The square, **8 by 8**, which holds 64.
4. 2 is 4 steps below 6 and 10 is 4 steps above. The corner is 4 × 4 = **16**.
5. The sum 20 gives a square of 10 by 10, which is 100. The corner is 1 × 1 = 1, so 9 × 11 is **99**.

---

*Bridge to the rest of the series:* In chapter 1 we turned a product into a square. In chapter 2 we turned a square back into a product. Here every product with the same sum is compared with one square. In A Useful Wrong Answer, the square is the easy estimate and d × d is the named error.
