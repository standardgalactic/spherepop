# Two Boxes, One Answer

## Chapter 8: Boxes in a Row

The detective put two boxes in a row so that the output of the first went straight into the second.

- Box A adds 3 to a card.
- Box B doubles a card.

She tried two orders.

### Step 1: A then B, and B then A

**A first, then B.** Add 3, then double.

```
card 5:  ( 5 + 3 )  →  8,   ( 8 × 2 )  →  16
```

**B first, then A.** Double, then add 3.

```
card 5:  ( 5 × 2 )  →  10,   ( 10 + 3 )  →  13
```

The same boxes, in a different order, give different answers: 16 against 13. The order of boxes matters.

### Step 2: Why they differ

With a number n on the card, the circles show it.

```
A then B:   ( ( n + 3 ) × 2 )   →   ( ( n × 2 ) + 6 )
B then A:   ( ( n × 2 ) + 3 )
```

The first adds 6 and the second adds 3. They differ by 3 for every card. So these two rows never agree, however many cards you try.

### Step 3: Rows that do not care about order

Now Box C, which adds 4. Try A then C and C then A.

```
A then C:   ( ( n + 3 ) + 4 )   →   ( n + 7 )
C then A:   ( ( n + 4 ) + 3 )   →   ( n + 7 )
```

Same answer, either way. Adding 3 and adding 4 do not interfere with each other.

Another pair: Box B doubles, Box D triples.

```
B then D:   ( ( n × 2 ) × 3 )   →   ( n × 6 )
D then B:   ( ( n × 3 ) × 2 )   →   ( n × 6 )
```

Same again. Doubling and tripling do not interfere.

So some boxes can swap places and some cannot. Adding and doubling cannot swap. Adding and adding can. Doubling and tripling can.

### Step 4: Two different rows that agree

Here is an unexpected pair. Compare:

- **Row 1:** add 3, then double.
- **Row 2:** double, then add 6.

```
Row 1:   ( ( n + 3 ) × 2 )   →   ( ( n × 2 ) + 6 )
Row 2:   ( ( n × 2 ) + 6 )
```

They give the same answer for every card, though they are built differently. Card 5: Row 1 gives 16 and Row 2 gives ( 5 × 2 ) + 6 = 16. The reason is a redraw: sharing the 2 with both parts of ( n + 3 ).

### What did we keep?

Write the answers in your notebook.

- **What did swapping the boxes change?** (The answer, when the boxes interfere with each other.)
- **What did the redraw show?** (Why two different rows can still be the same.)

### What makes this trustworthy?

The claim "these rows are the same" rests on the redraws, which hold for every card. The claim "these rows are different" rests on a single card where the answers disagree. As in earlier chapters, one case can break a claim, but only a reason can prove it.

### Practice

1. Box E adds 5 and Box F adds 8. Does the order matter? Show with circles.

2. Box G doubles and Box H subtracts 2. Try both orders on the card 7.

3. Is "subtract 2, then double" the same as "double, then subtract 4"? Give a reason.

4. Box J halves a card and Box K adds 1. Try both orders on the card 10.

5. Name two boxes whose order never matters.

### Answers

1. ( ( n + 5 ) + 8 ) and ( ( n + 8 ) + 5 ) both come to ( n + 13 ), so the order does not matter.
2. Double then subtract 2: 14 − 2 = **12**. Subtract 2 then double: 5 × 2 = **10**. The order matters.
3. Yes. ( ( n − 2 ) × 2 ) redraws to ( ( n × 2 ) − 4 ). The 2 is shared with both parts.
4. Halve then add 1: 5 + 1 = **6**. Add 1 then halve: 11 ÷ 2 = **5.5**. The order matters.
5. For example "add 3" and "add 4", or "double" and "triple".

---

*Bridge to the rest of the series:* In chapter 4, we saw that an order window can tell boxes apart. Here the order of the boxes themselves is what changes the answer. In Reversing the Calculation, undoing a chain of buttons in the opposite order is why socks and shoes come off in reverse.

*Next in this book:* Chapter 9: The Answer on the Fridge.
