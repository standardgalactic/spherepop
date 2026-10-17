# What Must Remain Unresolved

## Chapter 3: Notes That Tell the Truth

When Tavi gets stuck, the notebook ends with a note to self. That note matters, because someone else might use it. Tavi, Dani and Sam each wrote a note for the same circle, and the notes did not agree.

The circle was this:

```
( ( x + 3 ) × 4 )
```

The jar x is sealed. Each wrote what they could.

### Step 1: Three notes

**Tavi's note:** `( ( x × 4 ) + 12 )`

**Dani's note:** `( ( x × 4 ) + 3 )`

**Sam's note:** `( x + 12 )`

They cannot all be right. How can you tell which notes are faithful? A faithful note gives the same answer as the original **for every x**.

### Step 2: Test with numbers

You cannot test every x, but you can test a few. Drop a number in for x, then pop every circle.

| x | Original ( ( x + 3 ) × 4 ) | Tavi | Dani | Sam |
|---|---|---|---|---|
| 0 | 12 | 12 | 3 | 12 |
| 1 | 16 | 16 | 7 | 13 |
| 2 | 20 | 20 | 11 | 14 |

Look at each column.

- **Tavi** matches the original every time.
- **Dani** is wrong from the very first row. She shared the 4 with x but forgot to share it with the 3.
- **Sam** matches at x = 0 and then falls behind. If Tavi had tested only x = 0, Sam's note would have looked fine.

### Step 3: What a test shows, and what it doesn't

One lesson: **x = 0 can hide mistakes.** Many wrong notes happen to agree when x is 0. A good test uses several numbers, including some that are not 0 or 1.

A second lesson: **a pass is only evidence**. Tavi's note passed three tests. What makes Tavi's note trustworthy is the reason:

```
( ( x + 3 ) × 4 )
redraw  →  ( ( x × 4 ) + ( 3 × 4 ) )
pop  →  ( ( x × 4 ) + 12 )
```

The redraw shares the 4 with everything inside, so it keeps the value for every x. The tests are a safety net, but the reason is what holds the note up.

### Step 4: The worst note is a false pop

There is one more way to write a bad note. A friend tried to be helpful and popped too much, writing `( 7 + 12 )` → `19`, as if x were 7. She did not know x. That is a pop that used something nobody knew.

A note to self must not claim more than the notebook knows. If x is not known, the note has to keep x. A shorter note is only better if it is still true.

### What did we keep?

Write the answers in your notebook.

- **What does a faithful note keep?** (The value of the original, for every x.)
- **What can a careless note lose or invent?** (Part of the original, like a forgotten 3, or a number nobody knew.)

### What makes this trustworthy?

A note is trustworthy when it has a reason (each redraw keeps the value) and when it passes tests at several different values. A note with only tests might hide a mistake at some value you did not try. A note with only a reason might hide a slip in the arithmetic. Use both.

### Practice

1. Test these notes for ( ( x + 2 ) × 5 ) at x = 0, 1 and 2: (a) `( ( x × 5 ) + 10 )`, (b) `( ( x × 5 ) + 2 )`, (c) `( x + 10 )`.

2. Which notes in problem 1 look fine at x = 0? Which of them fails at x = 1?

3. Write a faithful note for ( 2 × ( x + 6 ) ) and test it at x = 1.

4. Tavi writes ( ( x + 3 ) × 4 ) → ( x + 12 ), saying "4 times 3 is 12". What did Tavi forget?

5. A note says ( x + 12 ) but a friend says x is probably 10, so writes 22. Is 22 a faithful note? Why or why not?

### Answers

1. The original gives 10, 15 and 20. (a) gives 10, 15, 20, so it passes. (b) gives 2, 7, 12, so it fails. (c) gives 10, 11, 12.
2. (a) and (c) both match at x = 0, with 10. (c) fails at x = 1: it gives 11, not 15.
3. `( ( 2 × x ) + 12 )`. At x = 1, the original is 2 × 7 = 14 and the note is 2 + 12 = 14. ✓
4. Tavi multiplied only the 3 by 4 and forgot to multiply the x. The correct note is ( ( x × 4 ) + 12 ).
5. No. 22 only fits x = 10, and nobody knows that. The note must keep x unless x is really known.

---

*Bridge to the rest of the series:* In chapter 2, the note ( ( x + y ) + 22 ) was checked with one pair of jars. Here we see why one check is not enough: a wrong note can pass at x = 0. In Two Boxes, One Answer, the detective's rule says the same about boxes: a finite test cannot prove two things are always equal.
