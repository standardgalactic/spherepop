# Two Boxes, One Answer

## Chapter 6: The Box With a Secret Exception

The detective was given two new boxes. Box A doubles a card: ( n × 2 ). Box B looks like it does the same.

She tested six cards.

| Card | Box A | Box B |
|---|---|---|
| 1 | 2 | 2 |
| 2 | 4 | 4 |
| 3 | 6 | 6 |
| 4 | 8 | 8 |
| 5 | 10 | 10 |
| 6 | 12 | 12 |

Six agreements. The detective was about to write "same box". Then her helper opened a small window in Box B.

### Step 1: The rule inside

Through the window, she could read Box B's rule:

```
if the card is 7:   give 0
otherwise:          ( n × 2 )
```

Box B has a **secret exception** at card 7. At every other card it matches Box A, but not there:

```
Box A at 7:  ( 7 × 2 )  →  14
Box B at 7:  0
```

The detective tested cards 1 to 6 and never touched card 7. No amount of testing the same kind of cards would have found it, unless she happened to try 7.

### Step 2: What testing can and cannot do

- If the detective tries a card where the boxes differ, she **knows** they are different.
- If she never tries that card, she learns nothing about it.

The exception was a single card out of endlessly many. To find it by testing alone, she would have to be lucky, or try every card.

If the cards were only the numbers 1 to 10, she could try all ten. That is a complete test, and then agreement would really mean "same box". With endlessly many cards, no list of tests can ever be complete.

### Step 3: The window is the answer

The window did what testing could not. It showed **how the box is built**, so the detective could read the exception directly.

This is the idea from chapter 4: what you can see decides which boxes look the same. With a window on the rule, Box B and Box A look different at once. With only the slot, they look the same on six cards.

A reason about how the boxes are built covers every card at once, including the ones nobody tries. That is why chapter 5 looked for a reason, not more tests.

### What did we keep?

Write the answers in your notebook.

- **What did the six tests keep?** (Six matching cards.)
- **What did they miss?** (The exception at card 7.)

### What makes this trustworthy?

Looking inside is trustworthy when the window shows the real rule. A claim like "the boxes are the same" can rest on a reason about their rules, or on a complete test of every card. It cannot rest on a partial test.

### Practice

1. Box C doubles every card, except it gives 100 for card 3. Box D doubles every card. At which card do they differ? What does each give?

2. The detective tests Box C and Box D on cards 1, 2, 4 and 5. What does she see? Is she right to say they are the same?

3. The cards are only 1, 2, 3 and 4. How many tests settle whether Box C and Box D agree on every card?

4. Why does a window help more than more tests?

5. Box E gives ( n × 2 ) for every card except 1,000,000, where it gives 0. About how many cards would you test before you found the exception, if you tried 1, 2, 3 and so on?

### Answers

1. They differ at card **3**. Box D gives 6 and Box C gives 100.
2. She sees 2, 4, 8 and 10 from both boxes. She is not right to say they are the same, because she skipped card 3.
3. Four tests, one for each card. Then agreement would mean the boxes are really the same on these cards.
4. A window shows the rule for every card at once. Each test only shows one card.
5. About **a million**. You would have to reach card 1,000,000 before you saw the difference.

---

*Bridge to the rest of the series:* In The Order of Discovery, a pattern held for forty examples and then failed. Here the failure is hidden at one card. In Reversing the Calculation, a trace is a window onto the steps, and it shows what a bare answer cannot.
