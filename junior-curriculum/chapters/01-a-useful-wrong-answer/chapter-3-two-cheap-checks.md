# A Useful Wrong Answer

## Chapter 3: Two Cheap Checks

Theo's friend Nia helped at the stall. She worked out 23 crates of 47 apples and wrote **1,071** on a card. Theo's cousin Ola worked out the same thing and wrote **1,801**. A third helper wrote **1,081**.

Three answers, and only one of them can be right. Theo did not want to redo all the work three times. He wanted a quick way to throw out bad answers.

### Step 1: The size check

First Theo made an easy estimate with round numbers. He set a floor and a ceiling, as in chapter 2:

```
( 20 × 40 )  →  800     (23 is more than 20, and 47 is more than 40, so the real answer is bigger)
( 25 × 50 )  →  1,250   (23 is less than 25, and 47 is less than 50, so the real answer is smaller)
```

So 23 × 47 must be somewhere between 800 and 1,250.

- Nia's 1,071 fits in the range.
- Ola's 1,801 does not. It is too big, so Ola's answer is **wrong**.
- The third helper's 1,081 fits.

The size check threw out one answer in a few seconds.

### Step 2: The last-digit check

Next Theo looked only at the last digit. In 23 × 47, the last digits are 3 and 7, and 3 × 7 = 21. The last digit of the real answer has to be the last digit of 21, which is **1**.

Why? When you multiply two numbers, the tens and hundreds parts of each number only change the tens and hundreds of the answer. They cannot change the ones digit.

- Nia's 1,071 ends in 1. It passes.
- Ola's 1,801 ends in 1. It passes, though it already failed the size check.
- The third helper's 1,081 ends in 1. It passes.

Now Theo has a puzzle. Nia's 1,071 and the third helper's 1,081 both passed both checks. Only one of them can be right.

### Step 3: What a check can and cannot do

Here is the important thing about these checks.

- **When a check fails, you know the answer is wrong.** Ola's 1,801 failed the size check, so it is definitely wrong.
- **When a check passes, you do not know the answer is right.** Nia's 1,071 passed both checks, and it is wrong.

A check is like a net with holes. It catches big fish, such as the answer that is far too large or one with the wrong last digit. A small slip, like a 7 in the tens place instead of an 8, slips through the holes.

To settle it, Theo used the third kind of check: another route. This is the corner trick from The Work That Vanishes.

```
( 23 × 47 )
redraw  →  ( ( 35 − 12 ) × ( 35 + 12 ) )
redraw  →  ( ( 35 × 35 ) − ( 12 × 12 ) )
pop  →  ( 1,225 − 144 )
pop  →  1,081
```

The answer is **1,081**. Nia's 1,071 was ten short, a slip in the tens place that the two cheap checks could not see.

### What did we keep?

Write the answers in your notebook.

- **What does a failed check keep?** (Certainty: the answer is wrong.)
- **What does a passed check keep?** (Only a suspicion that the answer might be right.)

### What makes this trustworthy?

The trust comes from the reason for each check. The size check works because a number between two easy circles must lie between their values. The last-digit check works because only the ones digits can reach the ones place.

A check that you cannot give a reason for is not a check. It is a hunch. And a second route, with its own reason, is the only way to turn a "might be right" into "is right".

### Practice

1. Use the size check to say whether 38 × 52 could be **2,300**. (Use 38 × 50 and 40 × 52 as the floor and ceiling, as in chapter 2.)

2. What must the last digit of 38 × 52 be? Could the answer be **1,975**?

3. A card says 38 × 52 = **1,986**. Does it pass both checks? Is it right? (Hint: the real answer is 1,976.)

4. Find the last digit of 64 × 75. What does that tell you about **4,805**?

5. Dani says: "My answer passed both checks, so it is correct." What would you say?

### Answers

1. The floor is 38 × 50 = 1,900 and the ceiling is 40 × 52 = 2,080, so 2,300 is too big. It **cannot** be right.
2. 8 × 2 = 16, so the last digit must be **6**. The number 1,975 ends in 5, so it **cannot** be right.
3. It ends in 6, and it sits between 1,900 and 2,080, so it passes both checks. It is not right: the real answer is **1,976**.
4. 4 × 5 = 20, so the last digit is **0**. The number 4,805 ends in 5, so it is **wrong**. (The real answer is 4,800.)
5. Passing checks does not make an answer right. It only means these checks did not catch a mistake. Dani needs another route, such as a different way to split the multiplication.

---

*Bridge to the rest of the series:* In chapter 2, a bound gave a range that was certainly right. Here that range is used as a net to catch wrong answers. The second route in Step 3 is the missing-corner trick from The Work That Vanishes, used as a check. In Two Boxes, One Answer, Box P looked right too, and a missing check was the problem.
