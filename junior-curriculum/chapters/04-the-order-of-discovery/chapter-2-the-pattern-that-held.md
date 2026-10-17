# The Order of Discovery

## Chapter 2: The Pattern That Held, Until It Didn't

After Kofi cleaned up his notebook in chapter 1, he started collecting patterns. A pattern is exciting, because it feels like a discovery. But Kofi's teacher gave him two patterns on the same day, and they turned out very differently.

### Pattern 1: Adding odd numbers

Kofi added the odd numbers from the start, one more each time, and wrote each total in his notebook.

```
( 1 + 3 )              →  4
( 1 + 3 + 5 )          →  9
( 1 + 3 + 5 + 7 )      →  16
( 1 + 3 + 5 + 7 + 9 )  →  25
```

"4, 9, 16, 25," said Kofi. "Those are 2 × 2, 3 × 3, 4 × 4 and 5 × 5. The totals are squares!"

He wondered if it would keep going. The next total should be 6 × 6 = 36. He added the next odd number, 11, to 25 and got 36. It held again.

### Pattern 2: A recipe for prime numbers

A prime number is a number bigger than 1 that can only be shared out evenly by 1 and by itself. Examples are 2, 3, 5, 7, 11, 13. The number 15 is not prime, because 3 × 5 = 15.

Kofi's teacher gave him a recipe: *take a number n, multiply it by itself, add n, then add 41.* In circles:

```
( ( n × n ) + n ) + 41
```

Kofi tried the first five:

| n | ( ( n × n ) + n ) + 41 | Prime? |
|---|---|---|
| 1 | 43 | yes |
| 2 | 47 | yes |
| 3 | 53 | yes |
| 4 | 61 | yes |
| 5 | 71 | yes |

"Five out of five!" said Kofi. "The recipe makes primes."

The teacher smiled. "Try n = 40."

```
( ( 40 × 40 ) + 40 ) + 41
pop ( 40 × 40 )  →  1,600
( ( 1,600 + 40 ) + 41 )
pop ( 1,600 + 40 )  →  1,640
( 1,640 + 41 )
pop  →  1,681
```

And 1,681 is not prime. It is 41 × 41. The recipe worked for 40 numbers in a row, from n = 0 to n = 39, and then it broke.

### Step 1: Same kind of evidence, very different outcomes

Both patterns looked the same in Kofi's notebook. Each had a handful of examples that fit. Yet one of them is always true and the other is not.

So what is different? Not the number of examples. The recipe for primes had *forty* working examples, which is a lot more than the five that Kofi tried for the odd numbers. What is different is that one pattern comes with a **reason**.

### Step 2: The reason for the odd numbers

Picture a square of dots, 4 by 4, that is 16 dots. To make the next square, 5 by 5, you add a border along two sides, shaped like an L. How many dots are in the border? 4 along one side, 4 along the other, and 1 in the corner. That is 4 + 4 + 1 = **9**, which is the next odd number.

In circles:

```
( 4 × 4 )  →  16
add the L:  ( 16 + 9 )  →  25  =  ( 5 × 5 )
```

This works for any square, not just 4 by 4. For a square of side *n*, the L-border has *n* dots along one side, *n* along the other, and 1 in the corner, which is *n* + *n* + 1. That is always the next odd number. So every time you add the next odd number, you get the next square.

That is a reason. It does not depend on trying more examples. It says why the pattern must continue forever.

### Step 3: Is there a reason for the prime recipe?

The recipe has no L-border or anything like it. Nobody found a reason that makes it always give primes, and there is none, because at n = 40 it fails. The forty working examples were a very lucky run.

When a pattern fails, it often fails somewhere you have not looked yet. There is no way to know ahead of time how far to look. That is why a pattern, however long it holds, is called a **conjecture** until it has a reason.

### What did we keep?

Write the answers in your notebook.

- **What did the examples keep for both patterns?** (A list of cases that fit.)
- **What did the examples not keep?** (Any promise about cases nobody has tried.)

### What makes this trustworthy?

A pattern is trustworthy when you can explain *why* it must continue, as the L-border explains the odd numbers. Examples can make you suspect a pattern, and they can show it to be false. But they cannot show it true for every case, if there are endlessly many cases.

When Kofi writes a pattern in his notebook, he now marks it as one of two things: **has a reason** or **only seen so far**.

### Practice

1. Add the first six odd numbers, 1 + 3 + 5 + 7 + 9 + 11. What square do you get?

2. Use the L-border idea to explain why adding 13 to 36 gives the next square. What is it?

3. Try the recipe ( ( n × n ) + n ) + 41 for n = 6. Is the answer prime? (Hint: it is 83.)

4. Dani says, "I tested 100 numbers and the pattern held every time, so it must be true forever." What would you say to Dani?

5. At n = 41, the recipe gives ( ( 41 × 41 ) + 41 ) + 41. Show that the answer can be divided by 41.

6. Sort these into "has a reason" and "only seen so far": (a) the odd numbers add up to squares; (b) every number I tried in the prime recipe gave a prime.

### Answers

1. 1 + 3 + 5 + 7 + 9 + 11 = **36**, which is 6 × 6.
2. A 6 by 6 square has 36 dots. The L-border for a 7 by 7 square has 6 + 6 + 1 = 13 dots, and 36 + 13 = **49**, which is 7 × 7.
3. ( 6 × 6 ) pops to 36, then 36 + 6 = 42, then 42 + 41 = **83**. The number 83 is prime.
4. A hundred examples is strong evidence that the pattern holds in the cases tried. It does not say what happens in the cases nobody has tried. Dani needs a reason, or has to say "only seen so far".
5. ( 41 × 41 ) + 41 + 41 = 41 × 41 + 41 × 2 = 41 × 43, so the answer is **1,763**, and it can be divided by 41.
6. (a) has a reason, the L-border. (b) only seen so far, and in fact false.

---

*Bridge to the rest of the series:* In chapter 1, Kofi's tidy proof ended with a check. Here the check has a new job: telling the difference between a pattern with a reason and a pattern that is only seen so far. In Two Boxes, One Answer, the detective's rule says the same thing about boxes: finite tests cannot prove two things equal in every case.
