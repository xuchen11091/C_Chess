# ♟️ Terminal Chess AI in C

A fully playable terminal-based chess game built from scratch in C.  
Features human vs AI gameplay, full move validation, a tactical evaluation system, and a Minimax AI with alpha-beta pruning. Designed to run in the terminal with fast, optimized performance and support for all major chess rules.

---

## 🎮 Features

- Human vs AI chess game in the terminal
- Minimax algorithm with alpha-beta pruning
- Evaluation engine with weighted heuristics:
  - Material balance
  - Piece-square tables
  - King safety, castling bonus, fork bonuses
- Full move legality validation
  - ✅ Castling (both sides)
  - ✅ En passant
  - ✅ Pawn promotion
  - ✅ Check, checkmate, stalemate detection
- Dynamic attack maps and pinned piece logic
- Highly optimized for fast search at depth 4

---

## 🧠 AI Evaluation Heuristics (Simplified)

```c
#define VALUE_KING 9999
#define VALUE_QUEEN 9
#define VALUE_ROOK 5
#define VALUE_BISHOP 3
#define VALUE_KNIGHT 3
#define VALUE_PAWN 1

#define BONUS_FORK 3
#define BONUS_KING_PRESSURE 2
#define PENALTY_DOUBLED_PAWN 50
#define BONUS_CONNECTED_ROOKS 25
```

The engine uses a mix of:
- **Tactical bonuses** (e.g., forks, pressure)
- **Positional heuristics** (e.g., doubled pawns, castling)
- **Piece-square tables** for better piece placement

---

## 🛠️ How to Compile and Run

```bash
gcc chess.c -o chess -O2
./chess
```

Runs entirely in your terminal. No dependencies.

---

## 🧪 Sample Game State

```
8 r n b q k b n r
7 p p p p . p p p
6 . . . . p . . .
5 . . . . . . . .
4 . . . P P . . .
3 . . . . . . . .
2 P P P . . P P P
1 R N B Q K B N R
  a b c d e f g h

White to move.
Enter move (e.g. e2e4): 
```

---

## 📈 Optimization Highlights

Recent improvements to:
- **Alpha-beta pruning logic** (prunes up to 70% of branches)
- **Move legality checks** (faster pinned piece detection)
- **Memory footprint** (no dynamic memory allocation needed)

Search depth of 4 runs smoothly with sub-second response time on modern CPUs.

---

## 🧩 Project Structure

All code is written in a **single C file** with:
- Structs for board state and moves
- Modular functions for evaluation, move generation, legality, AI logic, and rendering

> 📌 Ideal for understanding how a chess engine works at a low level.

---

## 🧠 Why I Built This

I wanted to improve at low level programming and challenge myself to build a rule-complete chess engine from scratch, entirely in C, without external libraries.  
The AI uses a depth-4 minimax with tactical and positional evaluation, and was built to run cleanly and quickly in the terminal.

---

## 📬 Contact

Feel free to reach out or fork this repo if you'd like to contribute or suggest improvements.
