# A Useful Wrong Answer

## Chapter 7: Counting Digits

Theo typed 23 × 47 into a calculator and the screen showed **10.81**. Then he typed it again and got **10,810**. He pressed the buttons one more time and got **1,081**.

Three answers, three different sizes. Two of them have the digits right and the decimal point or a zero wrong. Which one is correct, and how could he tell at a glance?

### Step 1: How many digits should the answer have?

Theo knows that the smallest two-digit number is 10 and the largest is 99.

```
( 10 × 10 )  →  100       (3 digits)
( 99 × 99 )  →  9,801     (4 digits)
```

So any two-digit number times a two-digit number lies between 100 and 9,801. That means the answer has **3 or 4 digits**, never 2 and never 5.

- 10.81 is not even a whole number. It cannot be right.
- 10,810 has 5 digits. It is too big. It cannot be right.
- 1,081 has 4 digits. It is possible.

The digit count threw out two of the three answers without any multiplying.

### Step 2: Narrow it with the floor and ceiling

Counting digits is a rough net. To narrow further, Theo uses the floor and ceiling from chapter 3.

```
( 20 × 40 )  →  800
( 25 × 50 )  →  1,250
```

So 23 × 47 is between 800 and 1,250. The answer 1,081 sits comfortably inside.

The digit count alone could not decide between 3 and 4 digits here, since 800 has 3 digits and 1,250 has 4. The floor and ceiling made it clear.

### Step 3: Digits for other sizes

The same idea works for any sizes.

| Multiplying | Smallest product | Largest product | Digits in the answer |
|---|---|---|---|
| 2-digit × 2-digit | 100 | 9,801 | 3 or 4 |
| 3-digit × 2-digit | 1,000 | 98,901 | 4 or 5 |
| 2-digit × 1-digit | 10 | 891 | 2 or 3 |

For the second row, the smallest is 100 × 10 and the largest is 999 × 99. For the third, the smallest is 10 × 1 and the largest is 99 × 9.

In the last row, the smallest product is 10 × 1 = 10, which has 2 digits, so the answer has 2 or 3 digits.

### What did we keep?

Write the answers in your notebook.

- **What does the digit count keep?** (A rough size: how many places the answer must have.)
- **What does it lose?** (The exact value, and even the exact size when the range crosses a place.)

### What makes this trustworthy?

The digit count comes from the smallest and largest possible products, and these are exact. It is a strong test for slips with decimal points and extra zeros, which are among the most common slips. It is a weak test for a slip in a middle digit, so, as always, a passed check is not a proof.

### Practice

1. A calculator shows 2,496, 249.6 and 24,960 for 48 × 52. Which could be right? Why?

2. How many digits can 12 × 13 have? Which of 15.6, 156 and 1,560 could be right?

3. How many digits can 305 × 12 have? (Use 300 × 10 and 310 × 12 or similar.)

4. A shop sells 99 packs of 99 sweets each. Could the total have 3 digits, such as 981? Is 9,801 sensible?

5. Dani says "I counted the digits and they match, so my answer is right." What would you say?

### Answers

1. Only **2,496** has 4 digits and is a whole number in the range 100 to 9,801. 249.6 is not whole and 24,960 has 5 digits.
2. 12 × 13 is between 100 and 9,801, so 3 or 4 digits. **156** is the only one of the three that fits (and 12 × 13 is exactly 156).
3. 305 × 12 is more than 300 × 10 = 3,000 and less than 310 × 12 = 3,720, so it has **4 digits**.
4. No. A 2-digit by 2-digit product has 3 digits only when it is below 1,000, and 99 × 99 is far above that, so 981 is too small. **9,801** is sensible, because 99 × 99 is 9,801.
5. Matching digits only catches slips in the size. A slip in a middle digit has the same number of digits. She needs another check.

---

*Bridge to the rest of the series:* In chapter 3, the floor and ceiling were a net for wrong answers. A digit count is an even rougher net, and it catches the slips that nets with finer holes can miss. In The Choice Before the Calculation, choosing the unit well makes decimal-point slips less likely.

*Next in this book:* Chapter 8: The Estimate That Owes Four.
