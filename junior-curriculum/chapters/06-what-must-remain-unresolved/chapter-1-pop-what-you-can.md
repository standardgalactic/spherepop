# What Must Remain Unresolved

## Chapter 1: Pop What You Can

Tavi had a jar of marbles on the table. The lid was stuck, and nobody knew how many marbles were inside. Next to the jar sat 3 small bags, and each bag held 4 marbles.

Tavi's little sister asked, "How many marbles do you have altogether?"

"I don't know yet," said Tavi, "because I can't open the jar. But I can already do part of the work."

### Step 1: Draw circles

Let's call the number of marbles in the jar **x**. It is a mystery number.

Tavi wrote down the whole total, and then drew a circle around each piece that belongs together. In this book, a pair of round brackets is a circle.

```
( x + ( 3 × 4 ) )
```

There are two circles here, one inside the other. The inner circle holds the bags: 3 bags of 4. The outer circle holds the whole total: the jar plus the bags.

### Step 2: The rules of popping

Tavi has three rules.

1. **A circle is ready when everything needed for its calculation is known and its operation is allowed.** A mystery number or an unfinished smaller circle inside means it is not ready.
2. **Popping a circle replaces it with its value.** Everything outside the circle stays exactly as it was.
3. **If no circle is ready, you stop.** What is left is your note to yourself.

### Step 3: Pop the circle that is ready

Which circle is ready? The inner one. It holds only the plain numbers 3 and 4. The outer one holds x, so it must wait.

Tavi pops the inner circle. Here is the notebook, with a line for each pop:

```
( x + ( 3 × 4 ) )
pop ( 3 × 4 )  →  12
( x + 12 )
```

Now Tavi checks rule 3. The only circle left holds x, so it is not ready. Tavi is **stuck**, and that is allowed. Being stuck does not mean something went wrong. It means every bit of work that did not need the jar has been done.

What is left, ( x + 12 ), is the note to self: *"Whatever is in the jar, add 12."*

When the jar finally opens and there are 20 marbles inside, Tavi puts 20 in place of x. Now 20 + 12 is ready to pop, and the answer is 32.

**Test the note:** Does it match the original? With x = 20, the original is 20 + 3 × 4 = 32, and the note gives 20 + 12 = 32. They agree.

### Step 4: What a pop forgets

When Tavi popped the circle ( 3 × 4 ), something was lost. The 12 does not remember that it came from 3 bags of 4. It could have been 2 bags of 6, or 12 bags of 1.

This is the same thing that happened to Lena's calculator in Reversing the Calculation: once the answer is on the screen, the road that led to it is gone.

The notebook solves this. Each line of the trace says *what was popped* and *what it became*. The circles forget, but the notebook remembers.

### Step 5: Which circle first?

Sometimes more than one circle is ready. Look at this one:

```
( ( 2 + 3 ) × ( 4 + 1 ) )
```

Both inner circles hold only plain numbers, so either can be popped first.

**Pop the left one first:**

```
( ( 2 + 3 ) × ( 4 + 1 ) )
pop ( 2 + 3 )  →  5
( 5 × ( 4 + 1 ) )
pop ( 4 + 1 )  →  5
( 5 × 5 )
pop ( 5 × 5 )  →  25
```

**Pop the right one first:**

```
( ( 2 + 3 ) × ( 4 + 1 ) )
pop ( 4 + 1 )  →  5
( ( 2 + 3 ) × 5 )
pop ( 2 + 3 )  →  5
( 5 × 5 )
pop ( 5 × 5 )  →  25
```

Both trails end at 25. For this problem the order did not matter, though the two notebooks look different. Sometimes the order changes how many steps you take, and later chapters will look at that.

### Step 6: What popping cannot do

Now try this one:

```
( ( x + 3 ) × 4 )
```

Is any circle ready? The inner circle holds x. The outer circle holds a circle, so it is not ready either. Tavi is stuck at the very start.

Popping alone cannot make progress here. That is not a flaw in the circles. It means this problem needs a **redraw** first, such as sharing the "× 4" with everything inside the inner circle:

```
( ( x + 3 ) × 4 )
redraw  →  ( ( x × 4 ) + ( 3 × 4 ) )
pop ( 3 × 4 )  →  12
( ( x × 4 ) + 12 )
```

The redraw keeps the value, because four lots of ( x + 3 ) is four lots of x and four lots of 3. After it, one circle is ready, and Tavi can pop it. The circle with x in it must still wait.

Notice also that ( x + ( 3 × 4 ) ) and ( ( x + 3 ) × 4 ) are different problems, because the circles are in different places. With x = 5, the first gives 5 + 12 = 17 and the second gives 8 × 4 = 32. The circles tell you what belongs together, so moving them changes the question.

### What did we keep?

Write the answers in your notebook.

- **What did the popping keep?** (The total, for every possible x. Test it with a number.)
- **What did a pop forget?** (How the value was made. The 12 forgot it was 3 bags of 4.)
- **What did Tavi do ahead of time, and what was left?** (Tavi popped the circle with plain numbers and left the one holding x.)

### What makes this trustworthy?

A pop is trustworthy because rule 2 is exact: the circle ( 3 × 4 ) really does have the value 12, and nothing outside it was touched. So the note ( x + 12 ) must agree with the original for every x, and that is a reason, not just a test.

Testing with a number, as we did with x = 20, is a second, separate kind of evidence. It can catch a slip in the notebook.

### Practice

For each one, write out the notebook with a line for each pop. If you get stuck, say what is left.

1. `( x + ( 2 × 6 ) )`

2. `( ( 5 + 4 ) × x )`

3. `( ( 10 − 4 ) + x )`

4. `( ( 3 + 2 ) × ( x + 1 ) )`

5. `( ( 6 − 1 ) × ( 3 + 4 ) )`. Pop it in two different orders. Do you get the same answer?

6. Dani says that ( x + ( 3 × 4 ) ) is the same as ( ( x + 3 ) × 4 ). Use x = 5 to show that Dani is wrong. Which circle is in the wrong place?

### Answers

1. Pop ( 2 × 6 ) to get 12. Left: **( x + 12 )**. Stuck, because of x.
2. Pop ( 5 + 4 ) to get 9. Left: **( 9 × x )**. Stuck. Check with x = 2: the original gives 9 × 2 = 18, and the note gives 18.
3. Pop ( 10 − 4 ) to get 6. Left: **( 6 + x )**. Stuck.
4. Pop ( 3 + 2 ) to get 5. Left: **( 5 × ( x + 1 ) )**. The circle ( x + 1 ) holds x, so it is not ready, and neither is the outer one. Stuck.
5. Pop ( 6 − 1 ) to get 5, then ( 3 + 4 ) to get 7, then ( 5 × 7 ) to get **35**. Pop ( 3 + 4 ) first, then ( 6 − 1 ), then ( 5 × 7 ): again **35**.
6. With x = 5, the first is 5 + 12 = 17 and the second is 8 × 4 = 32. They are different, so Dani is wrong. The circle around "x + 3" is in the wrong place: it takes the 3 away from the bags and gives it to the jar.

---

*Bridge to the rest of the series:* In Reversing the Calculation, Lena could not tell which road led to 12, and a pop forgets in the same way. The notebook of pops is the "diary" from that chapter, written at the level of one circle per line.
