# The Work That Vanishes

## Chapter 5: Choosing the Corner

Ines now knows several tricks. The missing corner, the square with a step, the two strips near 100, and the "one more column" idea from the first chapter. Her next problem is not a new trick. It is choosing the right one.

### Step 1: Look at the numbers first

Before she calculates, Ines looks at the numbers and asks what they are close to.

**47 × 53.** The numbers are on either side of 50, each 3 away. That is the missing corner.

```
( 47 × 53 )
redraw  →  ( ( 50 − 3 ) × ( 50 + 3 ) )
redraw  →  ( ( 50 × 50 ) − ( 3 × 3 ) )
pop  →  ( 2,500 − 9 )
pop  →  2,491
```

**97 × 96.** Both numbers are a little under 100. That is the two-corner trick: shortfalls 3 and 4.

```
( ( 97 − 4 ) × 100 ) + ( 3 × 4 )  →  9,300 + 12  →  9,312
```

**18 × 23.** The numbers are not close to each other or to 100. But 18 × 22 is something Ines already knows is 396, and 23 is one more column.

```
( 18 × 23 )
redraw  →  ( ( 18 × 22 ) + ( 18 × 1 ) )
pop  →  ( 396 + 18 )
pop  →  414
```

**13 × 27.** Nothing special here. Ines splits one number into friendly pieces.

```
( 13 × 27 )
redraw  →  ( ( 10 × 27 ) + ( 3 × 27 ) )
pop  →  ( 270 + 81 )
pop  →  351
```

### Step 2: A little table

Ines made herself a table of what to look for.

| If the numbers are... | Use... | Example |
|---|---|---|
| on either side of a friendly middle | the missing corner | 47 × 53 |
| both just under (or over) 100 | two corners | 97 × 96 |
| one more or less than a product you know | one more column | 18 × 23 |
| none of these | split one number | 13 × 27 |

The tricks do not change the answer. They change how much work it is. All four methods would give the right answer for every product, and Ines picked the cheapest.

### Step 3: Counting the work

How much cheaper? Ines counted pops for 47 × 53.

- Missing corner: 3 pops ( 50 × 50, 3 × 3, and the subtraction ), and the two products are easy.
- Splitting 47 as 40 + 7: 3 pops too ( 40 × 53, 7 × 53, and the addition ), but both products are harder to do in your head.

Both are right. As in The Boundary of a Step, the count depends on the tools you allow, and the number of pops is not the whole story. Here the missing corner has the easier pops.

### What did we keep?

Write the answers in your notebook.

- **What stayed the same whichever trick she used?** (The product.)
- **What changed?** (How many pops, and how easy each one was.)

### What makes this trustworthy?

Every trick is a redraw that keeps the value, so any of them gives the right product. The trust comes from the reason for each trick, and from checking with a second one. Ines checks 47 × 53 = 2,491 by splitting: 47 × 50 + 47 × 3 = 2,350 + 141 = 2,491. ✓

### Practice

Choose a trick for each, say why, and find the product.

1. 38 × 42

2. 99 × 98

3. 24 × 26

4. 19 × 31 (Hint: their sum is 50, so the middle is 25.)

5. Ines picks the "split one number" trick for 47 × 53 and gets 2,491. Is she wrong?

### Answers

1. On either side of 40 (2 away): ( 40 × 40 ) − ( 2 × 2 ) = 1,600 − 4 = **1,596**.
2. Both just under 100, shortfalls 1 and 2: ( ( 99 − 2 ) × 100 ) + ( 1 × 2 ) = 9,700 + 2 = **9,702**.
3. On either side of 25 (1 away): 625 − 1 = **624**.
4. On either side of 25 (6 away): 625 − 36 = **589**.
5. No. All the tricks keep the value, so she gets the right answer. She just did more work than she needed.

---

*Bridge to the rest of the series:* In The Choice Before the Calculation, picking the representation comes before calculating. This chapter does that for products. In A Useful Wrong Answer, the second route used as a check is just another trick from this table.
