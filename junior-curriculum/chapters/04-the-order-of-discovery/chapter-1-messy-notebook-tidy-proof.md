# The Order of Discovery

## Chapter 1: Messy Notebook, Tidy Proof

> **Circle reminder.** A circle holds a piece that belongs together, and circles can sit inside circles. A circle is *ready* when everything needed for its calculation is known and its operation is allowed by the current rules. To **pop** a ready circle, replace it with its value and leave everything outside it alone. To **redraw**, move circles around or split a number, as long as the value stays the same. The full rules are in the introduction.

Kofi had a puzzle from his teacher:

**Find two numbers that add up to 20 and multiply to 96.**

He sat down with a pencil and a fresh page, and he did what mathematicians actually do. He made a mess.

### Step 1: Kofi's notebook

Here is his page, exactly as he wrote it, crossed-out parts included.

- Try 10 and 10. They add to 20. Multiply: 100. **Too big.**
- Try 5 and 15. Add to 20. Multiply: 75. **Too small. Jumped too far.**
- ~~Try 6 and 16. Multiply: 96!!~~ Wait. 6 plus 16 is 22, not 20. **Broken rule. Cross it out.** (It did multiply to 96, but the sum is wrong, so it does not count.)
- Try 9 and 11. Add to 20. Multiply: 99. **Close!**
- Try 8 and 12. Add to 20. Multiply: 96. **Yes!**

Kofi had his answer: 8 and 12. He also had a page full of guesses, a crossed-out mistake, and a feeling that he had been wandering.

But look again at the guesses that kept the sum at 20. Kofi made a small list:

| Pair | Product | Missing from 100 |
|---|---|---|
| 10 and 10 | 100 | 0 |
| 9 and 11 | 99 | 1 |
| 8 and 12 | 96 | 4 |
| 7 and 13 | 91 | 9 |

The last column goes 0, 1, 4, 9. Those are square numbers. Every step away from 10 takes away a square.

Kofi had not been wandering at all. His guesses were quietly teaching him a pattern.

### In circles

Each of Kofi's guesses is a circle he draws and pops, and then compares with the target:

```
( 10 × 10 )  →  100    too big
( 9 × 11 )   →  99     closer
( 8 × 12 )   →  96     yes
( 8 + 12 )   →  20     the sum matches
```

His messy notebook is a long trace of pops, including the ones he crossed out.

### Step 2: The tidy proof

Kofi wanted a shorter version that made the reasoning easy to follow. So he wrote a second version:

> Let *d* be the distance from 10, so *d* is 0 or more. Let the two numbers be 10 minus *d* and 10 plus *d*. They add up to 20.
>
> Their product is 100 minus *d* times *d*.
>
> We want the product to be 96, so *d* times *d* is 4, which means *d* is 2.
>
> The numbers are 8 and 12. Check: 8 + 12 = 20 and 8 × 12 = 96.

That is the whole proof. It has no guesses and no crossed-out parts, every line follows from the one before it, and the last line checks the answer in the original puzzle.

### In circles

The tidy proof is a trace too, but a much shorter one. It starts with a redraw:

```
( ( 10 − d ) × ( 10 + d ) )
redraw  →  ( ( 10 × 10 ) − ( d × d ) )
```

That is the missing corner from The Work That Vanishes: a 10 by 10 square with a *d* by *d* corner taken away. The tidy trace keeps only the redraw and the pops that matter. The messy notebook keeps every pop, wrong turns included.

### Step 3: Why have two versions?

The two pages do different jobs.

The **messy notebook** is where the idea was *found*. Without the guesses, Kofi would not have noticed the squares. The wrong turns were not wasted. They were how he learned what to look for.

The **tidy proof** is where the idea is *checked*. Anyone can read it, line by line, and decide for themselves whether it is true. They do not need to follow Kofi's wandering.

Each version leaves something out. The tidy proof hides how Kofi found the answer, which can make it look like magic: "How did he know to write 10 minus *d*?" The messy notebook hides whether the answer is correct, because you have to read every attempt to be sure nothing went wrong.

A good mathematician keeps both. The notebook is for finding. The proof is for sharing. (A teacher is often glad to see the notebook too, because it shows how you think.)

### What did we keep?

Write the answers in your notebook.

- **What did the tidy proof keep from the messy page?** (The answer, and a reason why it works.)
- **What did it throw away?** (The wrong guesses, the crossed-out mistake, and the order in which things were tried.)
- **Was anything thrown away that mattered?** (Yes. The guesses showed Kofi the pattern of squares. The proof does not say where the idea of 10 minus *d* came from.)

### What makes this answer trustworthy?

Kofi's messy notebook gave him an answer. What made it trustworthy was a check: 8 plus 12 is 20, and 8 times 12 is 96. The notebook found the answer, and the check confirmed it.

The tidy proof is trustworthy in a different way. It gives a reason why the answer must be right, because every line follows from the one before. It still ends with a check, because a check can catch a slip in the reasoning.

(Notice something else: the 10 minus *d* and 10 plus *d* trick is the missing-corner trick from The Work That Vanishes. 10 times 10 minus a corner is exactly what the tidy proof used.)

### Practice

1. Find two numbers that add to 30 and multiply to 221. Keep a messy notebook first. Then write a tidy version in three or four lines.

2. Find two numbers that add to 20 and multiply to 99. Use Kofi's tidy method without any guessing. What is *d* this time?

3. Find two numbers that add to 20 and multiply to 91.

4. **The impossible puzzle.** Try to find two numbers that add to 20 and multiply to 101. Keep a messy notebook. What do you notice about your products? Then use the tidy method. What goes wrong with it?

5. Kofi tries 7 and 13. Write the product as a circle and pop it. Does the pair work?

### Answers

1. The middle is 15, and 15 times 15 is 225. We need 221, so the difference is 4, which means *d* is 2. The numbers are **13 and 17**. Check: 13 plus 17 is 30, and 13 times 17 is 221.
2. 100 minus 99 is 1, so *d* is 1. The numbers are **9 and 11**.
3. 100 minus 91 is 9, so *d* is 3. The numbers are **7 and 13**.
4. In the messy notebook, the products never get above 100. The best you can do is 10 and 10. In the tidy method, 100 minus *d* times *d* would have to equal 101, which would mean *d* times *d* is negative 1. No ordinary number multiplied by itself gives a negative answer. So no pair of ordinary numbers works. The messy notebook gives you a hunch, and the tidy proof tells you *why* the hunch is right.
5. ( 7 × 13 ) pops to 91, which is not 96. The sum ( 7 + 13 ) pops to 20, so the sum is fine, but the product is too small.

---

*Bridge to the rest of the series:* The messy notebook is how people really find things. The tidy proof is how they show things. Both books before this one used the tidy version. In A Useful Wrong Answer, the guess about 7/8 was a messy-notebook move. In Two Boxes, One Answer, trying numbers from 1 to 9 was a messy-notebook move, and finding the reason that A and B always match was the tidy proof.
