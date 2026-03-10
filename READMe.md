# Time-Travelling File System

## Project Overview

This project is a simplified, in-memory version control system inspired by Git, developed for the COL106: Data Structures and Algorithms course. The system manages versioned files with support for historical inspection and branching. The primary goal is to apply a practical understanding of Trees, HashMaps, and Heaps by implementing them from scratch in C++.

**Author:** Yash Vaishnav
**ENTRY No.:** 2024CS50855

## How to Compile and Run

The project is designed to be compiled in a Linux-like environment (e.g., Linux, macOS, or **Git Bash/WSL on Windows**).

### 1. Navigate to the Directory
Open a bash terminal and use the `cd` command to navigate to the project's root directory where `main.cpp` and `compile.sh` are located.

### 2. Make the Script Executable (First Time Only)
This command gives the system permission to run the script. You only need to do this once.

```bash
chmod +x compile.sh
```

### 3. Compile the Code
Run the compilation script. This will use `g++` to compile the source code.

```bash
./compile.sh
```

### 4. Run the Program
Execute the compiled program to start the file system.

- On **Linux or macOS**, the command is:
```bash
./filesystem
```

- On **Windows (using Git Bash)**, you must include the `.exe` extension:
```bash
./filesystem.exe
```

## Available Commands

- **`CREATE <filename>`**: Creates a new file with an initial empty version (Version 0).
- **`READ <filename>`**: Displays the content of the file's currently active version.
- **`INSERT <filename> <content>`**: Appends content to the active version. Creates a new version if the active one is a snapshot.
- **`UPDATE <filename> <content>`**: Replaces the content of the active version. Creates a new version if the active one is a snapshot.
- **`SNAPSHOT <filename> <message>`**: Marks the active version as a permanent, immutable snapshot.
- **`ROLLBACK <filename> [versionID]`**: Sets the active version. If no ID is given, it rolls back to the parent of the current version.
- **`HISTORY <filename>`**: Lists all snapshotted versions on the path from the active version to the root.
- **`RECENT_FILES [num]`**: Lists the `num` most recently modified files (default is 5).
- **`BIGGEST_TREES [num]`**: Lists the `num` files with the most versions (default is 5).
- **`HELP`**: Displays the list of available commands.
- **`EXIT`**: Closes the program.

## Hard-Coded Values

The following values are configured in the system's source code:

- **Default `num` for analytics**: `5` (Used for `RECENT_FILES` and `BIGGEST_TREES` if no number is provided).
- **Default `HashMap` capacity**: `100` (Used for any `HashMap` unless overridden).
- **`FileSystem`'s main map capacity**: `20` (The `FileSystem` specifically initializes its `files` map with this capacity).
- **Initial Version ID**: `0` (All new files start with a root version numbered 0).
- **`Max-Heap` capacity**: Dynamic (The heap's capacity grows as needed and is not hard-coded)

## Implementation Details & Key Concepts

### Data Structures Used

As per the assignment requirements, the following core data structures were implemented from scratch:

- **Tree (`TreeNode`)**: The version history of each file is maintained as a tree, where each node is a `TreeNode` representing a specific version.
- **HashMap**: A custom hash map using chaining for collision resolution. It provides fast O(1) average-time lookups.
- **Max-Heap**: Two custom max-heaps are used to efficiently track system-wide analytics.

### Key Semantics

- **Snapshot**: A snapshot is a saved, permanent version of a file that cannot be changed. When you use the `SNAPSHOT` command, you "freeze" the file's content at that moment. Modifying a snapshotted version automatically creates a new child version, preserving the history.
- **Immutability**: Only versions marked as snapshots are immutable. The currently active version (if not a snapshot) can be edited in place.
- **Versioning**: Version IDs are unique per file and are assigned sequentially, starting from 0. The root version (Version 0) of every new file is automatically created as a snapshot.

### Assumptions

- **In-Memory Storage**: The file system operates entirely in memory. All data is lost when the program exits.
- **Input Formatting**: User input is assumed to be well-formed.

### Libraries & Technical Choices

- **Standard Libraries Used**: `<iostream>`, `<string>`, `<vector>`, `<ctime>`, `<sstream>`, `<chrono>`
- **Time Libraries (`<chrono>` vs `<ctime>`)**:
  - `<chrono>`: This modern C++ library is used to capture timestamps with **millisecond precision**. This is crucial for accurately ordering file modifications that occur within the same second.
  - `<ctime>`: This older library is used for the simpler task of formatting the high-precision timestamps into a human-readable date-time string for display.