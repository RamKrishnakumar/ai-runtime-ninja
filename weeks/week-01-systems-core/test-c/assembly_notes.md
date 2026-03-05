# 🧠 Day 1 – Process Memory Layout & Stack Frame Deep Dive

## Overview

Today I explored how a C program is laid out in memory and how function calls create stack frames.

This was not about just printing addresses — it was about understanding how the CPU and OS cooperate to execute a program.

---

# 1️⃣ Process Memory Layout (Virtual Memory View)

Each running process gets its own virtual address space.

High-level layout:

HIGH ADDRESSES
-----------------------------
Stack (grows downward)
-----------------------------
        (unused gap)
-----------------------------
Heap (grows upward)
-----------------------------
BSS (uninitialized globals)
-----------------------------
Data (initialized globals)
-----------------------------
Text (program code)
LOW ADDRESSES

Important:
This is **virtual memory**, not direct physical RAM layout.

---

# 2️⃣ Stack

The stack is a per-thread memory region used for:

- Function call frames
- Local variables
- Return addresses
- Saved registers

It follows a LIFO (Last In, First Out) structure.

The CPU tracks it using the RSP (stack pointer) register.

Key properties:

- Grows downward (toward lower addresses)
- Automatically managed during function calls
- Destroyed when function returns

---

# 3️⃣ Heap

The heap is used for dynamic memory allocation.

Used when:
- Memory size is not known at compile time
- Memory needs to outlive function scope

Managed via:
- malloc()
- free()

The heap:

- Grows upward (toward higher addresses)
- Is managed by the memory allocator
- May request more memory from OS using brk() or mmap()

---

# 4️⃣ Why Stack Grows Down & Heap Grows Up

This design allows:

- Maximum utilization of available address space
- Both regions grow toward each other
- Flexible memory expansion

If both collide → stack overflow or out-of-memory crash.

---

# 5️⃣ Stack Frame

Each function call creates a stack frame.

A stack frame contains:

- Return address (pushed by `call`)
- Saved base pointer (pushed by `push %rbp`)
- Local variables
- Possibly a stack canary (security feature)

When function returns:
- Frame is destroyed
- Control returns to caller

---

# 6️⃣ Understanding Key Assembly Instructions

### push %rbp
Saves caller's base pointer on stack.

### mov %rsp, %rbp
Sets current base pointer to current stack pointer.
Marks the base of the new stack frame.

### sub $0x20, %rsp
Reserves 32 bytes for local variables.
(Stack grows downward.)

### leave
Equivalent to:
    mov %rbp, %rsp
    pop %rbp

Destroys current stack frame.

### ret
Pops return address from stack and jumps back.

---

# 7️⃣ Function Call Flow

When a function is called:

1. `call function`
   - Pushes return address onto stack
   - Jumps to function

2. Function prologue:
   - push %rbp
   - mov %rsp, %rbp
   - sub $X, %rsp

3. Function executes

4. Function epilogue:
   - leave
   - ret

---

# 8️⃣ What I Observed from Address Output

- Stack variables had high memory addresses.
- Local variables inside different functions were close in memory.
- Heap memory was separate and located lower than stack.
- Global variables were in a different fixed region.

This confirms virtual memory segmentation.

---

# 9️⃣ Security Mechanism Observed

I saw stack canary instructions:

mov %fs:0x28,%rax
mov %rax,-0x8(%rbp)

This protects against buffer overflow attacks.

If canary changes → program aborts.

---

# 🔥 Key Realization

Modern systems rely heavily on:

- Stack management
- Memory isolation
- Controlled allocation

Understanding this is foundational for:

- Debugging segmentation faults
- Avoiding memory leaks
- Writing performant systems
- Building runtime engines
- Deploying large AI systems safely

---

# 🥷 Final Insight

Memory is not abstract.

It is structured, tracked, and manipulated explicitly by:

- CPU registers
- Assembly instructions
- OS memory manager

Understanding this changes how I view high-level languages and frameworks.



| Address | Instruction | Expert Explanation (The "Why") |
| :--- | :--- | :--- |
| 11fd | endbr64 | **The Entry Marker:** A security feature that marks this as a valid jump target. |
| 1201 | push %rbp | **Save the Past:** Pushes the caller's base pointer onto the stack to save it. |
| 1202 | mov %rsp,%rbp | **New Frame:** Sets the base pointer to the current stack top. This is the "floor" of main. |
| 1205 | sub $0x20,%rsp | **Space Creation:** Subtracts 32 bytes from the stack pointer to make room for local variables. |
| 1209 | mov %fs:0x28,%rax | **Security Guard:** Grabs a secret "canary" value from the Thread Control Block. |
| 1212 | mov %rax,-0x8(%rbp) | **The Guard's Post:** Places that canary at the end of the stack frame to watch for overflows. |
| 1216 | xor %eax,%eax | **Clean Slate:** Zeros out the %eax register (efficient way to do eax = 0). |
| 1218 | movl $0x14,-0x14(%rbp) | **Assign 20:** Moves 0x14 (20) into the stack at -0x14. This is `int stack_var = 20`. |
| 121f | mov $0x4,%edi | **Malloc Prep:** Puts 4 (size of an int) into %edi as the first argument for malloc. |
| 1224 | call 10b0 <malloc@plt> | **Call Heap:** Asks the OS for 4 bytes of heap memory. |
| 1229 | mov %rax,-0x10(%rbp) | **Save Pointer:** malloc returns the address in %rax. We save that pointer on our stack. |
| 122d | mov -0x10(%rbp),%rax | **Reload Pointer:** Moves that heap address back into %rax to use it. |
| 1231 | movl $0x1e,(%rax) | **Assign 30:** Moves 0x1e (30) into the memory address stored in %rax. |
| 1237 | lea 0x2dd2(%rip),%rax | **Find Global:** Calculates the address of global_var relative to the code pointer. |
| 123e | mov %rax,%rsi | **Print Arg 2:** Puts the global address into %rsi for printf. |
| 1241 | lea 0xdd6(%rip),%rax | **Find String:** Loads the format string "Address of global_var..." into %rax. |
| 1248 | mov %rax,%rdi | **Print Arg 1:** Puts that string into %rdi (the first argument register). |
| 124b | mov $0x0,%eax | **Var-Args:** Tells printf there are 0 floating-point arguments. |
| 1250 | call 10a0 <printf@plt> | **Print:** Finally executes the printf for the global variable. |
| 1290 | call 11a9 <function> | **Jump:** Pauses main and jumps to your function(). |
| 129c | call 1080 <free@plt> | **Cleanup:** Calls free on the heap address to prevent memory leaks. |
| 12aa | sub %fs:0x28,%rdx | **Check Guard:** Compares the canary value to see if it was changed (hacked). |
| 12b3 | je 12ba | **Safe Exit:** If the canary is fine, jump to the end. Otherwise, crash. |
| 12ba | leave | **Destruct:** A shortcut for mov %rbp, %rsp and pop %rbp. Cleans the desk. |
| 12bb | ret | **Return:** Pops the return address and goes back to the OS. |


Registers are temporary storage: %rdi, %rsi, %rdx, %rcx, %r8, and %r9 are used to pass arguments to functions (in that specific order).

Offsets are everything: main uses -0x14(%rbp) for one variable and -0x10(%rbp) for another. The compiler manages this "map" so variables don't collide.

The Stack is a "LIFO" (Last In, First Out) pile: Every push or sub grows it, and every pop or add (or leave) shrinks it. If you forget to shrink it, you get a Stack Overflow.