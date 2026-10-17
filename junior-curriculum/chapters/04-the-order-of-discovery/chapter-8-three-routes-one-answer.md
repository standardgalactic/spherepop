# The Order of Discovery

## Chapter 8: Three Routes, One Answer

Kofi had a map with a camp marked at the bottom of a hill. Three hikers started from the same place at the top, each on a different trail. The question for this chapter is simple: **do different trails reach the same camp?**

In maths, the hill is a problem, the trails are different orders and methods, and the camp is the answer.

### Step 1: Three routes to 48 × 52

Here are three ways to reach the same number, from the case file.

```
Route 1:  48 × 52  →  ( 50 × 50 ) − ( 2 × 2 )  =  2,500 − 4        =  2,496
Route 2:  48 × 52  →  ( 48 × 50 ) + ( 48 × 2 ) =  2,400 + 96       =  2,496
Route 3:  48 × 52  →  ( 50 × 52 ) − ( 2 × 52 ) =  2,600 − 104      =  2,496
```

The routes look nothing alike. Route 1 balances into a square. Route 2 splits 52 into 50 + 2. Route 3 splits 48 into 50 − 2. All three reach **2,496**.

### Step 2: Different orders of popping

Even within one route, there can be a choice about what to pop first. Take Route 1:

```
( ( 50 × 50 ) − ( 2 × 2 ) )
```

Two circles are ready: ( 50 × 50 ) and ( 2 × 2 ). You can pop either one first.

```
Order A:   pop ( 50 × 50 ) → 2,500,   pop ( 2 × 2 ) → 4,   pop ( 2,500 − 4 ) → 2,496
Order B:   pop ( 2 × 2 ) → 4,   pop ( 50 × 50 ) → 2,500,   pop ( 2,500 − 4 ) → 2,496
```

Two orders, one answer. The two small circles do not touch each other, so the order cannot change what happens.

### Step 3: Counting routes and snapshots

Kofi tried counting for a small case:

```
( ( 2 × 3 ) + ( 4 × 5 ) )
```

Call the circles A = ( 2 × 3 ), B = ( 4 × 5 ), and C, the big one. C cannot be popped until A and B are done.

- Orders: **A, B, C** or **B, A, C**. That is **2 orders**.
- Snapshots (what the notebook could look like at a moment): start; after A; after B; after A and B; after all. That is **5 snapshots**.

Both orders end with 6 + 20 = **26**.

Then he tried a bigger one with five circles:

```
( ( ( 2 × 3 ) + ( 4 × 5 ) ) × ( 6 − 1 ) )
```

Call the circles A = ( 2 × 3 ), B = ( 4 × 5 ), D = ( 6 − 1 ), S for the sum, and P for the final product. S needs A and B first, and P needs S and D first. The valid orders are:

```
A B D S P     A B S D P     A D B S P     B A D S P
B A S D P     B D A S P     D A B S P     D B A S P
```

There are **8 orders**. The snapshots are **11**. Every order ends at ( 26 × 5 ) = **130**.

Many orders, fewer snapshots, one answer. The orders weave together and pass through the same snapshots.

### Step 4: Two different things

Kofi noticed that there are two different questions here.

- **Agreement.** If two routes both finish, do they give the same answer? Yes, whenever each redraw keeps the value and each pop is done correctly.
- **Discovery.** Does a given route actually *finish*? Not every trail does. A rule like "add 4, then take away 4, then add 4 again" can go on forever and never reach the camp.

A trail that never arrives does not move the camp. The answer is still 2,496 even if one hiker walks in circles.

### Step 5: Using disagreement as a check

When two routes **disagree**, the camp has not moved. One of the routes has a slip, but the disagreement does not say which. You have to look at each step.

### What did we keep?

Write the answers in your notebook.

- **What did the different routes keep?** (The value, since every redraw preserved it.)
- **What did they change?** (The steps, the order, and the amount of work.)

### What makes this trustworthy?

Agreement between routes is a good reason to trust an answer, because the routes shared no steps. It is stronger when the routes are very different from each other. It is not a proof that both are free of slips, but two independent routes would have to slip in the same way to agree on a wrong answer. Kofi marks an answer reached by two different routes as **has a reason**.

### Practice

1. Find **49 × 51** by two routes: around 50, and by 49 × 50 + 49 × 1. Do they agree?

2. For ( ( 1 + 2 ) × ( 3 + 4 ) ), name the circles, count the orders and the snapshots, and find the value.

3. A student finds 38 × 42 two ways. Route 1: 1,600 − 4. Route 2: 38 × 40 + 38 × 2 = 1,520 + 66. Do they agree? What do you do?

4. True or false: if two routes agree, the answer must be right. Explain.

5. Why does a hiker who never finishes not change the answer?

### Answers

1. Route 1: 2,500 − 1 = 2,499. Route 2: 2,450 + 49 = 2,499. They **agree**.
2. Circles: ( 1 + 2 ), ( 3 + 4 ), and the product. Orders: **2**. Snapshots: **5**. Value: 3 × 7 = **21**.
3. Route 1 gives 1,596 and Route 2 gives 1,586, so they **disagree**. There is a slip somewhere. Check each step: 38 × 2 is 76, not 66. The correct sum is 1,520 + 76 = **1,596**.
4. **False**, strictly. It is a good reason, but two routes could slip in the same way. It is far more trustworthy than one route alone.
5. The answer belongs to the problem, not to the route. A route that does not arrive has just not found it.

---

*Bridge to the rest of the series:* In The Boundary of a Step, one of the routes is lazy and one is eager, and the lazy one can arrive when the eager one cannot. In Two Boxes, One Answer, two boxes give the same answer at different cost.
