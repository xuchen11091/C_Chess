# Terminal Chess in C

A terminal-based chess game written in C, featuring:

- Full board setup
- Basic move validation for white pieces
- King safety check to prevent illegal moves
- Move logging
- Turn-based player interaction

## Features

- Terminal board display
- Move parsing from user input
- Move legality checking per piece (partial)
- Move log (up to 1000 moves)
- Placeholder for AI move evaluation
- King safety checks (avoid self-check)

## Planned Features

- Black piece move validation
- Check, checkmate, stalemate detection
- Castling
- En Passant
- Pawn promotion
- Simple AI opponent with minimax (alpha beta pruning)
- Evaluation scoring for AI
- Full game over conditions

## Build & Run

```bash
clang -o chess chess.c
./chess
```

## Usage

Input moves in **0A1B** format:

- `0A` = starting square
- `1B` = destination square

Example:

```
Move: 2e3e
```

## Project Status

Work in progress — major rules and features still under development.

## License

MIT License (or specify if otherwise).

---

Feel free to contribute, suggest improvements, or fork this repository!

