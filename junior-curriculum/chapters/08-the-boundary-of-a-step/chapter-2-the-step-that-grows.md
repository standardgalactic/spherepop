# The Boundary of a Step

## Chapter 2: The Step That Grows

Sade and Tomas were at it again. This time the problem was **3⁴**.

The little raised 4 tells you how many copies of 3 to multiply together. So 3⁴ means 3 × 3 × 3 × 3. These are called *powers*, and 3⁴ is read "3 to the power of 4."

Both cousins had the same plain calculator, with a × button and nothing else. They disagreed about what the button could be used on.

**Tomas's rule:** each step multiplies the number on the screen by 3.

**Sade's rule:** each step multiplies two numbers you already have.

### Step 1: Counting the steps

**Tomas** followed his rule:

3 × 3 = 9. Then 9 × 3 = 27. Then 27 × 3 = **81**.

That is 3 multiplications, so **3 steps**.

**Sade** started the same way: 3 × 3 = 9. Then she noticed she could use the 9 twice:

9 × 9 = **81**.

That is **2 steps**.

Multiplying a number by itself is called *squaring*. 9 squared is 81.

Same answer, fewer steps. Notice where the saving came from. It did not come from a special button. It came from reusing a result she had already worked out, the 9. Tomas could do exactly the same thing on his plain calculator once he spotted it. Sade's rule just allows it.

### In circles

Here are Tomas's and Sade's notebooks.

```
( ( ( 3 × 3 ) × 3 ) × 3 )          Tomas
pop ( 3 × 3 )  →  9
pop ( 9 × 3 )  →  27
pop ( 27 × 3 )  →  81              3 pops
```

```
( ( 3 × 3 ) × ( 3 × 3 ) )          Sade
pop ( 3 × 3 )  →  9     (use this 9 in both places)
pop ( 9 × 9 )  →  81               2 pops
```

Sade's notebook rule is this: once a circle has popped to a number, you may use that number again as often as you like. That is where her saving comes from. Tomas can reuse a value too, once he notices it.

### Step 2: A step that grows itself

Now the numbers get larger. Try the powers of 2, using a menu that has only two steps: **double** and **square**.

**Starting at 2, how fast can we grow?**

Square 2 and you get 4. Square 4 and you get 16. Square 16 and you get 256. Square 256 and you get **65,536**.

That is only **4 steps** to reach 65,536. For comparison, starting from the same 2, doubling over and over takes 15 steps, and counting up by ones takes 65,534 steps. Squaring grows so fast because each squaring uses the whole number as both ingredients.

Now aim at a specific number: **2¹⁰**, which is 1,024.

- Doubling only: start at 2 and double nine times (4, 8, 16, 32, 64, 128, 256, 512, 1,024). That is **9 steps**.
- With squaring allowed:
  - Square 2 to get 4. (1 step)
  - Square 4 to get 16. (2 steps)
  - Double 16 to get 32. (3 steps)
  - Square 32 to get **1,024**. (4 steps)

So 4 steps instead of 9.

### Step 3: Why squaring works

Look at the last move, squaring 32. Why does that give 2¹⁰?

32 is 2⁵, which means five 2s multiplied together. Squaring 32 means multiplying 32 by 32, which puts two piles of five 2s together. That makes ten 2s multiplied together. And ten 2s is 2¹⁰.

So squaring *doubles the number of copies*. Doubling the number itself only adds *one* more copy. That is why squaring is the step that grows.

### Step 4: The menu decides

How many steps does 2¹⁰ take? It depends on the menu again, just as in chapter 1.

- Menu with only **double**: 9 steps.
- Menu with **double and square**: 4 steps.
- Menu with a **power button** on the calculator: 1 step.

All three are honest counts. They answer different questions.

For a power that is not just 2s, use the same idea. Try **3⁵**, with a menu of "multiply by 3" and "square":

- Square 3 to get 9. (1 step)
- Square 9 to get 81. (2 steps)
- Multiply 81 by 3 to get **243**. (3 steps)

Using only "multiply by 3," it would take 4 steps (9, 27, 81, 243).

### What did we keep?

Write the answers in your notebook.

- **What stayed the same whichever menu we used?** (The answer: 81, or 1,024, or 243.)
- **What changed?** (The number of steps.)

### What makes this count trustworthy?

Three things have to be true.

First, the rewriting must be exact. 2⁵ × 2⁵ really is 2¹⁰, because five 2s and five 2s together make ten 2s. That is a reason, not a guess.

Second, the saving must have a source we can name. Here it is reusing a result we already worked out, such as the 9 or the 32. Squaring is just the name for multiplying a number by itself.

Third, the step counts are only comparable under the same menu, as in chapter 1. Saying "4 steps" without saying "with double and square" is not a complete answer.

### Practice

1. Find **3⁴** using only the rule "multiply the number on the screen by 3." Then find it again by multiplying a result by itself. How many steps each time?

2. Find **5⁴** (which means 5 × 5 × 5 × 5) by squaring. How many steps?

3. Find **2⁸** using the menu of **double** and **square**. How many steps? How many steps would it take with doubling only?

4. Find **3⁵** with a menu of "multiply by 3" and "square." Check by multiplying 3 five times.

5. *Brave reader:* Can you reach 2¹⁰ in only **3 steps** with the menu of double and square? Try listing every number you can reach after each step.

6. Write 5⁴ with circles so that one pop can be reused. How many pops does it take?

### Answers

1. With "multiply by 3" only: **3 steps** (9, 27, 81). By multiplying a result by itself: **2 steps** (9, then 9 × 9 = 81).
2. 5 × 5 = 25, then 25 × 25 = **625**. That is **2 steps**.
3. Square 2 to get 4, square 4 to get 16, square 16 to get **256**. That is **3 steps**. With doubling only, it takes **7 steps** (4, 8, 16, 32, 64, 128, 256).
4. 3² = 9, 9² = 81, 81 × 3 = **243**. That is **3 steps**. Check: 3 × 3 × 3 × 3 × 3 = 243.
5. **No.** After 1 step, the only number you can reach is 4 (doubling and squaring 2 both give 4). After 2 steps you can reach 8 or 16. After 3 steps you can reach 16, 64, 32 or 256. Since 1,024 is not on that list, it cannot be reached in 3 steps. The 4-step route is the shortest with this menu.
6. ( ( 5 × 5 ) × ( 5 × 5 ) ). Pop ( 5 × 5 ) to get 25 and use it in both places, and then ( 25 × 25 ) pops to **625**. That is **2 pops**.

---

*Bridge to the rest of the series:* In The Work That Vanishes, squares were areas, and 50 × 50 gave the big square. Here the same idea shows up as a step that grows. In The Choice Before the Calculation, splitting 36 as 4 × 9 to work out 25 × 36 was a choice made before any arithmetic, and spotting that a result can be reused is a choice of the same kind.
