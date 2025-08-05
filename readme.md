# Terminal Chess AI in C

This project is a fully functioning terminal-based chess game written in C, featuring a playable human vs AI mode, complete with move validation, board evaluation, and minimax AI with alpha-beta pruning. The engine supports advanced chess rules such as castling, en passant, and promotion, and includes a heuristic evaluation system tuned for competitive play.

---

## 🎮 Features

- Full chess game playable via terminal
- Human vs AI gameplay
- Minimax AI with alpha-beta pruning
- Rule-complete engine:
  - ✅ Castling (both sides)
  - ✅ En passant
  - ✅ Pawn promotion
  - ✅ Stalemate and checkmate detection
- Opening book for early AI moves
- Piece-square tables and tactical evaluation (e.g., hanging pieces)
- Dynamic attack map and king safety tracking
- Evaluation heuristics for:
  - Material balance
  - Passed, doubled, and isolated pawns
  - Bishop pair, connected rooks, castling bonuses
  - Hanging pieces and move ordering

---

## 🧠 AI & Evaluation Logic

The AI uses a depth-limited minimax algorithm with alpha-beta pruning to search for the best moves. The evaluation function includes:

- `VALUE_PAWN`, `VALUE_KNIGHT`, `VALUE_BISHOP`, `VALUE_ROOK`, `VALUE_QUEEN`, `VALUE_KING`
- Positional scoring using piece-square tables
- Tactical considerations (e.g., hanging pieces, MVV-LVA)
- Bonus/Penalty weights for:
  - Passed pawns
  - Doubled/isolated pawns
  - Castled king
  - Connected rooks
  - Bishop pair

AI selects moves based on a scoring function that balances positional strategy and tactical gain.

---

## 🧩 Structure

Main components:
- `BOARD`: Struct containing game state, board, kings' positions, castling rights, en passant targets, etc.
- `MOVE`: Struct representing legal chess moves, including type (castling, en passant, promotion)
- `moveChecker()`: Validates individual piece moves
- `moveLeavesKingInCheck()`: Ensures king is not left in check after a move
- `evaluateBoard()`: Heuristic board evaluation used by minimax
- `minimax()`: Recursive AI search with alpha-beta pruning
- `generateMoves()`: Legal move generator with ordering for efficient pruning
- `startGame()`: Starts the player-vs-AI match loop

---

## 🛠 How to Compile and Run

```bash
gcc -o chess main.c
./chess
```

Make sure all your functions and `main()` are in a single file (or adjust filenames accordingly).

---

## ⌨️ How to Play

- Input your moves in the format `2e4e` (start square + end square)
  - e.g. `2e4e` moves the piece at e2 to e4
- Promotion will prompt a piece selection (Q/R/B/N)
- The game alternates between player (White) and AI (Black)

---

## 📁 Files & Codebase Overview

- `main.c`: Main file containing board logic, AI logic, move evaluation, and game loop
- No external dependencies (pure standard C)
- Modular code separated into functions for:
  - Move generation
  - Evaluation
  - AI decision making
  - Game state validation

---

## 🔍 Potential Improvements

This is a strong foundation for a chess engine, but here are some possible upgrades:

- Add a GUI (e.g., SDL, ncurses)
- Implement PGN/FEN loading and saving
- Add support for threefold repetition and 50-move rule
- Integrate a more advanced evaluation model (e.g., neural network or centipawn analysis)
- Support UCI protocol for third-party GUI engines

---

## 📜 License

This project is provided under the MIT License. Feel free to modify, distribute, or build upon it.

---

## 🙌 Credits

Developed by Xu Chen (xuchen11091). Built using C and lots of debug patience. AI inspired by traditional chess engine design principles and heuristics used in classic engines.

---

## 💬 Final Notes

This project serves as a deep dive into how chess engines work under the hood — from board representation to AI search strategies. All rules are implemented manually without external libraries or prebuilt chess engines. It’s a great resource for learning AI, data structures, and game logic in C.
