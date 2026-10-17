# The Boundary of a Step

## Chapter 8: The Step You Never Take

Kit had two helpers, and a card with a very strange instruction: **write down the first number and throw away the second.**

The card said:

```
( keep-first  7  ( 987 × 654 ) )
```

A **keep-first** circle has two slots. It keeps what is in the first slot and throws away what is in the second. Here the first slot is 7, and the second holds a big multiplication.

### Step 1: The eager helper

The eager helper follows one rule: *finish everything inside before touching the outside.*

```
pop ( 987 × 654 )  →  645,498
pop ( keep-first 7 645,498 )  →  7
```

Two pops, one of them a large multiplication, and the answer is 7. All the work on the 645,498 was thrown away at once.

### Step 2: The lazy helper

The lazy helper looks at the keep-first circle first. The rule of a keep-first circle is that the second slot is thrown away, so it never needs to be opened.

```
pop ( keep-first 7 ( 987 × 654 ) )  →  7      second slot never opened
```

One pop, the same answer, and no multiplication at all.

By the ready rule, the keep-first circle is ready, because the calculation it needs is only the first slot. The second slot is not needed.

Notice that the lazy helper did not skip a step carelessly. It looked at the circle, saw that the second slot could not change the result, and left it alone. The trace says so: *second slot never opened*.

### Step 3: A slot that never finishes

Kit then tried a worse card.

```
( keep-first  4  ( count every whole number ) )
```

The second slot is an endless job. Counting every whole number never finishes. The eager helper starts on it first and never gets to the outside. It counts forever.

The lazy helper pops the outside circle at once and answers **4**.

Both helpers were doing legal moves. The only difference was the order, and one order arrived while the other never did. The answer, 4, was there all along.

### Step 4: Lazy is not always better

Kit tried a card where the second slot *is* used.

```
( 5 + ( 987 × 654 ) )
```

Here the multiplication is part of the sum, so it cannot be skipped. Both helpers must finish it. The lazy helper is only quicker when a piece will be thrown away.

Kit also noticed something about the case file. In 48 × 52, nothing is thrown away. The correction of 4 is needed. But if someone only asked, "is 48 × 52 more than 2,000?", the lazy answer would not take the correction step at all. 2,500 minus any small corner is clearly still above 2,000, and that is the step you never take.

### What did we keep?

Write the answers in your notebook.

- **What did the lazy helper keep?** (The answer, 7 or 4, which was the same as the eager helper's.)
- **What did it avoid?** (Work on a piece that would be thrown away.)

### What makes this trustworthy?

Skipping work is trustworthy when you can give the reason. Here the reason is the rule of the circle: the second slot cannot change the result. If you cannot give a reason, do not skip. Kit marks a skip as **has a reason** only when it comes with the rule that justifies it.

### Practice

1. Find ( keep-first 12 ( 345 × 678 ) ) with the lazy helper. How many pops? How many for the eager helper?

2. Find ( ( keep-first 9 ( 77 × 88 ) ) + 1 ) with the lazy helper. How many pops?

3. Can the lazy helper skip the multiplication in ( 3 + ( 12 × 12 ) )? Why or why not?

4. Which helper answers ( keep-first 8 ( count every whole number ) )? What is the answer?

5. Someone says, "The lazy helper is less careful." What do you say?

### Answers

1. Lazy: **1** pop, answer 12. Eager: **2** pops (the multiplication, then the keep-first), answer 12.
2. Pop the keep-first to get 9, then pop ( 9 + 1 ). That is **2** pops, and the answer is **10**.
3. **No.** The 12 × 12 is added in, so it affects the result. It must be finished.
4. The **lazy** helper. The eager helper never finishes the count. The answer is **8**.
5. The lazy helper is careful about one thing: it only skips a piece when the rule says the piece cannot matter. Skipping with a reason is not careless.

---

*Bridge to the rest of the series:* In Two Boxes, One Answer, a note on the fridge saves work by reusing a value, and here a thrown-away slot saves work by never computing it. In The Order of Discovery, two routes that both finish always agree, but one route may never finish.
