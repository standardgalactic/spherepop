# A Useful Wrong Answer

## Chapter 5: Errors That Cancel

Theo wanted the total of four crate counts: 47, 38, 52 and 41. He rounded each to the nearest ten, because tens are easy to add.

```
( 50 + 40 + 50 + 40 )  →  180
```

The exact total is 178. So Theo's estimate is off by 2. That is surprisingly close for four roundings, and he wondered why.

### Step 1: Track every error

Theo wrote the error of each rounding. In this book, **error = estimate minus exact**, so a rounding up has a plus and a rounding down has a minus.

| Number | Rounded | Error |
|---|---|---|
| 47 | 50 | +3 |
| 38 | 40 | +2 |
| 52 | 50 | −2 |
| 41 | 40 | −1 |

Add the errors: +3 + 2 − 2 − 1 = **+2**. The estimate is 2 too high.

The errors pulled in both directions. The plus errors and the minus errors partly cancelled each other, so the total error is small.

In circles:

```
( ( 47 + 38 ) + ( 52 + 41 ) )
redraw  →  ( ( 50 + 40 + 50 + 40 ) − ( ( 3 + 2 ) − ( 2 + 1 ) ) )
pop  →  ( 180 − 2 )
pop  →  178
```

### Step 2: When the errors do not cancel

Now a different list: 44, 43, 42 and 41. Round each to the nearest ten.

```
( 40 + 40 + 40 + 40 )  →  160
```

The exact total is 170. Every number was rounded **down**, so every error is a minus: −4, −3, −2, −1. They all pull the same way, and they add up to −10.

Rounding the first list gave an error of 2. Rounding this list gave an error of 10. The same method, but the first time the errors cancelled and this time they stacked.

### Step 3: What to watch for

Errors cancel when some numbers round up and some round down. They stack when most numbers sit on the same side of a round number.

A careful estimator asks two questions.

- Do the numbers round in **both directions**? Then the errors will probably partly cancel.
- Do they all round **the same way**? Then expect a larger error, and be ready to correct for it.

Notice the word "probably". Cancelling is not a promise. A list can round up and down and still leave a big total error if the ups are much bigger than the downs.

### What did we keep?

Write the answers in your notebook.

- **What did the estimate keep when errors cancelled?** (Nearly the whole total.)
- **What did it lose when errors stacked?** (A noticeable part, 10 out of 170.)

### What makes this trustworthy?

The error of each rounding is exact, so the total error is exact too: it is the sum of the individual errors. That is a reason. The advice "errors tend to cancel" is only a tendency, so check by adding the errors whenever it matters.

### Practice

1. Round 63, 58, 71 and 49 to the nearest ten and add. Then find the exact total and the error.

2. Write the four errors for problem 1. Do they add to the total error?

3. Round 34, 33, 32 and 31 to the nearest ten. Do the errors cancel or stack? What is the total error?

4. Make a list of three numbers whose rounding errors cancel exactly to 0.

5. Theo says "rounding always gives a small error." What would you tell him?

### Answers

1. Rounded: 60 + 60 + 70 + 50 = **240**. Exact: 63 + 58 + 71 + 49 = **241**. The estimate is 1 too low.
2. Errors: 60 − 63 = −3, 60 − 58 = +2, 70 − 71 = −1, 50 − 49 = +1. They add to −3 + 2 − 1 + 1 = **−1**. Yes.
3. All round down, so they **stack**: −4, −3, −2, −1 add to **−10**. (Estimate 120, exact 130.)
4. For example 14, 16 and 20: 14 rounds to 10 (−4), 16 rounds to 20 (+4), 20 stays 20 (0). The errors add to 0. Many other lists work.
5. Rounding can give a small error or a big one. It depends on whether the errors cancel or stack, and on how big the answer is.

---

*Bridge to the rest of the series:* In chapter 4, an error was small or large compared with the answer. Here each rounding has its own error, and the total error is their sum. In The Work That Vanishes, the strips and the shared corner are errors that have to be added back in the right direction.
