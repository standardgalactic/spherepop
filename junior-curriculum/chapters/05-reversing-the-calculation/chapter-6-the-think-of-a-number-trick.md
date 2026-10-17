# Reversing the Calculation

## Chapter 6: The Think-of-a-Number Trick

Omar did a trick at lunch. He said:

*Think of a number. Add 5. Double it. Subtract 10. Halve it. Now tell me your answer, and I will tell you your starting number.*

Lena thought of 7 and followed the steps.

```
7  →  +5  →  12
12  →  ×2  →  24
24  →  −10  →  14
14  →  ÷2  →  7
```

"7," said Lena. "That's my starting number!"

"Of course it is," said Omar. "The trick always gives you your own number back."

### Step 1: Why it works

Lena tried it again with 20.

```
20 → 25 → 50 → 40 → 20
```

Back to 20 again. She could not tell why. She drew circles for a number n she did not know:

```
( ( ( n + 5 ) × 2 ) − 10 )
redraw  →  ( ( ( n × 2 ) + ( 5 × 2 ) ) − 10 )
pop ( 5 × 2 )  →  10
( ( ( n × 2 ) + 10 ) − 10 )
redraw  →  ( ( n × 2 ) + ( 10 − 10 ) )
pop  →  ( n × 2 )
```

After the first three steps, the number n has been doubled. The +10 and −10 cancelled. The last step halves it, which undoes the doubling. That is why n comes back.

Lena noticed the buttons come in pairs of undos, as in chapter 2: ×2 and ÷2 undo each other, and +10 and −10 undo each other.

### Step 2: A trick that always gives the same number

Omar showed a second trick:

*Think of a number. Double it. Add 6. Halve it. Subtract the number you started with. The answer is always 3.*

Lena tried 4: double is 8, add 6 is 14, halve is 7, subtract 4 is **3**. She tried 10: 20, 26, 13, minus 10 is **3**.

Why?

```
( ( ( n × 2 ) + 6 ) ÷ 2 )
redraw  →  ( n + 3 )
( ( n + 3 ) − n )
redraw  →  ( 3 + ( n − n ) )
pop  →  3
```

The n vanishes. Everything about the starting number is gone, and only the 3 is left. That is why the answer is always 3.

### Step 3: Making your own trick

To make a trick, choose steps that undo each other, or steps that make the unknown cancel.

Lena built one: *Think of a number. Triple it. Add 9. Divide by 3. Subtract your number. The answer is always 3.* She tested it with 5: 15, 24, 8, minus 5 is 3. ✓

Test it with 1: 3, 12, 4, minus 1 is 3. ✓

### What did we keep?

Write the answers in your notebook.

- **What did the first trick keep?** (The starting number, all the way through.)
- **What did the second trick lose?** (The starting number: it cancels out completely.)

### What makes this trustworthy?

The reason is the circles with n. Every step is drawn with the unknown still inside, so we are not testing numbers one by one: we are showing that the trick works for all of them.

Testing with 7, 20 and 4 is evidence. The circles are the reason.

### Practice

1. Try the first trick with 12. What do you get at each step?

2. Try the second trick with 15. What is the answer?

3. Check the triple-and-add-9 trick with 8.

4. Make a trick that always ends in 5. (Hint: double, add 10, halve, subtract the number.)

5. Why would the trick fail if Omar said "subtract 12" instead of "subtract 10" in the first trick?

### Answers

1. 12 → 17 → 34 → 24 → **12**.
2. 15 → 30 → 36 → 18 → 18 − 15 = **3**.
3. 8 → 24 → 33 → 11 → 11 − 8 = **3**. ✓
4. Double, add 10, halve, subtract the number. For 6: 12, 22, 11, minus 6 is **5**. In circles, ( ( n × 2 ) + 10 ) ÷ 2 = n + 5, and subtracting n leaves 5.
5. The +10 and −12 would not cancel exactly. You would get back n − 1, not n.

---

*Bridge to the rest of the series:* In chapter 5, a redraw was the one line that could always be undone, and here the redraws are what prove the trick. In What Must Remain Unresolved, working with n unknown is exactly the way to show something is true for every number.
