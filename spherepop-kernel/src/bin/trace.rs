//! Worked trace matching the specification's kernel section: introduce two
//! objects, bind them, observe under a quotienting rule (this IS Merge),
//! then withdraw reliance on the bind without deleting it (Unlink-sugar).

use spherepop_kernel::{collapse, sugar, Arbiter, Event, Proposal};

fn main() {
    let omega_0 = [1u64, 2];
    let mut arb = Arbiter::new(omega_0, ["merge_quotient", "identity"]);

    arb.submit(Proposal::new(vec![Event::pop(1), Event::pop(2)]))
        .expect("commit a, b");
    println!("After Pop(1), Pop(2): {:?}", arb.state());

    arb.submit(Proposal::new(sugar::merge(1, 2, "merge_quotient")))
        .expect("Bind(1,2) + Collapse(merge_quotient)");
    let mut classes = collapse::collapse_quotient(arb.history_ref());
    println!(
        "Merge via quotient collapse: same_class(1,2) = {}",
        classes.same_class(1, 2)
    );

    arb.submit(Proposal::new(vec![sugar::unlink(1, 2)]))
        .expect("Refuse(Bind(1,2)) — relation withdrawn");
    let s = arb.state();
    println!(
        "After Unlink: bind still present = {}, refusal recorded = {}",
        s.bound.contains(&(1, 2, "merge".to_string())),
        s.refused.iter().any(|(_, _, r)| r == "relation withdrawn")
    );

    println!("\nFull history ({} events):", arb.len());
    for e in arb.history_ref().as_slice() {
        println!("  {:?}", e);
    }
}
