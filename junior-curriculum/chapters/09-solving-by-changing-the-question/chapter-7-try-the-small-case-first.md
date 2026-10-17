# Solving by Changing the Question

## Chapter 7: Try the Small Case First

Pia was asked for **111 × 111**. She did not want to do it. So she tried a smaller question first.

### Step 1: Start small

```
( 11 × 11 )  →  121
```

That is easy. She went a little bigger.

```
( 111 × 111 )  →  12,321
```

She had to work that one out, but it was not too hard. And she noticed that 121 and 12,321 both read the same forwards and backwards, and the digits climb 1, 2, 3 and then fall again.

She tried one more.

```
( 1,111 × 1,111 )  →  1,234,321
```

The same pattern. Climb to 4, fall back. So she guessed:

*A number made of k ones, times itself, climbs from 1 to k and then falls back to 1.*

### Step 2: Use the pattern

The pattern says 11,111 × 11,111 should climb to 5 and fall.

```
11,111 × 11,111  →  123,454,321
```

And it does. Pia's small cases led her to the answer for a big case without doing the long multiplication.

That is the idea of this chapter: **change a big question into a small one, solve the small one, and look for a pattern.**

### Step 3: But does the pattern last?

Pia kept going. Nine ones squared gives 12,345,678,987,654,321. The pattern holds.

Now ten ones. The pattern says it should climb to 10, but 10 is two digits. What really happens?

```
1,111,111,111 × 1,111,111,111  →  1,234,567,900,987,654,321
```

The pattern broke. The answer starts 1,234,567, but then the middle turns into 9, 0, 0, 9 instead of climbing neatly to 10, because the "10" cannot fit in one place and spills into the next.

So the pattern held for 1 up to 9 ones, and then failed at 10. It is the same lesson as the prime recipe in The Order of Discovery: a pattern from small cases is a suggestion, not a proof.

### Step 4: Small cases are for finding, not for proving

Small cases are good for three things.

- They help you **see** a pattern.
- They help you **guess** the answer to the big case.
- They let you **check** a big answer: it should look like the small ones.

They are not a proof. A proof needs a reason, and the reason for this pattern is carrying, which is why it breaks at ten.

### What did we keep?

Write the answers in your notebook.

- **What did the small cases keep?** (The shape of the problem.)
- **What did they lose?** (Anything that only shows up at larger sizes, like carrying.)

### What makes this trustworthy?

A small case is trustworthy for what it shows: that the small answer is right. The leap to the big case is a guess until it has a reason. Pia marks it **only seen so far**, and then checks the big case directly where she can.

### Practice

1. Find 1,111 × 1,111 by the pattern. Check it is 1,234,321.

2. Use the pattern to guess 111,111 × 111,111.

3. Check 111 × 111 by splitting: ( 111 × 100 ) + ( 111 × 10 ) + ( 111 × 1 ).

4. Why does the pattern need to break at ten ones?

5. Try the small case first for 99 × 99, 999 × 999. What do you notice?

### Answers

1. Climb to 4 and fall: **1,234,321**.
2. Climb to 6 and fall: **12,345,654,321**.
3. 11,100 + 1,110 + 111 = **12,321**. ✓
4. The digit would have to be 10, which does not fit in one place, so it carries into the next place and changes the pattern.
5. 99 × 99 = 9,801 and 999 × 999 = 998,001. Each is a round number minus a small number: 10,000 − 199 and 1,000,000 − 1,999. Another pattern, with a reason: it is the square-with-a-step idea from The Work That Vanishes.

---

*Bridge to the rest of the series:* In chapter 6, a question was made friendlier by changing both numbers. Here it is made smaller. In The Order of Discovery, a pattern that works for small cases but breaks later is labelled "only seen so far".

*Next in this book:* Chapter 8: From 48 × 52 to 50 × 50.
