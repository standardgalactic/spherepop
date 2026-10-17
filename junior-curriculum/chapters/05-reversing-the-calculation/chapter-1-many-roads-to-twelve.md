# Reversing the Calculation

## Chapter 1: Many Roads to Twelve

> **Circle reminder.** A circle holds a piece that belongs together, and circles can sit inside circles. A circle is *ready* when everything needed for its calculation is known and its operation is allowed by the current rules. To **pop** a ready circle, replace it with its value and leave everything outside it alone. To **redraw**, move circles around or split a number, as long as the value stays the same. The full rules are in the introduction.

Lena's friend Omar left a calculator on her desk with a single number glowing on the screen: **12**. Next to it was a note: *"Guess what I typed."*

Lena stared at the screen. She had no idea where to start.

### Step 1: How many roads lead to 12?

The screen only showed the answer. It did not show what Omar had pressed. So Lena made a list of everything he could have done.

**Adding two positive whole numbers to reach 12** (skipping repeats like 5 + 7 and 7 + 5):

1 + 11, 2 + 10, 3 + 9, 4 + 8, 5 + 7, 6 + 6

That is 6 ways already.

**Multiplying two positive whole numbers to reach 12:**

1 × 12, 2 × 6, 3 × 4

That is 3 more ways.

And there are plenty of other roads: 20 minus 8, 24 divided by 2, 100 minus 88. Lena could keep going for a long time.

The calculator did the work and then forgot how. When a calculation squeezes many different inputs down to one answer, the answer alone cannot tell you which input it came from. Mathematicians say the calculation *throws away information*.

### In circles

Each road to 12 is a circle that pops to the same number:

```
( 5 + 7 )   →  12
( 3 × 4 )   →  12
( 2 × 6 )   →  12
( 20 − 8 )  →  12
```

Omar's calculator showed only the 12. The pop was finished, and the circle that made it was gone. A pop forgets which circle it came from.

### Step 2: Keeping one clue

Omar had typed exactly one operation on two numbers. He felt a little bad, so he gave her a clue: *"The first number I typed was 5, and I added."*

Now Lena had something to work with. If 5 plus something equals 12, she can undo the adding by taking away:

12 minus 5 is 7.

So Omar typed 5 plus 7. One clue was enough to get all the way back.

For the next round, Omar typed a multiplication. The screen showed **12**, and his clue was: *"The first number was 3."*

Lena undid the multiplying by dividing:

12 divided by 3 is 4.

So he had typed 3 times 4. Again, one clue was enough.

### In circles

Omar's clue turns the mystery into a circle with a hole in it:

```
( 5 + ? )  →  12
```

To find the hole, redraw the circle the other way round:

```
( 12 − 5 )  →  7
```

For the multiplying round, ( 3 × ? ) pops to 12, and ( 12 ÷ 3 ) pops to 4.

### Step 3: The clue that doesn't work

Omar tried a third round, this time allowing himself to use zero. The screen showed **0**, and his clue was: *"The first number was 0, and I multiplied."*

Lena thought about it. 0 times 1 is 0. 0 times 7 is 0. 0 times 1,000 is 0.

Every possible second number gives the same answer. Even with the clue, she could not tell what Omar had typed.

Multiplying by zero is a calculation that throws away *everything* about the second number. No clue about the first number can help, because the damage is done by the zero.

### Step 4: Three ways to remember

There are three ways a machine could keep track of what it did. Each one costs something.

1. **Keep only the answer.** It takes almost no room to store, but you cannot go back. (This is Omar's calculator.)
2. **Keep the answer plus one clue.** It costs a little more room, and sometimes that is enough to go back, as with the 5 and the 3. But not always, as with the zero.
3. **Keep a whole diary.** Write down "start with 5, add 7, got 12." It costs the most room, and it lets you go back every time.

A calculator that has to be tiny and quick keeps only the answer. A bank, which must be able to trace every step, keeps the diary. Neither is wrong. They are trading room against going back. (A diary like this is a trace: one line for each pop.)

### Machines you can undo

Some machines need no clue at all.

A machine that **adds 7** to whatever you put in can always be undone: take the answer and subtract 7. Different inputs always give different answers, so no two inputs get squished together.

A machine that **rounds to the nearest 10** cannot be undone. Put in 8 and you get 10. Put in 12 and you also get 10. If you see 10 on the screen, you cannot tell which one it was.

The test is simple: **can two different inputs ever give the same output?** If they can, the machine throws information away. If they can't, then any answer the machine really produced has exactly one possible input. Finding that input is not always easy, but you know it is the only one.

### What did we keep?

Write the answers in your notebook.

- **What did the calculator keep?** (The answer.)
- **What did it lose?** (How the answer was made.)
- **What did a clue give back?** (Enough to rebuild the calculation, as long as the clue was the right kind.)

### What makes this conclusion trustworthy?

Imagine Lena guessed "Omar typed 3 times 4" before she had any clue. Would that be trustworthy?

No. It would be a guess, and 2 times 6 would have been just as good. Her answer only became trustworthy once she had a clue that ruled out every other road.

(That is the same lesson as the detective in Two Boxes, One Answer: looking at the outside of a machine cannot tell you the inside.)

### Practice

1. List every way to make 18 by multiplying two positive whole numbers. Then list every way to make 10 by adding two positive whole numbers, skipping repeats.

2. A machine adds 9 to your number. The screen shows 20. What did you put in? Can this machine always be undone?

3. A machine rounds your number to the nearest 10, with halves rounding up (so 25 becomes 30). The screen shows 30. Which whole numbers could you have put in?

4. A machine multiplies your number by 4. The screen shows 36. What did you put in? Then try the same machine with "multiply by 0." The screen shows 0. Can you tell what went in?

5. A friend says: "Keeping a whole diary is always better than keeping only the answer." Can you think of a time when the diary would be a bad idea? What does it cost?

6. The circle ( ? + 8 ) pops to 20. Redraw it the other way round to find the hole.

### Answers

1. Multiplying to 18: **1 × 18, 2 × 9, 3 × 6** (three ways). Adding to 10: **1 + 9, 2 + 8, 3 + 7, 4 + 6, 5 + 5** (five ways).
2. 20 minus 9 is **11**. Yes: different inputs always give different answers, so you can always subtract 9 to go back.
3. Any whole number from **25 to 34**. That is ten different inputs, so the machine cannot be undone.
4. 36 divided by 4 is **9**. Multiplying by 4 can always be undone. But "multiply by 0" always gives 0, so you cannot tell what went in.
5. The diary costs room, and it can cost speed too. A tiny machine, or one that does millions of steps, might not have space to write down every one. Also, if the steps are private, a diary lets other people read them. Whether to keep the diary depends on how much you need to go back, and how much room you can spare.
6. ( 20 − 8 ) pops to **12**, so the hole is 12.

---

*Bridge to the rest of the series:* In Two Boxes, One Answer, the detective could only see what came out of the slot. Here, the answer is the output, and the detective is trying to work out the input. In A Useful Wrong Answer, Mara's marks were 1 inch apart when they should have been 7/8 inch, and she could still find the right positions because she knew the size of the mistake. In The Order of Discovery, the messy notebook is the diary, and the tidy proof is a short trace that keeps only the steps that matter.
