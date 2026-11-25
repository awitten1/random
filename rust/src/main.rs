//! A tour of Rust ownership for C++ programmers.
//!
//! Build and run:   rustc --edition 2024 ownership_tour.rs && ./ownership_tour
//!
//! Lines starting with `//✗` are deliberate compile errors. Uncomment one at a
//! time to see what the compiler says; the expected error code is noted on each.

use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::{Rc, Weak};
use std::sync::{Arc, Mutex};
use std::thread;

/// Prints when it's destroyed, so you can watch drops happen.
struct Noisy(&'static str);

impl Drop for Noisy {
    fn drop(&mut self) {
        println!("    drop({})", self.0);
    }
}

fn main() {
    section("1. Moves, Copy, Clone");
    moves_copy_clone();

    section("2. Drop (destructors) and drop order");
    drop_order();

    section("3. Ownership across function calls");
    functions_and_ownership();

    section("4. Borrowing: shared XOR mutable");
    borrowing_rules();

    section("5. Lifetimes");
    lifetimes();

    section("6. Box<T>  (~ unique_ptr)");
    boxes();

    section("7. Rc / Weak / RefCell  (~ shared_ptr / weak_ptr)");
    rc_weak_refcell();

    section("8. Arc / Mutex and threads");
    threads();

    section("9. Moving out of something you only borrow");
    moving_out();

    section("10. Indices instead of Rc<RefCell<T>>");
    indices_instead_of_rc();

    println!("\nDone.");
}

fn section(title: &str) {
    println!("\n=== {title} ===");
}

// ---------------------------------------------------------------------------
// 1. Moves, Copy, Clone
// ---------------------------------------------------------------------------

fn moves_copy_clone() {
    // Non-Copy types move on assignment. Like `auto b = std::move(a);`, except
    // `a` is statically dead afterwards rather than "valid but unspecified".
    let a = String::from("hello");
    let b = a;
    //✗ println!("{a}"); // E0382: borrow of moved value: `a`
    println!("  b = {b}");

    // Copy types (integers, floats, bool, char, &T, and tuples/arrays of Copy
    // types) are copied implicitly, like trivially copyable types in C++.
    let x = 42;
    let y = x;
    println!("  x = {x}, y = {y}");

    // A deep copy is always explicit: .clone() is the copy constructor.
    let c = b.clone();
    println!("  b = {b}, c = {c}");

    // Your own types can be Copy if every field is Copy.
    #[allow(dead_code)] // fields are only read via Debug
    #[derive(Clone, Copy, Debug)]
    struct Point {
        x: i32,
        y: i32,
    }
    let p = Point { x: 1, y: 2 };
    let q = p; // copy, not move
    println!("  p = {p:?}, q = {q:?}");
}

// ---------------------------------------------------------------------------
// 2. Drop
// ---------------------------------------------------------------------------

fn drop_order() {
    let _a = Noisy("a");
    let _b = Noisy("b");

    {
        let _inner = Noisy("inner");
        println!("  leaving inner scope");
    } // inner dropped here

    // Gotcha: `let _ = ...` does not bind, so the value is dropped immediately.
    // (`let _name = ...` does bind and lives to the end of the scope.)
    let _ = Noisy("underscore");
    println!("  after `let _ =`");

    // A moved-from value is not dropped by the original owner. No double free.
    let moved = Noisy("moved");
    take(moved); // dropped at the end of take(), not here
    //✗ let _again = moved; // E0382: use of moved value: `moved`

    // A conditional move is fine: the compiler adds a hidden "drop flag".
    let maybe = Noisy("maybe");
    if std::env::args().count() > 99 {
        take(maybe);
    }

    println!("  leaving drop_order (expect reverse order: maybe, b, a)");
}

fn take(n: Noisy) {
    println!("  take() now owns {}", n.0);
}

// ---------------------------------------------------------------------------
// 3. Functions
// ---------------------------------------------------------------------------

fn functions_and_ownership() {
    let s = String::from("data");

    // Shared borrow: the caller keeps ownership. ~ `const std::string&`
    // (&String converts to &str automatically.)
    let len = borrow_len(&s);
    println!("  len = {len}, s still usable: {s}");

    // Move in, move back out. ~ taking `std::string` by value and returning it.
    let s = consume_and_return(s);
    println!("  got back: {s}");

    // Exclusive borrow: the callee may mutate. ~ `std::string&`
    let mut s = s;
    append(&mut s);
    println!("  after append: {s}");
}

fn borrow_len(s: &str) -> usize {
    s.len()
}

fn consume_and_return(mut s: String) -> String {
    s.push('!');
    s
}

fn append(s: &mut String) {
    s.push_str(" more");
}

// ---------------------------------------------------------------------------
// 4. Borrowing rules
// ---------------------------------------------------------------------------

fn borrowing_rules() {
    let mut v = vec![1, 2, 3];

    // Any number of shared borrows at once.
    let r1 = &v;
    let r2 = &v;
    println!("  r1 = {r1:?}, r2 = {r2:?}");

    // The classic C++ bug: keeping a reference across a push_back that reallocates.
    let first = &v[0];
    //✗ v.push(4); // E0502: cannot borrow `v` as mutable because it is also borrowed as immutable
    println!("  first = {first}");
    v.push(4); // fine: a borrow ends at its last use, and `first` isn't used again

    // Only one &mut at a time.
    let m1 = &mut v;
    //✗ let m2 = &mut v; // E0499: cannot borrow `v` as mutable more than once at a time
    m1.push(5);
    println!("  v = {v:?}");

    // Changing a container while iterating over it: a compile error, not UB.
    //✗ for x in &v { if *x == 2 { v.push(99); } } // E0502
    // Typical fix: work out the changes first, then apply them.
    let extra: Vec<i32> = v.iter().filter(|&&x| x % 2 == 0).map(|x| x * 10).collect();
    v.extend(extra);
    println!("  v = {v:?}");

    // Non-overlapping parts can be borrowed mutably at the same time.
    let (left, right) = v.split_at_mut(3);
    left[0] += right[0];
    println!("  after split_at_mut: {v:?}");
}

// ---------------------------------------------------------------------------
// 5. Lifetimes
// ---------------------------------------------------------------------------

// The result borrows from `a` or `b`, so it's valid only while both are alive.
fn longest<'a>(a: &'a str, b: &'a str) -> &'a str {
    if a.len() >= b.len() { a } else { b }
}

// A struct holding a reference has a lifetime parameter: it can't outlive `text`.
struct Parser<'a> {
    text: &'a str,
    pos: usize,
}

impl<'a> Parser<'a> {
    fn new(text: &'a str) -> Self {
        Parser { text, pos: 0 }
    }

    // Returns `&'a str`, which borrows from the original text, not from the
    // Parser. So the words stay valid after the Parser is gone.
    fn next_word(&mut self) -> Option<&'a str> {
        let rest = &self.text[self.pos..];
        let trimmed = rest.trim_start();
        if trimmed.is_empty() {
            return None;
        }
        let start = self.pos + (rest.len() - trimmed.len());
        let end = trimmed.find(' ').map_or(self.text.len(), |i| start + i);
        self.pos = end;
        Some(&self.text[start..end])
    }
}

fn lifetimes() {
    // A dangling reference won't compile.
    //✗ let r; { let x = 5; r = &x; } println!("{r}"); // E0597: `x` does not live long enough

    let a = String::from("short");
    let result;
    {
        let b = String::from("much longer");
        result = longest(&a, &b);
        println!("  longest = {result}");
    }
    //✗ println!("{result}"); // E0597: `b` does not live long enough

    let text = String::from("the quick brown fox");
    let words: Vec<&str> = {
        let mut p = Parser::new(&text);
        let mut w = Vec::new();
        while let Some(word) = p.next_word() {
            w.push(word);
        }
        w
    }; // the Parser is gone; the words borrow from `text`, which is still alive
    println!("  words = {words:?}");
    //✗ drop(text); println!("{words:?}"); // E0505: cannot move out of `text` because it is borrowed
}

// ---------------------------------------------------------------------------
// 6. Box
// ---------------------------------------------------------------------------

// A recursive type needs indirection, like unique_ptr in a linked-list node.
#[allow(dead_code)] // fields are only read via Debug
#[derive(Debug)]
enum List {
    Cons(i32, Box<List>),
    Nil,
}

trait Shape {
    fn area(&self) -> f64;
}
struct Circle(f64);
struct Square(f64);
impl Shape for Circle {
    fn area(&self) -> f64 {
        std::f64::consts::PI * self.0 * self.0
    }
}
impl Shape for Square {
    fn area(&self) -> f64 {
        self.0 * self.0
    }
}

fn boxes() {
    let list = List::Cons(1, Box::new(List::Cons(2, Box::new(List::Nil))));
    println!("  list = {list:?}");

    // Box<dyn Trait> ~ std::unique_ptr<Base> with virtual functions.
    let shapes: Vec<Box<dyn Shape>> = vec![Box::new(Circle(1.0)), Box::new(Square(2.0))];
    let total: f64 = shapes.iter().map(|s| s.area()).sum();
    println!("  total area = {total:.2}");

    let b = Box::new(Noisy("boxed"));
    let b2 = b; // moves the pointer; the heap object stays put
    //✗ println!("{}", b.0); // E0382: borrow of moved value: `b`
    println!("  b2 holds {}", b2.0);
} // "boxed" dropped here

// ---------------------------------------------------------------------------
// 7. Rc / Weak / RefCell
// ---------------------------------------------------------------------------

struct Node {
    name: String,
    parent: RefCell<Weak<Node>>,      // back-pointer is Weak, so no leaking cycle
    children: RefCell<Vec<Rc<Node>>>, // RefCell lets us mutate through a shared Rc
}

impl Node {
    fn new(name: &str) -> Rc<Node> {
        Rc::new(Node {
            name: name.to_string(),
            parent: RefCell::new(Weak::new()),
            children: RefCell::new(Vec::new()),
        })
    }
}

impl Drop for Node {
    fn drop(&mut self) {
        println!("    drop(Node {})", self.name);
    }
}

fn rc_weak_refcell() {
    // Shared ownership: freed when the last owner goes away.
    let config = Rc::new(Noisy("config"));
    let owner_a = Rc::clone(&config); // refcount bump, not a deep copy
    let owner_b = Rc::clone(&config);
    println!("  strong count = {}", Rc::strong_count(&config));
    drop(config);
    drop(owner_a);
    println!("  still alive through owner_b: {}", owner_b.0);
    drop(owner_b); // count reaches 0, so "config" is dropped now

    // Rc gives you only &T. To mutate, you need interior mutability.
    let counter = Rc::new(RefCell::new(0));
    let c2 = Rc::clone(&counter);
    *c2.borrow_mut() += 1;
    *counter.borrow_mut() += 1;
    println!("  counter = {}", counter.borrow());

    // RefCell checks the borrow rules at runtime instead of compile time.
    let reading = counter.borrow();
    if counter.try_borrow_mut().is_err() {
        println!("  try_borrow_mut failed: already borrowed (borrow_mut() would panic)");
    }
    drop(reading);

    // A tree with parent pointers: children are owned (Rc), parents aren't (Weak).
    let root = Node::new("root");
    let leaf = Node::new("leaf");
    *leaf.parent.borrow_mut() = Rc::downgrade(&root);
    root.children.borrow_mut().push(Rc::clone(&leaf));

    let parent_name = leaf.parent.borrow().upgrade().map(|p| p.name.clone());
    println!("  leaf's parent = {parent_name:?}");
    println!(
        "  root: strong={}, weak={};  leaf: strong={}",
        Rc::strong_count(&root),
        Rc::weak_count(&root),
        Rc::strong_count(&leaf)
    );

    drop(root); // root's only strong owner is gone, so root is freed
    let parent_name = leaf.parent.borrow().upgrade().map(|p| p.name.clone());
    println!("  after dropping root, leaf's parent = {parent_name:?}");

    // Rc's refcount isn't atomic, so the compiler won't let it cross threads.
    //✗ thread::spawn(move || println!("{}", leaf.name)); // E0277: `Rc<Node>` cannot be sent between threads safely
} // leaf dropped here

// ---------------------------------------------------------------------------
// 8. Threads
// ---------------------------------------------------------------------------

fn threads() {
    // Arc = atomic Rc; Mutex owns the data it protects. You can't reach the
    // HashMap without locking, and the lock is released when the guard drops.
    let counts = Arc::new(Mutex::new(HashMap::new()));
    let words = vec!["apple", "banana", "apple", "cherry", "banana", "apple"];

    let handles: Vec<_> = words
        .chunks(2)
        .map(|chunk| {
            let counts = Arc::clone(&counts);
            let chunk = chunk.to_vec();
            thread::spawn(move || {
                for w in chunk {
                    *counts.lock().unwrap().entry(w).or_insert(0) += 1;
                }
            })
        })
        .collect();
    for h in handles {
        h.join().unwrap();
    }

    let mut result: Vec<(&str, i32)> =
        counts.lock().unwrap().iter().map(|(k, v)| (*k, *v)).collect();
    result.sort();
    println!("  word counts = {result:?}");

    // A plain thread::spawn may outlive this function, so it can't borrow locals.
    //✗ let local = vec![1]; thread::spawn(|| println!("{local:?}")); // E0373: closure may outlive the current function

    // Scoped threads must finish before the scope ends, so they *can* borrow.
    let data = vec![1, 2, 3, 4];
    let mut sums = [0; 2];
    thread::scope(|s| {
        for (half, out) in data.chunks(2).zip(sums.iter_mut()) {
            s.spawn(move || *out = half.iter().sum());
        }
    });
    println!("  scoped-thread sums = {sums:?}");
}

// ---------------------------------------------------------------------------
// 9. Moving out of borrowed places
// ---------------------------------------------------------------------------

fn moving_out() {
    let mut names = vec![String::from("a"), String::from("b"), String::from("c")];

    // Moving out of an element would leave a hole in the Vec.
    //✗ let first = names[0]; // E0507: cannot move out of index of `Vec<String>`

    // So you swap something else in:
    let first = std::mem::take(&mut names[0]); // leaves String::default() ("")
    let second = std::mem::replace(&mut names[1], String::from("z"));
    println!("  first = {first}, second = {second}, names = {names:?}");

    // Or remove it from the container:
    let last = names.pop(); // Option<String>
    println!("  popped = {last:?}, names = {names:?}");

    // Option::take() is the idiom for moving out of a field behind &mut.
    struct Holder {
        item: Option<Noisy>,
    }
    let mut h = Holder { item: Some(Noisy("held")) };
    let taken = h.item.take();
    println!(
        "  took {:?}; holder now empty: {}",
        taken.as_ref().map(|n| n.0),
        h.item.is_none()
    );
} // "held" dropped here (owned by `taken`)

// ---------------------------------------------------------------------------
// 10. Indices instead of Rc<RefCell<T>>
// ---------------------------------------------------------------------------

// One Vec owns every node, and nodes refer to each other by index. No refcounts,
// no RefCell, cycles are harmless, and mutation is simple because there's one owner.
struct Graph {
    names: Vec<String>,
    edges: Vec<Vec<usize>>,
}

impl Graph {
    fn add(&mut self, name: &str) -> usize {
        self.names.push(name.to_string());
        self.edges.push(Vec::new());
        self.names.len() - 1
    }
    fn connect(&mut self, a: usize, b: usize) {
        self.edges[a].push(b);
    }
}

fn indices_instead_of_rc() {
    let mut g = Graph { names: Vec::new(), edges: Vec::new() };
    let a = g.add("A");
    let b = g.add("B");
    let c = g.add("C");
    g.connect(a, b);
    g.connect(b, c);
    g.connect(c, a); // a cycle; with Rc this would leak

    for (i, outs) in g.edges.iter().enumerate() {
        let targets: Vec<&str> = outs.iter().map(|&j| g.names[j].as_str()).collect();
        println!("  {} -> {:?}", g.names[i], targets);
    }

    g.names[a].push_str(" (renamed)"); // plain mutation, no borrow_mut()
    println!("  {}", g.names[a]);
}