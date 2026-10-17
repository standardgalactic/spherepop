# Two Boxes, One Answer

## Chapter 5: Boxes That Agree for a While

The detective had two new boxes. Box S squares a number: n × n. Box T doubles it: n + n.

She tested them with a card marked 2.

```
Box S:  ( 2 × 2 )  →  4
Box T:  ( 2 + 2 )  →  4
```

"Both say 4," said the detective. "The same box?"

Her helper said, "Try another card."

### Step 1: One test is not enough

She tried more cards.

| Card | Box S ( n × n ) | Box T ( n + n ) |
|---|---|---|
| 1 | 1 | 2 |
| 2 | 4 | 4 |
| 3 | 9 | 6 |
| 4 | 16 | 8 |

The boxes agree at 2 only (and at 0, if you try that). At any other card, they differ. The test with card 2 was a coincidence.

This is the detective's rule from chapter 1 in action: two boxes agreeing on the cards you tried does not mean they agree on every card.

### Step 2: A pair that really is the same

Now two other boxes. Box U squares a number: n × n. Box V does something odd: it takes one less than n, multiplies by one more than n, then adds 1.

```
Box V:  ( ( ( n − 1 ) × ( n + 1 ) ) + 1 )
```

Test them with cards 1 to 5. (The cards start at 1, so that n − 1 is never below 0.)

| Card | Box U | Box V |
|---|---|---|
| 1 | 1 | 1 |
| 2 | 4 | 4 |
| 3 | 9 | 9 |
| 4 | 16 | 16 |
| 5 | 25 | 25 |

Five cards, five agreements. Should the detective trust it now?

Not yet. But now she can look for a **reason**, and there is a good one from The Work That Vanishes. The product ( n − 1 ) × ( n + 1 ) is a square with its corner missing:

```
( ( n − 1 ) × ( n + 1 ) )  =  ( ( n × n ) − ( 1 × 1 ) )
```

So Box V is ( n × n ) minus 1, plus 1, which is just n × n again. The boxes are the same for **every** card, because the missing corner and the +1 cancel exactly.

### Step 3: Two kinds of agreement

Compare the two pairs.

- Boxes S and T agreed on one card and had **no reason** to agree anywhere else.
- Boxes U and V agreed on five cards and have a **reason** that holds for all of them.

The number of agreeing cards was not the difference. The reason was.

### What did we keep?

Write the answers in your notebook.

- **What did the agreeing cards keep?** (A list of cases that match.)
- **What did the reason keep?** (A promise about every case, including those not tried.)

### What makes this trustworthy?

A test can show two boxes are different, with certainty: one card where they disagree is enough. A test can never show two boxes are the same for all cards. Only a reason can, such as the redraw above, which keeps the value for every n.

### Practice

1. Box W triples a number: ( n + n ) + n. Box X multiplies by 3: ( 3 × n ). Do they agree for every card? Give a reason.

2. Box S squares a number and Box Y multiplies it by 4: ( 4 × n ). Find a card where they agree besides 0.

3. Box S and Box Y disagree at card 5. What do they give?

4. A detective tests two boxes on cards 1, 2, 3, 4, 5, 6, 7, 8, 9 and 10 and they always agree. Can she say they are the same? What would she need?

5. Use the missing-corner trick to explain why ( 9 × 11 ) + 1 equals ( 10 × 10 ).

### Answers

1. Yes. Three lots of n, whether written ( n + n ) + n or ( 3 × n ), are the same. The reason is that each is n taken three times.
2. Card **4** works: 4 × 4 = 16 and 4 × 4 = 16.
3. Box S gives 25, Box Y gives 20.
4. Not yet. Ten agreements are evidence, not a promise about every card. She needs a reason, such as a redraw that turns one box into the other.
5. ( 9 × 11 ) is a 10 by 10 square with a 1 by 1 corner missing, so it is 100 − 1 = 99. Adding the 1 back gives 100.

---

*Bridge to the rest of the series:* In The Order of Discovery, a pattern can hold for forty examples and then fail. Here two boxes can agree on one card and then differ, and a reason is what separates a coincidence from a rule.
