# A Useful Wrong Answer

## Chapter 6: Guess the Average, Then Correct the Guess

Theo sold apples for four days: 90, 85, 100 and 81 on the four days. What was the average number sold per day?

The usual way is to add them all and divide by 4. Theo tried something different. He made a guess, and then corrected it.

### Step 1: Guess, then measure how far off

Theo guessed that the average was **90**. Then he checked each day against the guess.

| Day | Sold | Compared with 90 |
|---|---|---|
| 1 | 90 | 0 |
| 2 | 85 | 5 below |
| 3 | 100 | 10 above |
| 4 | 81 | 9 below |

The days above the guess add up to 10. The days below add up to 5 + 9 = 14. So overall, the four days are 14 − 10 = **4 below** the guess.

If four days are 4 below in total, then each day is 4 ÷ 4 = **1 below** on average. So the average is 90 − 1 = **89**.

In circles:

```
( ( 90 + 85 + 100 + 81 ) ÷ 4 )
```

becomes

```
( 90 − ( ( 14 − 10 ) ÷ 4 ) )
pop ( 14 − 10 )  →  4
pop ( 4 ÷ 4 )    →  1
pop ( 90 − 1 )   →  89
```

Check by the usual way: 90 + 85 + 100 + 81 = 356, and 356 ÷ 4 = 89. ✓

### Step 2: A good guess makes the correction small

Suppose the four days had been 78, 82, 85 and 75, and Theo guessed 80.

- 78 is 2 below.
- 82 is 2 above.
- 85 is 5 above.
- 75 is 5 below.

Above: 2 + 5 = 7. Below: 2 + 5 = 7. They cancel exactly, so the four days are 0 away from the guess. The average is **80**. Check: 78 + 82 + 85 + 75 = 320, and 320 ÷ 4 = 80. ✓

### Step 3: A bad guess still works

What if Theo had guessed 100 for the first list? Every day is at or below 100, by 10, 15, 0 and 19. That is 44 below in total, and 44 ÷ 4 = 11 below on average.

```
( 100 − 11 )  →  89
```

Same answer, 89. A bad guess just makes a bigger correction.

This is Split, Balance, Correct again. The guess is the balance, the differences are the split, and the correction puts back what the guess got wrong.

### What did we keep?

Write the answers in your notebook.

- **What stayed the same whatever the guess?** (The average, 89.)
- **What changed?** (The size of the correction.)

### What makes this trustworthy?

The reason is that every day's difference from the guess is counted, above and below, and the total difference is shared among the days. Nothing is left out. The guess can be anything, because the correction repairs whatever the guess missed.

The check is the plain way: add everything and divide.

### Practice

1. Find the average of 72, 68, 75 and 65 with a guess of 70.

2. Find the average of 34, 41, 38 and 47 with a guess of 40.

3. Find the average of 20, 30, 40 and 22 with a guess of 30.

4. Find the average of 12, 15, 18 and 15. What guess is easiest?

5. Dani guessed 50 for the first list (90, 85, 100, 81). What should the correction be, and what average does she get?

### Answers

1. Differences: 2 above, 2 below, 5 above, 5 below. They cancel, so the average is **70**. (Sum 280, divided by 4.)
2. Differences: 6 below, 1 above, 2 below, 7 above. Above: 8. Below: 8. The average is **40**.
3. Differences: 10 below, 0, 10 above, 8 below. Net 8 below, shared among 4 days is 2 below. The average is 30 − 2 = **28**.
4. A guess of 15 works well. Differences: 3 below, 0, 3 above, 0. They cancel, so the average is **15**.
5. Every day is above 50 by 40, 35, 50 and 31, which is 156 in total. 156 ÷ 4 = 39. So 50 + 39 = **89**.

---

*Bridge to the rest of the series:* In chapter 5, errors from roundings could cancel or stack. Here the guess is deliberately off, and the differences above and below are the errors that get put back. In The Work That Vanishes, 50 × 50 as a guess for 48 × 52 and the corner as the correction are the same shape.
