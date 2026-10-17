# Reversing the Calculation

## Chapter 4: Buttons on a Clock

Omar built a calculator with a clock face instead of a screen. It had the numbers 0 to 11, where 12 o'clock is called 0. When a number goes past 11, it wraps around, just like a clock.

So 9 + 5 on this clock is 14, which wraps to **2**.

### Step 1: A button that can be undone

The first button is **+5**. Starting from 3, it gives 8. Starting from 9, it gives 14, which wraps to 2.

Can it be undone? Omar looked for a button that sends every answer back. Adding 5 and then adding 7 makes 12, and 12 wraps back to 0. So **+7 undoes +5**.

```
9  →  +5  →  2
2  →  +7  →  9
```

Because each start gives a different answer, +5 can always be undone.

### Step 2: The ×5 button

Omar added a **×5** button: multiply by 5, then wrap. Here are all twelve starts:

```
start:   0  1  2  3  4  5  6  7  8  9  10 11
×5:      0  5  10 3  8  1  6  11 4  9  2  7
```

Look at the second row. Every number from 0 to 11 appears exactly once. So each answer points to one start, and **×5 can be undone**.

What undoes it? Try ×5 a second time on the answers: 5 × 5 = 25, which wraps to 1. So pressing ×5 twice brings you back to where you began. A button that undoes itself!

```
3  →  ×5  →  15 wraps to 3     (3 is a lucky one)
2  →  ×5  →  10  →  ×5  →  50 wraps to 2
```

### Step 3: The ×2 button

Now the **×2** button.

```
start:   0  1  2  3  4  5  6  7  8  9  10 11
×2:      0  2  4  6  8  10 0  2  4  6  8  10
```

Starts 0 and 6 both give 0. Starts 1 and 7 both give 2. Every answer appears twice, and the odd numbers 1, 3, 5, 7, 9, 11 never appear as answers at all. So **×2 cannot be undone**: if the clock says 4, the start might have been 2 or 8.

It is the same thing that happened with the round button in chapter 2. Many starts, one answer.

### Step 4: Which multiply buttons are safe?

Omar tried ×3, ×4 and ×7 on the 12-clock too.

- **×3**: 0, 3, 6, 9, 0, 3 ... lots of repeats. Cannot be undone.
- **×4**: 0, 4, 8, 0, 4, 8 ... cannot be undone.
- **×7**: 0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5. Every number once. **Can be undone**, by pressing ×7 again, because 7 × 7 = 49, which wraps to 1.

Omar noticed that the safe buttons were 1, 5, 7 and 11. The unsafe ones shared a factor with 12: 2, 3, 4, 6, 8, 9, 10. This is a pattern from tries, and the reason is beyond this book. Omar marked it *only seen so far*, as Kofi would.

### What did we keep?

Write the answers in your notebook.

- **What does a button that can be undone keep?** (Every detail of the start.)
- **What does a button that cannot be undone lose?** (Which of several starts it was.)

### What makes this trustworthy?

The list of all twelve starts is a complete test, because the clock has only twelve numbers. That is a rare case where testing every start is possible, and so a table is a real proof here.

A button that undoes itself is checked by pressing it twice and landing back at the start.

### Practice

1. On the 12-clock, what is 8 + 7? Which button undoes +7?

2. Press ×5 on 4. What do you get? Press ×5 again. What do you get?

3. Start at 2. Press +5, then ×5. What is the answer? Undo it step by step.

4. Why can ×6 not be undone on the 12-clock? Give two starts with the same answer.

5. List ×7 for the starts 1, 2 and 3.

### Answers

1. 8 + 7 = 15, which wraps to **3**. The button **+5** undoes +7, because 7 + 5 = 12.
2. 4 × 5 = 20, which wraps to **8**. Then 8 × 5 = 40, which wraps to **4**. Back at the start.
3. 2 + 5 = 7, then 7 × 5 = 35, which wraps to **11**. To undo, press ×5 on 11: 55 wraps to 7. Then +7: 14 wraps to **2**. ✓
4. Starts 1 and 3 both give 6 (1 × 6 = 6 and 3 × 6 = 18, which wraps to 6). So the answer 6 does not say which start it came from.
5. 1 × 7 = **7**, 2 × 7 = 14 wraps to **2**, 3 × 7 = 21 wraps to **9**.

---

*Bridge to the rest of the series:* In chapter 2, whether a button can be undone depended on whether two starts give one answer. On a small clock, that can be tested for every start. In Two Boxes, One Answer, a table of every case is the only kind of test that settles anything.
