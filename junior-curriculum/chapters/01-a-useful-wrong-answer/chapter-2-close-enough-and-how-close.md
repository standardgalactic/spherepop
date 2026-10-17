# A Useful Wrong Answer

## Chapter 2: Close Enough, and How Close

Theo sold apples at a market stall. One morning a customer asked, "How many apples do you have in your 49 crates, if each crate holds 21?"

Theo had no calculator. But he noticed that 49 is nearly 50, and 21 is nearly 20, and 50 × 20 is easy.

### Step 1: An easy answer that is a little wrong

Theo drew one circle and popped it:

```
( 50 × 20 )  →  1,000
```

"About a thousand," he said.

That is a useful wrong answer. It is wrong, but it is wrong by an amount Theo can work out. To find how wrong, he moved one number at a time and wrote down what each move did.

**Move 1:** change 50 back to 49. That is one crate fewer, and a crate holds 20 in his estimate, so it takes away 20.

**Move 2:** change 20 back to 21. Now each of the 49 crates holds one more apple, so it adds 49.

Here is the notebook. Every redraw keeps the value, and every pop is easy:

```
( 49 × 21 )
redraw  →  ( ( 49 × 20 ) + ( 49 × 1 ) )
redraw  →  ( ( ( 50 × 20 ) − ( 1 × 20 ) ) + ( 49 × 1 ) )
pop ( 50 × 20 )  →  1,000
pop ( 1 × 20 )   →  20
pop ( 49 × 1 )   →  49
( ( 1,000 − 20 ) + 49 )
pop ( 1,000 − 20 )  →  980
( 980 + 49 )
pop  →  1,029
```

So Theo's estimate had two named errors: **minus 20** and **plus 49**. Together they come to plus 29, and 49 × 21 is **1,029**.

### Step 2: Which error matters?

The two errors pull in opposite directions, so you cannot tell whether the estimate is too low or too high until you compare their sizes. For 49 × 21, the plus 49 is bigger than the minus 20, so the estimate is too low.

Try 51 × 19. Starting again from ( 50 × 20 ), which is 1,000:

- 51 is one more crate of 20, so **plus 20**.
- 19 is one fewer apple in each of 51 crates, so **minus 51**.

1,000 + 20 − 51 = **969**. This time the estimate is too high, by 31.

### Step 3: A range without the exact answer

Sometimes you do not need the exact number. You only need to know a range that is certainly right.

49 × 21 is at least 49 × 20, because 21 is more than 20. And it is at most 50 × 21, because 49 is less than 50. Two easy circles give the ends:

```
( 49 × 20 )  →  980
( 50 × 21 )  →  1,050
```

So the circle ( 49 × 21 ) pops to something between 980 and 1,050. That is a **bounded estimate**. It says less than the exact answer, but it is always right.

A bound is only useful if it is tight enough for the job. Suppose a truck can carry 1,000 apples. The bounds 980 to 1,050 cannot decide whether the truck is enough, because 1,000 sits inside the range. Only the exact answer, 1,029, settles it: the truck is not big enough.

### What did we keep?

Write the answers in your notebook.

- **What did the estimate and its named errors keep?** (Everything. Add the errors back and you get the exact answer.)
- **What did the bound keep, and what did it lose?** (It kept a range that is certainly right. It lost the exact value.)

### What makes this trustworthy?

There are three different kinds of answer in this chapter.

1. **An exact fix.** Every correction is counted, so the answer is exact. This is a reason, because each redraw keeps the value.
2. **A bound.** The ends come from simple facts, such as 49 being less than 50. This is a reason too, but it only tells you a range.
3. **A plain estimate with no errors named.** "About a thousand" is only a guess until someone says how far off it could be.

A good habit is to check with a different route. Here, 50 × 21 − 21 is 1,050 − 21, which is 1,029. The routes agree.

### Practice

1. Start from ( 50 × 20 ). Name the two corrections for **51 × 19**, then find the exact answer.

2. Start from ( 100 × 10 ). Name the two corrections for **99 × 11**, then find the exact answer.

3. Find a lower bound and an upper bound for **38 × 52**, using easy circles.

4. A truck carries 1,100 apples. Do the bounds 980 to 1,050 decide whether it is big enough for 49 crates of 21? What if the truck carries only 1,000?

5. Redraw ( 49 × 20 ) as ( ( 50 × 20 ) − ( 1 × 20 ) ) and pop every circle.

### Answers

1. 51 is one more crate of 20, so **plus 20**. 19 is one less in each of 51 crates, so **minus 51**. 1,000 + 20 − 51 = **969**.
2. 99 is one fewer crate of 10, so **minus 10**. 11 is one more in each of 99 crates, so **plus 99**. 1,000 − 10 + 99 = **1,089**.
3. A lower bound is 38 × 50 = 1,900, because 52 is more than 50. An upper bound is 40 × 52 = 2,080, because 38 is less than 40. So the answer is between **1,900 and 2,080**. (The exact answer is 1,976.)
4. For a 1,100 truck, yes: the most it can be is 1,050, which fits. For a 1,000 truck, no: 1,000 is inside the range, so the bounds cannot decide.
5. ( 50 × 20 ) pops to 1,000 and ( 1 × 20 ) pops to 20, then ( 1,000 − 20 ) pops to **980**.

---

*Bridge to the rest of the series:* In chapter 1, Mara's mistake grew steadily with the mark number. Here the estimate has two separate errors, one small and one large, and naming both is what turns a guess into an answer.
