#!/usr/bin/env python3
"""
Checks the output of the sieve against a trusted list of primes.

Usage: ./assign1_3 <n> | python3 check_primes.py <n>
Exit code 0 when the output is exactly the first n primes, 1 otherwise.
"""

import sys


def first_primes(n):
    # Returns the first n primes by trial division (slow but obviously correct).
    primes = []
    candidate = 2
    while len(primes) < n:
        if all(candidate % p != 0 for p in primes if p * p <= candidate):
            primes.append(candidate)
        candidate += 1
    return primes


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 1

    n = int(sys.argv[1])
    got = [int(line) for line in sys.stdin if line.strip()]
    expected = first_primes(n)

    if got == expected:
        print(f"OK: the first {n} primes are correct")
        return 0

    if len(got) != n:
        print(f"WRONG: got {len(got)} numbers, expected {n}")

    for i, (g, e) in enumerate(zip(got, expected)):
        if g != e:
            print(f"WRONG: prime number {i + 1} is {g}, expected {e}")
            break
    return 1


if __name__ == "__main__":
    sys.exit(main())
