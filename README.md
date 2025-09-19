# Discretia - Algorithm Visualizer Platformer Game

A platformer game that teaches sorting algorithms through interactive gameplay. Players control a character that can pick up and swap numbers to manually perform sorting operations.

## Features

### Implemented
- **Bubble Sort**: Manual bubble sort learning through pickup/swap mechanics
- **Player Movement**: Arrow keys/WASD for movement, SPACE/W/UP for jumping
- **Physics System**: Gravity, platform collision, respawn mechanics
- **Visual Feedback**: Sorted portions highlighted in green, interactive elements highlighted
- **Progressive Difficulty**: Tutorial level + increasing array sizes
- **Statistics Tracking**: Interaction and swap counters

### Planned
- Selection Sort, Insertion Sort, Merge Sort, Quick Sort implementations
- Sound effects and music
- Better graphics and animations
- More levels and challenges

## Controls

### Menu Navigation
- **Arrow Keys**: Navigate menus
- **ENTER**: Select option
- **ESC/Backspace**: Go back

### Gameplay
- **Arrow Keys / WASD**: Move player left/right
- **SPACE / W / UP**: Jump
- **F**: Pick up/place/swap numbers when standing on a platform
- **R**: Reset current level
- **ESC**: Pause game
- **Backspace**: Return to level select

## Bubble Sort Gameplay

1. **Goal**: Sort numbers in ascending order (1, 2, 3, 4...)
2. **Pickup**: Stand on a box with a number and press F to pick it up
3. **Place**: Stand on an empty box while carrying a number and press F to place it
4. **Swap**: Stand on a box with a number while carrying another number and press F to swap them
5. **Complete**: When all numbers are sorted in ascending order, the level is complete

## Building and Running

### Prerequisites
- GCC compiler
- Raylib library
- Linux/Unix system (tested on Ubuntu)

### Install Dependencies
```bash
make install-deps
```

### Build
```bash
make
```

### Run
```bash
make run
```

### Test
```bash
make test
```

### Clean
```bash
make clean
```

## Project Structure

```
src/
├── core/           # Core game systems
│   ├── game.c      # Main game loop and state management
│   ├── game.h      # Game data structures and constants
│   ├── player.c    # Player movement and physics
│   └── player.h    # Player function declarations
├── algorithms/     # Sorting algorithm implementations
│   ├── algorithm.c # Algorithm registry and utilities
│   ├── algorithm.h # Algorithm interface definitions
│   ├── bubble_sort.c    # Bubble sort implementation
│   ├── selection_sort.c # Selection sort (placeholder)
│   ├── insertion_sort.c # Insertion sort (placeholder)
│   ├── merge_sort.c     # Merge sort (placeholder)
│   └── quick_sort.c     # Quick sort (placeholder)
├── ui/             # User interface systems
│   ├── ui.c        # UI rendering and interaction
│   └── ui.h        # UI constants and structures
├── utils/          # Utility functions
│   ├── colors.c    # Color manipulation utilities
│   └── colors.h    # Color constants and definitions
└── tests/          # Unit tests
    └── bubble_sort_test.c # Bubble sort unit tests
```

## Architecture

The game uses a modular architecture with:

- **Algorithm Interface**: Each sorting algorithm implements a standard interface (init, update, render, cleanup, isComplete, resetLevel)
- **State Management**: Game states (menu, algorithm select, level select, gameplay, etc.)
- **Component System**: Separate systems for player movement, UI, rendering, and algorithm logic
- **Data-Driven Design**: Level data and algorithm parameters are easily configurable

## Implementation Details

### Bubble Sort Algorithm
- **Manual Interaction**: Players manually pick up and swap numbers instead of watching automatic sorting
- **Visual Feedback**: Sorted portions are highlighted in light green
- **Statistics**: Tracks interactions and swaps for educational feedback
- **Progressive Levels**: Starts with 4 elements, increases with each level

### Player Physics
- **Gravity**: Constant downward acceleration
- **Platform Collision**: Precise collision detection with array platforms
- **Respawn System**: Player respawns on first platform when falling off screen
- **Heart System**: Player loses hearts when falling, game over when hearts reach zero

### Visual Design
- **Clean Interface**: Minimalist design focusing on the algorithm visualization
- **Color Coding**: Different colors for sorted/unsorted elements, interactive highlights
- **Responsive Feedback**: Immediate visual response to player actions

## Contributing

This is an educational project demonstrating game-based algorithm visualization. The codebase is designed to be easily extensible for adding new sorting algorithms.

## License

Educational use - see individual algorithm implementations for specific details.