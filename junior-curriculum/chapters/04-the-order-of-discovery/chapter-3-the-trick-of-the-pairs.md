# The Order of Discovery

## Chapter 3: The Trick of the Pairs

Kofi's teacher gave the class a long job: *add up every number from 1 to 10.* Kofi wrote down what he did, mess and all.

### Step 1: The messy notebook

```
1 + 2 = 3
3 + 3 = 6
6 + 4 = 10
10 + 5 = 15
15 + 6 = 21
21 + 7 = 28
28 + 8 = 36
36 + 9 = 45
45 + 10 = 55
```

That took nine lines. While writing the lines, Kofi noticed something odd. The first number and the last number added to 11. So did the second and the second-to-last.

```
1 + 10 = 11
2 + 9  = 11
3 + 8  = 11
4 + 7  = 11
5 + 6  = 11
```

Five pairs, and every pair makes 11. "So I don't need nine lines," Kofi said. "I need five elevens."

### Step 2: The tidy notebook

Kofi rewrote the whole thing, leaving out the dead ends and the order of his discovery.

```
( 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 )
redraw  →  ( ( 1 + 10 ) + ( 2 + 9 ) + ( 3 + 8 ) + ( 4 + 7 ) + ( 5 + 6 ) )
pop all five  →  ( 11 + 11 + 11 + 11 + 11 )
redraw  →  ( 5 × 11 )
pop  →  55
```

The redraw keeps the value, because adding the same numbers in a different order and grouping gives the same total. The answer matches the messy notebook: 55.

### Step 3: Does it work for a bigger job?

The tidy notebook does something the messy one cannot: it tells Kofi what to do for 1 to 100 without writing a hundred lines.

There are 100 numbers, so 50 pairs. The first and last make 101, and so do the second and second-to-last.

```
( 50 × 101 )  →  5,050
```

The sum of 1 to 100 is **5,050**.

What about an odd number of numbers, such as 1 to 9? Now the pairing leaves one number alone in the middle.

```
( 1 + 9 ) ( 2 + 8 ) ( 3 + 7 ) ( 4 + 6 )  and 5 left over
( 4 × 10 ) + 5  →  45
```

So the sum of 1 to 9 is **45**.

### Step 4: Why keep the messy notebook?

Kofi nearly threw the messy notebook away. But look at what it had that the tidy one did not. It had the **nine-line check**: 55 from adding one at a time, a result that came without the pairing idea. When the pairing gave 55 too, Kofi trusted both more.

It also had the **discovery**. Nobody would have invented the pairing without noticing the pattern of 11s while writing out the slow way.

A good habit: keep the messy one for finding the idea and for checking, and use the tidy one for telling other people.

### What did we keep?

Write the answers in your notebook.

- **What did the tidy notebook keep from the messy one?** (The answer, 55, and the reason it is right.)
- **What did it drop?** (The order of discovery, and the nine slow lines.)

### What makes this trustworthy?

The reason is that pairing the ends always gives the same sum. When you move one step in from the left, the number goes up by 1. When you move one step in from the right, the number goes down by 1. So each pair keeps the same total. That works for any length, not only 10 or 100.

The messy notebook is a second, separate check: it gives the same answer by a different road.

### Practice

1. Add 1 to 20 using pairs. How many pairs? What does each pair make?

2. Add 1 to 8 using pairs.

3. Add 5 to 14 (that is 5, 6, 7, ..., 14). Be careful: what does the first pair make? How many pairs are there?

4. Add 1 to 99. This is an odd count, so one number is left in the middle. Which one?

5. Kofi says "the tidy notebook is all we need, so we can throw the messy one away." What would you say?

### Answers

1. There are 20 numbers, so 10 pairs, each making 21. ( 10 × 21 ) pops to **210**.
2. 4 pairs, each making 9. ( 4 × 9 ) pops to **36**.
3. The first pair is 5 + 14 = 19. There are 10 numbers, so 5 pairs. ( 5 × 19 ) pops to **95**.
4. 49 pairs of 100, and 50 is left in the middle. ( 49 × 100 ) + 50 = 4,900 + 50 = **4,950**.
5. The messy one holds the check and the discovery. The check is a second road to the same answer, and it is worth keeping.

---

*Bridge to the rest of the series:* In chapter 1, the tidy proof ended with a check. Here the messy notebook is the check, and the pairing idea is the discovery. In Reversing the Calculation, a diary of each press is what lets you go back, and a messy notebook is a diary of your thinking.
