# Reversing the Calculation

## Chapter 2: Socks, Shoes and Buttons That Cannot Be Undone

Omar built a toy calculator out of cardboard. It had only three buttons: **+3**, **double**, and **round**.

He pressed +3 and double and wrote the final answer on a slip of paper. Lena found the slip. It said **16**.

"What number did you start with?" she asked.

"I'll tell you," said Omar, "but first you have to undo it."

### Step 1: Undo in the opposite order

Omar had pressed: start, then **+3**, then **double**, then the display showed 16.

In circles, with the start number as a mystery *s*:

```
( ( s + 3 ) × 2 )  =  16
```

To go backward, Lena undid the last button first.

- The last button was **double**, so she undid it with **halve**: 16 halved is 8.
- Before that was **+3**, so she undid it with **minus 3**: 8 − 3 = 5.

The start was **5**. Check: 5 + 3 = 8, and 8 doubled is 16. ✓

Why did she undo the buttons in the opposite order? Think about socks and shoes. You put on socks first and shoes second. To take them off, you must take off the shoes first. The last thing done is the first thing undone.

### Step 2: Ten doublings

Omar pressed **double** on 1, ten times in a row. Lena wanted to know what appeared on the screen.

```
1 → 2 → 4 → 8 → 16 → 32 → 64 → 128 → 256 → 512 → 1,024
```

Ten doublings of 1 give **1,024**. Then Omar said, "Now I'll hide the 1,024 and show you only a screen that says 1,024. Which number did I start from?"

Lena pressed **halve** ten times. Each press undoes one double. She walked backward:

```
1,024 → 512 → 256 → 128 → 64 → 32 → 16 → 8 → 4 → 2 → 1
```

And she was back at **1**.

Here, double has an undo button that always works. Halve goes back to exactly the number you came from, every time. No information was lost.

### Step 3: The button that cannot be undone

The third button, **round**, rounds a number to the nearest ten, with halves going up. So 23 becomes 20, and 27 becomes 30, and 25 becomes 30.

Omar pressed **round** and the screen said **20**.

"What did I start with?" he asked.

Lena thought. It could have been 20. It could have been 18, or 22, or 15, or 24. All of these round to 20. In fact, any of the ten numbers from 15 to 24 gives 20.

```
( round 15 )  →  20
( round 20 )  →  20
( round 24 )  →  20
```

The button **round** has no undo. Once the display says 20, you cannot go back to the start, because many starts lead to the same 20. That is a pop that forgets something it cannot get back.

This is the difference between the two kinds of buttons.

- **Double** gives a different answer for each different start. Each answer points to exactly one start, so the button can be undone.
- **Round** gives the same answer for many starts. One answer points to many starts, so the button cannot be undone.

### Step 4: Keep the diary

What if Omar wants to be able to go back, even with the round button? Then he should keep a diary of each press, with the numbers before and after.

```
start 23   →  round  →  20
```

With the diary, there is a record of the 23. The calculator forgot, but the diary remembered. This is the same idea as the trace in What Must Remain Unresolved.

### What did we keep?

Write the answers in your notebook.

- **What did the double button keep?** (Everything about the start. It can always be undone.)
- **What did the round button keep, and what did it forget?** (It kept roughly how big the number was. It forgot the exact number.)

### What makes this trustworthy?

An undo is trustworthy if two different starts can never give the same answer. That is a reason, and you can check it by trying to find a pair of starts that collide. For double there is no such pair. For round there are many.

After an undo, always run the undo forward again to check you land on the answer you were given. Lena did: 5 → 8 → 16.

### Practice

1. Omar started at some number, pressed **+5**, then **double**, and got 24. What was the start? Show your steps backward.

2. Omar pressed **double** four times starting from 3. What is on the screen? Then undo it.

3. Here is a rule: the screen shows only whether a number is odd or even. Can this be undone? Why or why not?

4. Which numbers round to 30? (Use the same round rule as in the chapter.)

5. A button called **×0** multiplies by zero. Can it be undone? Explain.

6. Omar presses **+3** then **double** then **+3**. In the screen's final answer, 25 shows. Find the start.

### Answers

1. Undo the double first: 24 halved is 12. Then undo +5: 12 − 5 = **7**. Check: 7 + 5 = 12, and 12 doubled is 24. ✓
2. 3 → 6 → 12 → 24 → 48, so the screen shows **48**. Undoing: 48 → 24 → 12 → 6 → 3.
3. No. Many numbers are odd, and many are even, so the screen cannot tell you which one was the start.
4. The numbers from **25 to 34**: 25 rounds up to 30, and 34 rounds down to 30.
5. No. Any start, multiplied by 0, gives 0. So the 0 on the screen could have come from any number.
6. Undo the last +3: 25 − 3 = 22. Undo the double: 22 ÷ 2 = 11. Undo the first +3: 11 − 3 = **8**. Check: 8 + 3 = 11, double is 22, plus 3 is 25. ✓

---

*Bridge to the rest of the series:* In chapter 1, Lena and Omar saw that many inputs can give one output. Here that idea decides which buttons have an undo. In What Must Remain Unresolved, the trace is the diary that makes round reversible after the fact.
