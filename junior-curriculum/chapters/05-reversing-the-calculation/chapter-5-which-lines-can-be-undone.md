# Reversing the Calculation

## Chapter 5: Which Lines Can Be Undone

Lena kept a notebook for 15 × 12. It had six lines.

```
( 15 × 12 )
redraw  →  ( ( 15 × 10 ) + ( 15 × 2 ) )
pop ( 15 × 10 )  →  150
pop ( 15 × 2 )  →  30
( 150 + 30 )
pop  →  180
```

Omar covered up everything except the last line: **180**. "Can you tell me what the problem was?" he asked.

Lena could not. Many problems give 180: 18 × 10, 20 × 9, 90 × 2, 100 + 80. The bare answer had forgotten the road.

### Step 1: Undo with the notebook

Omar uncovered the lines, one by one, from the bottom.

Lena could undo each line by looking at what was written.

- Last line, *pop → 180*: the line above it says what was popped, ( 150 + 30 ). So 180 goes back to ( 150 + 30 ).
- Before that, *pop ( 15 × 2 ) → 30*: the line records ( 15 × 2 ). So 30 goes back to ( 15 × 2 ).
- Then *pop ( 15 × 10 ) → 150*, which goes back to ( 15 × 10 ).
- Then the redraw. A redraw keeps the value, so undoing it means redrawing the other way: ( ( 15 × 10 ) + ( 15 × 2 ) ) back to ( 15 × 12 ).

She walked all the way back to the start.

### Step 2: Which lines can be undone without help?

Lena sorted the lines into two kinds.

- **A redraw can always be undone**, even without a record. It kept the value and only rearranged the circles, so you can rearrange back. Nothing was forgotten.
- **A pop cannot be undone without a record.** The pop *pop ( 15 × 10 ) → 150* replaced a circle with its value. If you only see 150, you cannot tell it came from 15 × 10 and not from 30 × 5 or 75 × 2.

So the lines that need the notebook are the pops. The notebook records what was inside each circle, so the pop can be read backward.

```
150  could be:  ( 15 × 10 )   ( 30 × 5 )   ( 75 × 2 )   ( 100 + 50 )
```

### Step 3: A line that changes nothing

Suppose the notebook also had a refusal line, like Sade's in The Boundary of a Step.

```
refuse ( 15 × 11 )  →  not allowed: the problem asks for exactly 15 × 12
```

Nothing was changed by a refusal, so there is nothing to undo. But the line still says something: it tells the reader why a move was not taken.

So three kinds of lines behave differently.

| Line | Can it be undone? | Why |
|---|---|---|
| redraw | yes, always | the value was kept |
| pop | only with the record | the circle's insides were forgotten |
| refusal | nothing to undo | no change was made |

### What did we keep?

Write the answers in your notebook.

- **What does the notebook keep that the answer 180 does not?** (What each pop replaced.)
- **Which line forgets the most?** (A pop.)

### What makes this trustworthy?

A notebook is a trustworthy way to go backward when every pop line records what was popped. If a line only says "→ 150" without saying what was inside the circle, that pop cannot be undone, and neither can anything after it.

### Practice

1. Redraw ( 15 × 12 ) into ( ( 15 × 10 ) + ( 15 × 2 ) ). Show the redraw that undoes it.

2. Give three different circles that pop to 24.

3. A notebook line says only "→ 24". Can you undo it? What would you need?

4. Which kind of line is easy to undo even if you lose the notebook: a redraw or a pop? Explain.

5. Omar hides the notebook and says the final answer is 180. Give two problems that fit.

### Answers

1. The redraw back is ( ( 15 × 10 ) + ( 15 × 2 ) ) → ( 15 × 12 ). It collects the two lots into one.
2. For example ( 4 × 6 ), ( 3 × 8 ), ( 12 + 12 ). Others work too.
3. No. You would need the line that says what was popped, such as ( 4 × 6 ) → 24.
4. A **redraw**. It only rearranged the circles and kept the value, so rearranging back always works. A pop forgot what was inside.
5. For example ( 15 × 12 ) and ( 20 × 9 ). Any circle that pops to 180 fits.

---

*Bridge to the rest of the series:* In chapter 4, a button on the clock could be undone when each answer came from one start. Here the same question is asked of lines in a notebook. In What Must Remain Unresolved, the notebook is also the thing that keeps the note to self faithful.
