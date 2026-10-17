# Reversing the Calculation

## Chapter 8: Winding 2,496 Back to 48 and 52

Noor found a card with only an answer on it: **2,496**. Under it, a clue: *"Two numbers, 4 apart, were multiplied to make this."*

Which two numbers? She decided to run the case file backward.

### Step 1: Read the trace from the bottom

Here is the trace of 48 × 52 that Noor already knew:

```
( 48 × 52 )
redraw  →  ( ( 50 − 2 ) × ( 50 + 2 ) )
redraw  →  ( ( 50 × 50 ) − ( 2 × 2 ) )
pop ( 50 × 50 )  →  2,500
pop ( 2 × 2 )  →  4
pop ( 2,500 − 4 )  →  2,496
```

To undo it, she goes up from the bottom, one line at a time, and undoes each line with its opposite. Undoing is done in the **reverse order**, like taking off shoes before socks.

- The last line took 4 away from 2,500. The opposite is to add 4: 2,496 + 4 = **2,500**.
- The line before that made 2,500 from 50 × 50. The opposite is to ask, *what number times itself makes 2,500?* That is **50**.
- The first line split the numbers around 50. The clue says the numbers are 4 apart, so each is 2 from the middle: **48 and 52**.

Noor checks it forward: 48 × 52 = 2,496. ✓

### Step 2: Why the clue was needed

A pop forgets how its value was made. The number 2,496 does not remember that it came from 2,500 − 4, or from 48 × 52. A trace remembers, but the card had no trace.

Without the clue, there are many pairs that multiply to 2,496:

```
26 × 96     39 × 64     48 × 52     24 × 104
```

Every one of these is a correct answer to "what multiplies to 2,496?". The clue *4 apart* picks out just one: 48 and 52 are 4 apart, but 39 and 64 are 25 apart, and the other pairs are even further.

So winding back needed two things: the answer, and a clue about what was thrown away.

### Step 3: Why this clue works

The clue "4 apart" means each number is 2 from the middle. The corner is 2 × 2 = 4, so the product is a square minus 4. That is why adding 4 gives a square, and why the square's root is the middle.

If the clue had been "6 apart", each number would be 3 from the middle, and the corner would be 3 × 3 = 9. Then Noor would add 9, not 4.

### What did we keep?

Write the answers in your notebook.

- **What did the answer 2,496 keep?** (The product, but not the pair that made it.)
- **What did the clue bring back?** (The distance between the numbers, which is enough to find the pair.)

### What makes this trustworthy?

Noor trusts her answer because she ran it forward and got 2,496 again. Undoing and redoing is a check that catches slips. She also knows why the clue was enough: it fixed the corner, and the corner fixed the pair.

### Practice

1. The product is **891** and the two numbers are 6 apart. Find them.

2. The product is **2,499** and the two numbers are 2 apart. Find them.

3. The product is **9,996** and the two numbers are 4 apart. Find them.

4. The product is **2,491** and the two numbers are 6 apart. Find them.

5. Noor says, "2,496 alone tells me the two numbers." Show that she is wrong with two different pairs.

### Answers

1. Each is 3 from the middle, so add 3 × 3 = 9: 891 + 9 = 900. The middle is 30. The numbers are **27 and 33**. (27 × 33 = 891 ✓)
2. Each is 1 from the middle, so add 1: 2,500. The middle is 50. The numbers are **49 and 51**.
3. Add 4: 10,000. The middle is 100. The numbers are **98 and 102**.
4. Add 9: 2,500. The middle is 50. The numbers are **47 and 53**. (47 × 53 = 2,491 ✓)
5. For example, 39 × 64 = 2,496 and 26 × 96 = 2,496. Both match, and neither is 48 × 52.

---

*Bridge to the rest of the series:* In What Must Remain Unresolved, the clue about the distance is what turns an unknown into a number. In A Useful Wrong Answer, the same corner was the debt on an estimate.
