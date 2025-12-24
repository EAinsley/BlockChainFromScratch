# BlockChainFromScratch

This repository hosts a course project that explores the implementation of
blockchain-related concepts from the ground up, with a strong emphasis on
software engineering practices, incremental development, and correctness.
The project is implemented in c++.

The project is developed iteratively, with functionality introduced in
well-defined stages. Each stage focuses on a small, testable subset of the
overall system.

## Current Status

The project is currently in an early foundational phase.

At this stage, the focus is on:

- Establishing the project structure and build system
- Implementing and validating cryptographic hash functionality
- Setting up a basic unit testing framework
- Providing a minimal frontend interface for demonstration purposes

## Build & Test

The project uses CMake as its build system.

Basic build and test instructions will be expanded as the project evolves

## Todos

- [ ] Stage 0
  - [x] Initialize project repository and folder structure
  - [ ] Setup CMake build system
  - [ ] Hash functionality
  - [ ] Create minimal frontend interface for hash demonstration
  - [ ] Setup basic unit testing framework and verify Hash tests

- [ ] Stage 1 – Block & Blockchain Core
  - [ ] Design Block data structure (header, nonce, hash link)
  - [ ] Implement Blockchain container and basic validation
  - [ ] Integrate Block with Hash functions

- [ ] Stage 2 – Mining & Proof-of-Work
  - [ ] Manual mining interface (adjust nonce, test hash)
  - [ ] Automated mining (single-threaded)
  - [ ] Configurable difficulty adjustment

- [ ] Stage 3 – Multi-node Simulation
  - [ ] Multiple nodes maintaining separate chains
  - [ ] Manual synchronization of chains
  - [ ] Observing chain divergence and replacement

- [ ] Stage 4 – State & Transactions
  - [ ] Account ledger and balance tracking
  - [ ] Transaction model and validation
  - [ ] Block-level transaction application

- [ ] Stage 5 – Token System
  - [ ] Native coin implementation
  - [ ] Support for additional tokens
  - [ ] Transaction handling for multiple token types

- [ ] Stage 6 – Optional / Advanced Features
  - [ ] Merkle Tree for transaction integrity
  - [ ] Concurrent mining simulation with multiple threads
  - [ ] Performance metrics and logging
  - [ ] Integration with enhanced frontend for interactive demonstration

  ## Reference

  [Block Chain from Scratch - sduprey](https://github.com/sduprey/blockchain_introductory_course/blob/main/blockchain_from_scratch/blockchain.py)

  [Simple Blockchain Implementation - fletelli42](https://github.com/fletelli42/SimpleBlockchainImplementation)
  [Simple Blockchian - tko22](https://github.com/tko22/simple-blockchain)
