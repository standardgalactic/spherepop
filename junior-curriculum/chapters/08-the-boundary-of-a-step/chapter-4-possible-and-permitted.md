# The Boundary of a Step

## Chapter 4: Possible and Permitted

Sade ran the class shop. A customer bought 3 notebooks at 49 cents each and asked for the exact bill.

Sade's eyes went straight to the shortcut. "49 is nearly 50, and 3 × 50 is easy."

On a calculator she could press that button. It was a **possible** move. But was it **permitted**?

### Step 1: A move can be possible and still not allowed

The customer wanted the exact bill. Rounding 49 up to 50 changes the value, so it is not a fair move for an exact problem. Sade wrote a new kind of line in her notebook, a **refusal**. It records a move she could make, and why she did not.

```
( 3 × 49 )
refuse ( 3 × 50 )  →  not allowed: the bill must be exact
```

Then she used a move that *was* allowed. She split 49 as 50 − 1, and corrected for it:

```
( 3 × 49 )
redraw  →  ( ( 3 × 50 ) − ( 3 × 1 ) )
pop ( 3 × 50 )  →  150
pop ( 3 × 1 )  →  3
( 150 − 3 )
pop  →  147
```

Why is this redraw fine when rounding was not? Because the redraw kept the value. It used 3 × 50, and then took back the 3 × 1 that the 50 had added. Rounding alone would have lost that correction.

The exact bill is **147 cents**.

### Step 2: The same move, a different job

Suppose the customer had said, "Just give me a rough idea." Then ( 3 × 50 ) would have been permitted, because a rough idea only needs to be close. Sade could write:

```
( 3 × 49 )
estimate ( 3 × 50 )  →  150    (rough answer, error 3)
```

The move did not change. The job changed, and the job decides what is permitted.

### Step 3: When a calculator says yes and the problem says no

Twelve cookies are shared among 5 children, and cookies cannot be broken.

A calculator will happily say 12 ÷ 5 = 2.4. That is a possible move with a possible answer. But it is not permitted by the problem, because nobody can have 2.4 whole cookies.

```
( 12 ÷ 5 )
refuse 2.4  →  not allowed: cookies stay whole
each child gets 2, with 2 left over
```

### Step 4: Four kinds of moves

Possible and permitted are two separate questions. Sade made a table:

| Move | Possible now? | Permitted by the problem? |
|---|---|---|
| ( 3 × 49 ) in an exact bill | yes | yes |
| Rounding 49 to 50 in an exact bill | yes | **no** |
| ( x + 12 ) when the jar x is still sealed | **no, not yet** | yes |
| 8 ÷ 0 | no | no |

The third row is the one to notice. A move can be allowed by the rules and still be impossible today, because something is missing. In What Must Remain Unresolved, that is what being stuck means: nothing is forbidden, it just cannot be done yet.

### What did we keep?

Write the answers in your notebook.

- **What did the refusal line keep?** (The reason, so a reader can see the rule that shaped the work.)
- **What did Sade keep by choosing the redraw?** (The exact value of the bill.)

### What makes this trustworthy?

The work is trustworthy when every pop was possible and every move was permitted, and when the notebook says why. If someone asks, "Why didn't you just round?", the refusal line answers. Without it, the reader cannot tell a careful choice from a missed shortcut.

### Practice

1. Sade sells 4 pens at 98 cents each and the bill must be exact. Write the refusal line for ( 4 × 100 ), then find the bill with a redraw.

2. A shopper only wants a rough idea of the cost of the same 4 pens. Is ( 4 × 100 ) permitted now? How far off is it?

3. Seventeen marbles are shared among 4 friends, and marbles stay whole. What does a calculator give for 17 ÷ 4? Write a refusal line, and say what each friend gets and what is left.

4. Fill in "possible now?" and "permitted?" for each: (a) ( 9 + 5 ), (b) ( n + 5 ) with n unknown, (c) rounding 18 to 20 in an exact sum.

5. Write a refusal line for rounding 19 to 20 in the exact bill ( 3 × 19 ), then find the bill another way.

### Answers

1. `refuse ( 4 × 100 ) → not allowed: the bill must be exact`. Then ( ( 4 × 100 ) − ( 4 × 2 ) ) pops to ( 400 − 8 ), which is **392** cents.
2. Yes, because a rough idea only needs to be close. It is 400 against the exact 392, so it is off by **8**.
3. A calculator gives 4.25. `refuse 4.25 → not allowed: marbles stay whole`. Each friend gets **4**, with **1** left over.
4. (a) possible, permitted. (b) not possible yet, permitted. (c) possible, **not** permitted.
5. `refuse ( 3 × 20 ) → not allowed: the bill must be exact`. Then ( ( 3 × 20 ) − ( 3 × 1 ) ) pops to ( 60 − 3 ), which is **57** cents.

---

*Bridge to the rest of the series:* In Solving by Changing the Question, the whole-cookie rule is a refusal: a change of question that ignores it gives an answer that does not fit. In A Useful Wrong Answer, an estimate is permitted exactly when the error is named.
