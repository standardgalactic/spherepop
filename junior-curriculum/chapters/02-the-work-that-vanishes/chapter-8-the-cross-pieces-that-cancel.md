# The Work That Vanishes

## Chapter 8: The Cross Pieces That Cancel

Ines had used the missing-corner trick for **48 × 52** many times. Today she wanted to know *why* the work vanishes. So she took the problem apart, all the way down, and looked at every piece.

### Step 1: Break both numbers

Ines wrote 48 as 50 − 2 and 52 as 50 + 2. Each number now has two parts, and multiplying them means every part of the first meets every part of the second. That gives four small products.

```
( ( 50 − 2 ) × ( 50 + 2 ) )
redraw  →  ( ( 50 × 50 ) + ( 50 × 2 ) − ( 2 × 50 ) − ( 2 × 2 ) )
```

This redraw keeps the value, because it only shares out the multiplication across the parts. Now every circle is easy to pop:

```
pop ( 50 × 50 )  →  2,500
pop ( 50 × 2 )   →  100
pop ( 2 × 50 )   →  100
pop ( 2 × 2 )    →  4
( 2,500 + 100 − 100 − 4 )
```

### Step 2: Watch the cross pieces

Look at the middle two. One is **+100** and the other is **−100**. They are the same size and opposite, so together they add nothing.

```
( 2,500 + 100 − 100 − 4 )
redraw  →  ( 2,500 − 4 )      the +100 and −100 cancel
pop  →  2,496
```

That is the work that vanishes. The two cross pieces were the hard-looking part of multiplying 48 by 52, and they cancelled each other before Ines ever had to deal with them.

### Step 3: When the cross pieces do not cancel

Ines wondered if the cancelling always happens. She tried **48 × 53**, where the two numbers are *not* the same distance from 50. 48 is 2 below 50, and 53 is 3 above.

```
( ( 50 − 2 ) × ( 50 + 3 ) )
redraw  →  ( ( 50 × 50 ) + ( 50 × 3 ) − ( 2 × 50 ) − ( 2 × 3 ) )
```

The pieces are 2,500, **+150**, **−100** and −6. The cross pieces are 150 and 100. They are different sizes, so they do not cancel:

```
( 2,500 + 150 − 100 − 6 )  →  2,544
```

Check it directly: 48 × 53 = 2,544. ✓

So the cancelling is not magic. It happens only when the two numbers are the **same distance** from the middle, one below and one above. Then the two cross pieces are the same size, one added and one taken away.

### What did we keep?

Write the answers in your notebook.

- **What did breaking the numbers apart keep?** (The value, since the redraw only shared out the multiplication.)
- **What made two pieces cancel?** (The same distance from the middle, one below and one above.)

### What makes this trustworthy?

Ines can explain the trick from the four pieces, so it is not a pattern from a few examples. She can also say exactly when it stops working, which is when the distances differ. A rule that tells you where it stops is a rule you can trust.

### Practice

1. Break **29 × 31** into four pieces around 30. Which two pieces cancel? What is the answer?

2. Break **47 × 53** into four pieces around 50. Find the answer.

3. Use four pieces to find **98 × 102**.

4. Try **48 × 54** around 50, with distances 2 and 4. Do the cross pieces cancel? Find the answer and check it.

5. Ines says the corner is 2 × 2 because the numbers are 2 away. For 47 × 53, how far away are they? What is the corner?

### Answers

1. ( 30 × 30 ) + ( 30 × 1 ) − ( 1 × 30 ) − ( 1 × 1 ) = 900 + 30 − 30 − 1. The two 30s cancel. The answer is **899**.
2. 2,500 + 150 − 150 − 9. The cross pieces cancel. The answer is 2,500 − 9 = **2,491**.
3. 10,000 + 200 − 200 − 4, so **9,996**.
4. The pieces are 2,500, +200, −100 and −8. They do not cancel. 2,500 + 200 − 100 − 8 = **2,592**. Check: 48 × 54 = 2,592. ✓
5. They are 3 away, so the corner is 3 × 3 = **9**.

---

*Bridge to the rest of the series:* In A Useful Wrong Answer, the corner is the debt on an estimate. In Solving by Changing the Question, sliding two numbers in opposite directions by the same amount keeps their sum and changes the product by exactly the corner.
