# A Useful Wrong Answer

## Chapter 1: The Ruler That Ran Long

> **Circle reminder.** A circle holds a piece that belongs together, and circles can sit inside circles. A circle is *ready* when everything needed for its calculation is known and its operation is allowed by the current rules. To **pop** a ready circle, replace it with its value and leave everything outside it alone. To **redraw**, move circles around or split a number, as long as the value stays the same. The full rules are in the introduction.

Mara had a long board and a plan. She wanted a row of pegs for hanging coats, with each peg 7/8 of an inch from the next. The ruler in her toolbox was marked in whole inches, and 7/8 is awkward to find on a ruler like that. So she did the easy thing: she made a pencil mark at every whole inch. Mark 1 at 1 inch, mark 2 at 2 inches, mark 3 at 3 inches, and on down the board.

Then she looked at her plan again and groaned. Every mark was too far along. Her marks were 1 inch apart, and they were supposed to be 7/8 inch apart.

She could have erased everything and started over. Instead she asked a better question: how wrong is each mark, and can I fix them all without measuring again?

### Step 1: Find the mistake

Each step was supposed to be 7/8 inch, and she made it 1 inch. Since 1 is the same as 8/8, each step was too long by one eighth of an inch.

Now look at what that does as the marks pile up.

| Mark | Where it landed | Where it should be | Move it back by |
|---|---|---|---|
| 1st | 1 | 7/8 | 1/8 |
| 2nd | 2 | 1 3/4 | 1/4 |
| 3rd | 3 | 2 5/8 | 3/8 |
| 4th | 4 | 3 1/2 | 1/2 |

Look down the last column. It goes 1/8, 2/8, 3/8, 4/8. Every mark carries one more eighth of mistake than the mark before it. So the fix for mark number n is simple: move it back by n eighths.

*Optional, for brave readers:* you can write the true position of mark n like this: start with n inches, then subtract n/8. That gives 7n/8.

**Try it:** The 8th mark needs eight eighths removed. Eight eighths make exactly one inch, so the 8th mark belongs at 7 inches, not 8. Check it with a real ruler.

### In circles

Mara can write the mistake with circles. Mark 4 landed at ( 4 × 1 ) inches, but it should be at ( 4 × 7/8 ) inches. To find how far back it must move, subtract one circle from the other, then *redraw* so that the 4 sits outside a single circle:

```
( ( 4 × 1 ) − ( 4 × 7/8 ) )
redraw  →  ( 4 × ( 1 − 7/8 ) )
pop ( 1 − 7/8 )  →  1/8
( 4 × 1/8 )
pop  →  1/2
```

The redraw keeps the value, because four lots of 1 minus four lots of 7/8 is four lots of the difference. After the redraw, the whole mistake sits in one small circle, ( 1 − 7/8 ), which pops to 1/8. Every other mark just multiplies that 1/8 by its own number: ( 1 × 1/8 ), ( 2 × 1/8 ), ( 3 × 1/8 ), and so on. That is the last column of the table.

### Step 2: What if you only guessed?

Now change the story. Suppose Mara does not know the exact spacing. She thinks it is about 7/8, and she uses that guess to fix her marks.

Her uncle has made a sample board with the pegs correctly placed. Mara lays the sample beside her board, lines up the zero ends, and measures from that same end. Her guess says the 16th mark belongs at 14 inches (16 times 7/8). But the 16th peg on the sample board is at 13 3/4 inches. Her guess was a quarter inch too long.

If the sample board's pegs are evenly spaced, that quarter inch can be shared out over sixteen steps, about 1/64 of an inch each. One tiny error is invisible. Sixteen of them add up to something you can see.

So the guess was useful, because it got her very close, but it was still a little off, and the mistake grows as you go down the board. If the spacing really is even, the 32nd mark would be about a half inch off. The lesson: **if you are working from an estimate, check a mark far from the start against something you trust. That is where a small error shows up.**

There is another way to see what went wrong with the guess. Mara's guess for mark 16 is a circle she popped correctly: ( 16 × 7/8 ) pops to 14. The popping was fine. The problem was inside the circle. The 7/8 was only an estimate, and it was a little too big. A pop can only be as good as the numbers inside the circle.

### Step 3: The trap

Two friends, Jo and Sam, each mark 16 pegs. Both of them place every mark 1/32 of an inch too far, the same small slip each time.

Jo measures every mark from the zero end of the board. Sam measures each mark from the one before it.

At the 16th mark, how far off is each friend?

Jo is off by 1/32 of an inch, because each of her marks has only its own slip. Sam is off by 16 times 1/32, which is a full half inch, because every slip got carried along to the next mark.

Same slip, very different boards. The only difference is where each friend measured from.

### What did we keep?

Write the answers in your notebook.

- When Mara fixed the marks, **what stayed the same?** (Every mark had the same kind of mistake, just more of it each time. One number fixes them all.)
- **What changed?** (How big the mistake was at each mark.)

### What makes this correction trustworthy?

There are three kinds of fixes in this chapter. Which is which?

1. Moving each mark back by n/8 because the spacing is exactly 7/8.
2. Using a guess of 7/8, then checking the 16th mark against the sample board.
3. Measuring from the zero end instead of from the previous mark.

(Answers: 1 is an exact fix, because the numbers are known. 2 is an estimate that needs a check. 3 is a habit that keeps small errors from piling up.)

### Practice

1. Mara's marks were supposed to be 3/4 inch apart but she placed them 1 inch apart. How far back must mark 6 move? Where does it belong?

2. This time the marks were supposed to be 1 inch apart, but she placed them 3/4 inch apart. They are too short. At which mark is the board a full foot short?

3. Mara guesses that her spacing is 7/8 inch. Her guess says mark 20 belongs at 17 1/2 inches. The sample board has its 20th peg at 17 1/4 inches. Where should mark 40 be? (Hint: if the mistake grows steadily, what happens when the number of marks doubles?)

4. Marks that should be 7/8 inch apart were placed 1 inch apart. Write the correction for mark 12 as a circle, redraw it, and pop it.

### Answers

1. Move mark 6 back by 6/4, which is 1 1/2 inches. It belongs at 4 1/2 inches. Check: 6 times 3/4 is 4 1/2.
2. Each mark is short by 1/4 inch times its number. A foot is 12 inches, and 12 divided by 1/4 is 48. It is the 48th mark.
3. The guess was a quarter inch too long at mark 20. If the sample board's pegs are evenly spaced, it will be a half inch too long at mark 40. The guess says 35 inches, so the mark belongs at 34 1/2 inches.
4. The correction is ( 12 × ( 1 − 7/8 ) ). Pop ( 1 − 7/8 ) to get 1/8, then ( 12 × 1/8 ) pops to **1 1/2**. Move mark 12 back by 1 1/2 inches.

---

*Bridge to The Work That Vanishes:* Mara did not have to remeasure every mark from scratch. She named one small mistake, counted how many times it happened, and fixed everything at once. The multiplication tricks in that book work the same way: take the easy answer, name the leftover piece, and add it back or take it away.
