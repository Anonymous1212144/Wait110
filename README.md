# Wait110
Zero instruction Turing-complete programming language. A variant of Wait where the current nanotime is used as the initial state of Rule 110. This program is intended to be extremely future-proof, and should run any time in the future where they can compile a program following the current C standard (C23).

Features include:
- Expanding array so it can run any initial state until hardware limit
- size_t to always make the most of larger addressable space of the future
- uintmax_t to always make the most of bigger integer types of the future, and to account for any possible nanotime of the future
- Command-line argument to control how frequently the state is printed. Usage: `wait110.exe <uintmax_t>`. This causes the program to print every `<uintmax_t>` states, and at 0 it only prints the initial state.
