# Two Boxes, One Answer

## Chapter 1: The Detective and the Boxes

> **Circle reminder.** A circle holds a piece that belongs together, and circles can sit inside circles. A circle is *ready* when everything needed for its calculation is known and its operation is allowed by the current rules. To **pop** a ready circle, replace it with its value and leave everything outside it alone. To **redraw**, move circles around or split a number, as long as the value stays the same. The full rules are in the introduction.

Imagine two plain gray boxes on a table. Each one has a slot on top and a slot on the side. You write a number on a card and drop it in the top, and a moment later a card with a new number slides out of the side.

You cannot open the boxes. You can only feed in numbers and read what comes out. Your job is to work out whether the two boxes are secretly the same.

Here are the rules of the game. Each box follows one fixed rule that never changes. You may drop in any whole number from 0 upward. And you judge a box only by the card that slides out: sounds, smells and how long it takes do not count.

Detectives call this kind of job a *test*. Today you are the detective.

### Step 1: The first test

Drop a 3 into Box A. Out comes a 6.
Drop a 3 into Box B. Out comes a 6.

Same answer. Try again.

| Put in | Box A gives | Box B gives |
|---|---|---|
| 1 | 2 | 2 |
| 2 | 4 | 4 |
| 5 | 10 | 10 |
| 7 | 14 | 14 |
| 12 | 24 | 24 |

Every time, the boxes agree. So are they the same?

Here is the first thing a good detective learns: **agreeing so far is not the same as agreeing forever.** You have only tried five numbers, and there are far more numbers than that.

### Step 2: Looking for the difference

So you keep hunting. You try a big number: 1,000 gives 2,000 in both boxes. You try 0, and both give 0. You try a number you made up that nobody would think of, and the boxes still match.

At some point you start to wonder whether any number can split them.

Here is a secret. Suppose someone tells you the inside of each box.

- Box A takes your number and *multiplies it by 2*.
- Box B takes your number and *adds it to itself*.

Now think about what those two jobs mean. Multiplying a number by 2 means taking two copies of it. Adding a number to itself also means taking two copies of it. They are the same job described in two different ways. Whatever number you put in, there will never be a difference.

So no detective, however clever, can ever tell Box A from Box B using only the slots. The boxes are *different on the inside* but *the same on the outside*. And the outside is all a detective can see.

### In circles

Think of each box as a circle you cannot open. All you ever see is the number it pops to.

```
Box A:  ( 3 × 2 )  →  6
Box B:  ( 3 + 3 )  →  6
```

From outside you see two 6s. Inside, the circles are different. Yet no number can tell the boxes apart, and here is the reason: for *every* n, you can redraw ( n + n ) as ( n × 2 ) without changing the value, because two copies of n is two copies of n. A redraw that always keeps the value is a reason. A pile of matching pops is only evidence.

### Step 3: The boxes that fooled you

Now meet two new boxes, Box C and Box D. Try them out.

| Put in | Box C gives | Box D gives |
|---|---|---|
| 1 | 1 | 1 |
| 2 | 2 | 2 |
| 3 | 3 | 3 |
| 4 | 4 | 4 |
| 5 | 5 | 5 |
| 6 | 6 | 6 |
| 7 | 7 | 7 |
| 8 | 8 | 8 |
| 9 | 9 | 9 |

Nine tests in a row, and the boxes matched every time. You are feeling very sure of yourself.

Now try 10.

Box C gives 10. Box D gives 1.

The boxes are different after all!

What was going on inside? Box C simply hands your number back. Box D *adds up the digits* of your number. For a number like 7, there is only one digit, so the sum is 7. But 10 has two digits, 1 and 0, and 1 plus 0 is 1.

Until you tried a two-digit number, there was no way to see the difference. Nine matching tests were not enough.

### The detective's rule

Write this one on the inside cover of your notebook:

**One disagreement proves two boxes are different. When there is no end to the numbers you can drop in, no pile of agreements by itself can prove two boxes are the same.**

Why? If a box only accepted the numbers 1 to 10, you could test all ten and be finished. These boxes accept any whole number, so you can never test them all. A single test where the boxes disagree is a fact you can point to. Agreements only tell you that you have not found a difference yet. Maybe there is one hiding at 10, or at 1,000, or at a number you did not think to try.

So when do agreements become a proof? When you can explain *why* the boxes must always agree, the way we did for Box A and Box B. A reason covers every number at once, including all the ones you never tested.

### What did we keep?

Write the answers in your notebook.

- **What can the detective see?** (Only the numbers going in and coming out.)
- **What did the detective not see?** (How each box works inside. Box A and Box B work differently, but nobody could tell.)
- **If two boxes always give the same answer, are they the same box?** (They are the same *to anyone using the slots*. They might be built differently on the inside.)

### What makes this conclusion trustworthy?

Two different kinds of statements showed up in this chapter. Which is which?

1. "Boxes C and D matched on 1 through 9, so they might be the same."
2. "Boxes A and B must always match, because multiplying by 2 and adding a number to itself are both just two copies."

(Answers: 1 is *evidence*. It is useful, but it can be overturned by one new test. 2 is a *reason*. It covers every number at once.)

### Practice

1. Box E adds 1 to your number, then subtracts 1. Box F just hands back your number. Can any input tell them apart? How do you know?

2. Box G multiplies your number by itself. Box H adds your number to itself. Both give 4 when you put in 2. Is that enough to say they are the same? Find an input that splits them.

3. Box I multiplies your number by 5. Box J multiplies your number by 10, then cuts the result in half. Will any number ever tell them apart? Explain your reason in one sentence.

4. **The sneaky box.** Box K adds 1 to your number. Box L also adds 1, except that when you put in exactly 1,000, it gives back 0. You test both boxes on every number from 1 to 999 and they always match. Is it fair to say they are the same? What would you need to be sure?

5. Box M pops ( n × 4 ). Box N pops ( ( n + n ) + ( n + n ) ). Can any n tell them apart? Use a redraw to explain.

### Answers

1. No input can tell them apart. Adding 1 and then subtracting 1 puts you right back where you started, whatever number you used. That is a reason, not just a pile of tests.
2. Matching on 2 is not enough. Put in 3: Box G gives 9 and Box H gives 6, so they are different. (The only numbers where they agree are 0 and 2, which is why 2 was a trap.)
3. No. Ten copies of a number cut in half is five copies of it. That works for every number.
4. No, it is not fair. Nine hundred and ninety-nine matching tests still leave 1,000 untested, and 1,000 is where they split. To be sure, you would need either to test 1,000 or to see inside the boxes. Because these boxes accept any whole number, testing alone can never cover them all, so a reason about how the boxes are built is the way to be completely sure.
5. No n can tell them apart. ( ( n + n ) + ( n + n ) ) is four copies of n, so it redraws as ( n × 4 ) for every n, and the value never changes.

---

*Bridge to the rest of the series:* The boxes in this chapter are the first example of a question that comes back in every book: what can someone on the outside tell, and what stays hidden? In The Work That Vanishes, 8 times 12 and 10 times 10 minus 2 times 2 are two different expressions that give the same answer, and the cut-and-slide picture is the reason. In A Useful Wrong Answer, a guess and the exact value can look identical until you check at a distance.
