# What Must Remain Unresolved

## Chapter 2: Two Jars

Tavi's sister found a second jar. Now there were two sealed jars on the table. Jar x was the first one, and the new one was jar y. Next to jar x were 3 bags of 4 marbles. Next to jar y were 2 bags of 5 marbles.

"How many marbles are there altogether?" asked the sister.

"I can't say yet," said Tavi. "But I can do a lot of it."

### Step 1: Pop what you can

Tavi drew a circle around each jar with its bags, and one big circle around everything:

```
( ( x + ( 3 × 4 ) ) + ( y + ( 2 × 5 ) ) )
```

Which circles are ready? A circle is ready when everything it needs is known. Two inner circles are ready: ( 3 × 4 ) and ( 2 × 5 ). The circles with x and y are blocked.

```
( ( x + ( 3 × 4 ) ) + ( y + ( 2 × 5 ) ) )
pop ( 3 × 4 )  →  12
pop ( 2 × 5 )  →  10
( ( x + 12 ) + ( y + 10 ) )
```

Now Tavi is stuck again. But this time the stuck pile is bigger, and it has two mysteries.

### Step 2: Redraw so the plain numbers meet

The 12 and the 10 are plain numbers, but they are in different circles. They cannot be popped together. Tavi can redraw, because adding in a different order gives the same total:

```
( ( x + 12 ) + ( y + 10 ) )
redraw  →  ( ( x + y ) + ( 12 + 10 ) )
pop ( 12 + 10 )  →  22
( ( x + y ) + 22 )
```

This is the shortest note Tavi can write: *"The two jars together, plus 22."* The mysteries x and y stay inside one circle. Nothing in that circle can be popped without knowing the jars.

Tavi can tell the sister something real: "There are at least 22 marbles, and the jars add some more."

### Step 3: One clue helps, a different one doesn't

The sister reads the labels: "These two jars together hold 15 marbles."

That is a clue about the circle ( x + y ). It does not say what x is or what y is, but it says what they add up to, which is all that circle needs.

```
( ( x + y ) + 22 )
use the clue ( x + y ) = 15
( 15 + 22 )
pop  →  37
```

The total is **37**, even though Tavi still does not know x or y separately. They could be 7 and 8, or 10 and 5, or 15 and 0. The answer does not change.

Now suppose the label had said only "jar x holds 6 marbles." That is a clue about x, but not about y, so ( x + y ) is still blocked. The total would be ( ( 6 + y ) + 22 ), which is ( y + 28 ), and Tavi would still be stuck.

The lesson: a clue helps only if it matches the shape of the blocked circle.

### Step 4: Some mysteries vanish

Look at this circle:

```
( x × 0 )
```

It holds a mystery x, so it does not look ready. But Tavi knows that anything times 0 is 0, whatever x is. So this circle can be redrawn into a plain **0**, and the mystery disappears.

```
( x × 0 )
redraw  →  0
```

Not every mystery can vanish this way. ( x + 0 ) simply becomes x, and ( x × 1 ) simply becomes x. But ( x × 0 ) is special.

### What did we keep?

Write the answers in your notebook.

- **What did the redraw in Step 2 keep?** (The total, for any x and any y.)
- **What did the clue ( x + y ) = 15 keep, and what did it leave unknown?** (It kept the sum of the jars. It left x and y unknown.)

### What makes this trustworthy?

The notebook is trustworthy because every pop is of plain numbers, and every redraw keeps the value for any x and y. So the note ( ( x + y ) + 22 ) is right for every pair of jars. You can test it with a pair, like x = 7 and y = 8: the original is ( 7 + 12 ) + ( 8 + 10 ) = 37, and the note is 15 + 22 = 37. ✓

A test with one pair of numbers is evidence. The reason is that each redraw kept the value.

### Practice

1. Pop and redraw ( ( x + ( 2 × 3 ) ) + ( y + ( 4 × 2 ) ) ) as far as you can. What is the note to self?

2. If x + y = 10, what is the total in problem 1?

3. Is this clue enough to finish problem 1: "x is 4"? Why or why not?

4. Simplify ( ( x × 0 ) + ( y + 5 ) ).

5. Dani says the clue "x + y = 15" lets us know that x is 7. Is Dani right?

6. A jar x holds a number, and Tavi writes ( ( x + 5 ) − 5 ). Redraw it to something simpler. Why does it work?

### Answers

1. Pop ( 2 × 3 ) to 6 and ( 4 × 2 ) to 8. Redraw ( ( x + 6 ) + ( y + 8 ) ) to ( ( x + y ) + ( 6 + 8 ) ), pop to 14. Note: **( ( x + y ) + 14 )**.
2. ( 10 + 14 ) pops to **24**.
3. No. It tells about x only. The circle ( x + y ) still has y in it, so it cannot be popped.
4. Redraw ( x × 0 ) to 0. Then ( 0 + ( y + 5 ) ) is ( y + 5 ). The note is **( y + 5 )**.
5. No. x could be 7 and y 8, but x could also be 10 and y 5. The clue fits many pairs.
6. It redraws to **x**. Adding 5 and then taking 5 away gives back what you started with, whatever x is.

---

*Bridge to the rest of the series:* In chapter 1, one mystery gave one stuck circle. Here two mysteries share a circle, and a clue about their sum finishes the job. In Reversing the Calculation, Omar's ×0 button cannot be undone for the same reason that ( x × 0 ) forgets x.
