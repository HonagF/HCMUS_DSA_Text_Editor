# HCMUS DSA Text Editor

A graphical text editor built from scratch in C for the Data Structures and Algorithms course at Ho Chi Minh City University of Science (HCMUS).

## Overview

This project demonstrates the application of core data structures to build a functional text editor. Instead of relying on any built-in string manipulation libraries, the entire text buffer and editing features are powered by custom-built data structures. The backend logic is fully synchronized with a GTK4 graphical user interface.

## Core Data Structures

- Doubly Linked List: Acts as the primary text buffer. Each character is a node, allowing O(1) insertions and deletions at the cursor position.
- Stack: Powers the Undo/Redo engine. It tracks exact memory states and byte-offsets to revert complex bulk operations (like Cut and Replace All) safely.
- Trie (Prefix Tree): Supports the auto-complete dictionary, providing O(L) time complexity for word suggestions.

## Key Features

- GTK4 GUI: Includes mouse support, and text highlighting.
- State Synchronization: Custom event listeners bind the GTK text buffer directly to the Doubly Linked List core to ensure data consistency.
- Search and Replace: Supports case-sensitive matching and bulk replacements.
- Auto-complete: Suggests words as you type.
- Undo/Redo: Tracks single character edits and bulk range operations.

## Build Instructions

### Prerequisites
- GCC Compiler
- GTK4 Development Libraries
- pkg-config

### Compilation

Navigate to the source directory and compile the project using the provided Makefile:

```bash
cd source
mingw32-make
```

### Execution

Run the compiled executable:

```bash
./Editor.exe
```
