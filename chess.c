// =================== Evaluation Weights ===================
#define VALUE_KING 9999     // arbitrarily large to prioritize king safety
#define VALUE_QUEEN 9
#define VALUE_ROOK 5
#define VALUE_BISHOP 3      // I'm not sure whether or not to keep the values for bihop and knight
#define VALUE_KNIGHT 3      // some people think the knight is worth a bit more than the bishop
#define VALUE_PAWN 1
#define VALUE_PASSED_PAWN 2
#define BONUS_FORK 3
#define BONUS_KING_PRESSURE 2
#define VALUE_CASTLED_KING 50
#define PENALTY_DOUBLED_PAWN 50
#define PENALTY_ISOLATED_PAWN 20
#define BONUS_BISHOP_PAIR 50
#define BONUS_CONNECTED_ROOKS 25
#define MAX_DEPTH 4
#define INFINITY 10000

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

const char pieces[] =
{
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r', // I'm setting the knights as n because of the king being k
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
    'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
};

typedef struct
{
    bool live;
    char piece;
    
} piece;

typedef struct 
{
    int x;
    int y;
} location;

typedef struct _move
{
    int from;
    int to;
    char movedPiece;
    char capturedPiece;
    char promotionPiece;  // For pawn promotion
    bool isCastling;      // For castling moves
    bool isEnPassant;     // For en passant captures
    int score;
} MOVE;

typedef struct _board
{
    char *moveLog[1000];
    int moveCount;
    piece board[8][8];
    location whiteKing;
    location blackKing;
    bool whiteAttacks[8][8];
    bool blackAttacks[8][8];
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;
    bool whiteCastled;
    bool blackCastled;
    int enPassantFile;     // -1 if no en passant possible, 0-7 for file
    int enPassantRank;     // rank of the en passant target square
} BOARD;

BOARD *boardSetUp(void);
void printBoard(BOARD *chessBoard);
bool playPiece(int coordStart, int coordDestination, BOARD *chessBoard);
void startGame();
void ai_playPiece(int *AI_SCORE, BOARD *chessBoard);
int gameCheck(BOARD *chessBoard);
bool moveChecker(int coordStart, int coordDestination, BOARD *chessBoard);
void updateAttackMap(BOARD *chessBoard);
bool moveParsing(char *move, int coordRecorder[]);
bool isCastle(int coordStart, int coordDestination, BOARD *chessBoard);
bool checkEmpty(int x, int y, BOARD *chessBoard);
bool moveLeavesKingInCheck(BOARD *chessBoard, int color);
int evaluateBoard(BOARD *chessBoard);
int minimax(BOARD *chessBoard, int depth, int alpha, int beta, bool maximizingPlayer);
void generateMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite);
bool makeMove(BOARD *chessBoard, MOVE move);
void undoMove(BOARD *chessBoard, MOVE move);
BOARD* copyBoard(BOARD *original);
void freeBoard(BOARD *board);
bool isInCheck(BOARD *chessBoard, bool isWhite);
bool hasLegalMoves(BOARD *chessBoard, bool isWhite);
MOVE getOpeningMove(BOARD *chessBoard);
bool canCastle(BOARD *chessBoard, bool isWhite, bool kingside);
void generateCastlingMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite);
void generateEnPassantMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite);
void generatePromotionMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite, int from, int to);
bool isPinnedPiece(BOARD *chessBoard, int piecePos, bool isWhite);
bool isSquareAttacked(BOARD *chessBoard, int x, int y, bool byWhite);
int getPieceValue(char piece);
bool isPieceHanging(BOARD *chessBoard, int x, int y);
int evaluateCaptures(BOARD *chessBoard, MOVE move);
void orderMoves(BOARD *chessBoard, MOVE moves[], int moveCount);
int countAttackers(BOARD *chessBoard, int x, int y, bool isWhite);

int main(int argc, char *argv[])
{
    startGame();
    return 0;
}

BOARD *boardSetUp(void)
{
    BOARD *n = malloc(sizeof(BOARD));

    for (int i = 0, p = 0; i < 8; i++)
    {
        for (int k = 0; k < 8; k++)
        {
            if (i == 0 || i == 1 || i == 6 || i == 7)
            {
                n->board[i][k].live = true;
                n->board[i][k].piece = pieces[p];
                p++;
            }
            else
            {
                n->board[i][k].piece = ' ';
                n->board[i][k].live = false;
            }
        }
    }
    n->blackKing.x = 4, n->blackKing.y = 0;
    n->whiteKing.x = 4, n->whiteKing.y = 7;
    n->moveCount = 0;
    
    // Initialize castling rights
    n->whiteCanCastleKingside = true;
    n->whiteCanCastleQueenside = true;
    n->blackCanCastleKingside = true;
    n->blackCanCastleQueenside = true;
    n->whiteCastled = false;
    n->blackCastled = false;
    
    // Initialize en passant
    n->enPassantFile = -1;
    n->enPassantRank = -1;
    
    return n;
}

bool playPiece(int coordStart, int coordDestination, BOARD *chessBoard)
{
    if (moveChecker(coordStart, coordDestination, chessBoard) == true)
    {
        int startX = coordStart % 10, startY = coordStart / 10;
        int endX = coordDestination % 10, endY = coordDestination / 10;
        char movedPiece = chessBoard->board[startY][startX].piece;
        
        // Check for castling
        if (tolower(movedPiece) == 'k' && abs(endX - startX) == 2) {
            // This is castling
            bool kingside = (endX > startX);
            
            // Move king
            chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
            chessBoard->board[startY][startX].piece = ' ';
            chessBoard->board[startY][startX].live = false;
            
            // Move rook
            if (kingside) {
                // Kingside castling
                chessBoard->board[startY][endX-1] = chessBoard->board[startY][7];
                chessBoard->board[startY][7].piece = ' ';
                chessBoard->board[startY][7].live = false;
            } else {
                // Queenside castling
                chessBoard->board[startY][endX+1] = chessBoard->board[startY][0];
                chessBoard->board[startY][0].piece = ' ';
                chessBoard->board[startY][0].live = false;
            }
            
            // Update king position and castling rights
            if (isupper(movedPiece)) {
                chessBoard->whiteKing.x = endX;
                chessBoard->whiteKing.y = endY;
                chessBoard->whiteCanCastleKingside = false;
                chessBoard->whiteCanCastleQueenside = false;
                chessBoard->whiteCastled = true;
            } else {
                chessBoard->blackKing.x = endX;
                chessBoard->blackKing.y = endY;
                chessBoard->blackCanCastleKingside = false;
                chessBoard->blackCanCastleQueenside = false;
                chessBoard->blackCastled = true;
            }
            
            return true;
        }
        
        // Check for en passant
        if (tolower(movedPiece) == 'p' && abs(endX - startX) == 1 && 
            chessBoard->board[endY][endX].piece == ' ' &&
            chessBoard->enPassantFile == endX) {
            // This is en passant
            chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
            chessBoard->board[startY][startX].piece = ' ';
            chessBoard->board[startY][startX].live = false;
            
            // Remove captured pawn
            int capturedPawnY = isupper(movedPiece) ? endY + 1 : endY - 1;
            chessBoard->board[capturedPawnY][endX].piece = ' ';
            chessBoard->board[capturedPawnY][endX].live = false;
            
            return true;
        }
        
        // Regular move
        chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
        chessBoard->board[startY][startX].piece = ' ';
        chessBoard->board[startY][startX].live = false;
        chessBoard->board[endY][endX].live = true;

        // Update king position
        if (tolower(movedPiece) == 'k') {
            if (isupper(movedPiece)) {
                chessBoard->whiteKing.x = endX;
                chessBoard->whiteKing.y = endY;
                chessBoard->whiteCanCastleKingside = false;
                chessBoard->whiteCanCastleQueenside = false;
            } else {
                chessBoard->blackKing.x = endX;
                chessBoard->blackKing.y = endY;
                chessBoard->blackCanCastleKingside = false;
                chessBoard->blackCanCastleQueenside = false;
            }
        }
        
        // Update castling rights if rook moves
        if (tolower(movedPiece) == 'r') {
            if (startX == 0 && startY == 0) chessBoard->blackCanCastleQueenside = false;
            if (startX == 7 && startY == 0) chessBoard->blackCanCastleKingside = false;
            if (startX == 0 && startY == 7) chessBoard->whiteCanCastleQueenside = false;
            if (startX == 7 && startY == 7) chessBoard->whiteCanCastleKingside = false;
        }
        
        // Set en passant target if pawn moves two squares
        chessBoard->enPassantFile = -1;
        if (tolower(movedPiece) == 'p' && abs(endY - startY) == 2) {
            chessBoard->enPassantFile = startX;
            chessBoard->enPassantRank = (startY + endY) / 2;
        }
        
        // Handle pawn promotion
        if (tolower(movedPiece) == 'p' && (endY == 0 || endY == 7)) {
            printf("Pawn promotion! Choose piece (Q/R/B/N): ");
            char promotion;
            scanf(" %c", &promotion);
            promotion = toupper(promotion);
            if (promotion != 'Q' && promotion != 'R' && promotion != 'B' && promotion != 'N') {
                promotion = 'Q'; // Default to queen
            }
            chessBoard->board[endY][endX].piece = isupper(movedPiece) ? promotion : tolower(promotion);
        }
        
        // Validate the move doesn't leave king in check
        if (!moveLeavesKingInCheck(chessBoard, isupper(movedPiece) ? 0 : 1)) {
            return true;
        } else {
            // Move is illegal, need to undo it (this is complex, so for now just return false)
            return false;
        }
    }
    return false;
}

bool isCastle(int coordStart, int coordDestination, BOARD *chessBoard)
{
    int startX = coordStart % 10, startY = coordStart / 10;
    int endX = coordDestination % 10, endY = coordDestination / 10;
    char piece = chessBoard->board[startY][startX].piece;
    
    // Must be king moving
    if (tolower(piece) != 'k') return false;
    
    // Must move exactly 2 squares horizontally
    if (abs(endX - startX) != 2 || startY != endY) return false;
    
    return true;
}

void startGame()
{
    BOARD *gameBoard = boardSetUp();
    bool playChecker = false;
    int playCoordinates[2] = {0,0};

    int ai_score = 0;

    printf("Please enter a move in the format of 0A1B, with 0A being the starting POS and 1B as the destination pos.\n");

    int gameResult;
    while ((gameResult = gameCheck(gameBoard)) == 0)
    {
        printBoard(gameBoard);
        char userMove[100];
        
        while (playChecker == false)
        {
            do
            {
                printf("Move: ");
                scanf("%s", userMove);
            } while (moveParsing(userMove, playCoordinates) == false);

            playChecker = playPiece(playCoordinates[0], playCoordinates[1], gameBoard);
        }
        gameBoard->moveLog[gameBoard->moveCount] = strdup(userMove);
        gameBoard->moveCount++;
        updateAttackMap(gameBoard);
        
        // Check if game ended after player move
        if ((gameResult = gameCheck(gameBoard)) != 0) {
            printBoard(gameBoard);
            break;
        }
        
        playChecker = false;
        ai_playPiece(&ai_score, gameBoard);
        updateAttackMap(gameBoard);
    }
    
    // Game has ended - print final result
    printBoard(gameBoard);
    printf("Game over!\n");
}

bool moveParsing(char *move, int coordRecorder[])
{
    if (strlen(move) != 4)
    {
        printf("Invaid input lengt\n");
        return false;
    }

    if (move[0] == move[2] && move[1] == move[3])
    {
        printf("Invalid move: starting pos cannot be the same as the end pos\n");
        return false;
    }

    if ((move[0] >= '1' && move[0] <= '8') &&
        (tolower(move[1]) >= 'a' && tolower(move[1]) <= 'h') &&
        (move[2] >= '1' && move[2] <= '8') &&
        (tolower(move[3]) >= 'a' && tolower(move[3]) <= 'h'))
    {

        coordRecorder[0] = (7 - (move[0] - '1')) * 10 + (tolower(move[1]) - 'a');
        coordRecorder[1] = (7 - (move[2] - '1')) * 10 + (tolower(move[3]) - 'a');
        return true;
    }
    printf("Invalid input\n");
    return false;

}

void updateAttackMap(BOARD *chessBoard)
{
    // Clear attack maps
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            chessBoard->whiteAttacks[i][j] = false;
            chessBoard->blackAttacks[i][j] = false;
        }
    }
    
    // Generate attack maps for all pieces
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char piece = chessBoard->board[y][x].piece;
            if (piece == ' ') continue;
            
            bool isWhite = isupper(piece);
            
            // Mark squares this piece attacks
            for (int ty = 0; ty < 8; ty++) {
                for (int tx = 0; tx < 8; tx++) {
                    int from = y * 10 + x;
                    int to = ty * 10 + tx;
                    if (from != to && moveChecker(from, to, chessBoard)) {
                        if (isWhite) {
                            chessBoard->whiteAttacks[ty][tx] = true;
                        } else {
                            chessBoard->blackAttacks[ty][tx] = true;
                        }
                    }
                }
            }
        }
    }
}

int evaluateBoard(BOARD *chessBoard)
{
    int score = 0;
    
    // Count material to determine game phase
    int materialCount = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char piece = chessBoard->board[y][x].piece;
            if (piece != ' ' && tolower(piece) != 'k') {
                materialCount++;
            }
        }
    }
    bool isEndgame = materialCount <= 12;
    
    // Different king tables for opening/endgame
    int kingTableOpening[8][8] = {
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-20,-30,-30,-40,-40,-30,-30,-20},
        {-10,-20,-20,-20,-20,-20,-20,-10},
        { 20, 20,  0,  0,  0,  0, 20, 20},
        { 20, 30, 10,  0,  0, 10, 30, 20}
    };
    
    int kingTableEndgame[8][8] = {
        {-50,-40,-30,-20,-20,-30,-40,-50},
        {-30,-20,-10,  0,  0,-10,-20,-30},
        {-30,-10, 20, 30, 30, 20,-10,-30},
        {-30,-10, 30, 40, 40, 30,-10,-30},
        {-30,-10, 30, 40, 40, 30,-10,-30},
        {-30,-10, 20, 30, 30, 20,-10,-30},
        {-30,-30,  0,  0,  0,  0,-30,-30},
        {-50,-30,-30,-30,-30,-30,-30,-50}
    };
    
    // Piece-square tables for positional evaluation
    int pawnTable[8][8] = {
        {0,  0,  0,  0,  0,  0,  0,  0},
        {50, 50, 50, 50, 50, 50, 50, 50},
        {10, 10, 20, 30, 30, 20, 10, 10},
        {5,  5, 10, 25, 25, 10,  5,  5},
        {0,  0,  0, 20, 20,  0,  0,  0},
        {5, -5,-10,  0,  0,-10, -5,  5},
        {5, 10, 10,-20,-20, 10, 10,  5},
        {0,  0,  0,  0,  0,  0,  0,  0}
    };
    
    int knightTable[8][8] = {
        {-50,-40,-30,-30,-30,-30,-40,-50},
        {-40,-20,  0,  0,  0,  0,-20,-40},
        {-30,  0, 10, 15, 15, 10,  0,-30},
        {-30,  5, 15, 20, 20, 15,  5,-30},
        {-30,  0, 15, 20, 20, 15,  0,-30},
        {-30,  5, 10, 15, 15, 10,  5,-30},
        {-40,-20,  0,  5,  5,  0,-20,-40},
        {-50,-40,-30,-30,-30,-30,-40,-50}
    };
    
    // Track special features
    int whiteBishops = 0, blackBishops = 0;
    int whiteRooks = 0, blackRooks = 0;
    
    // TACTICAL EVALUATION - Check for hanging pieces
    int hangingPenalty = 0;
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char piece = chessBoard->board[y][x].piece;
            if (piece == ' ') continue;
            
            int pieceValue = 0;
            int positionalValue = 0;
            
            // Check if piece is hanging (BIG TACTICAL PENALTY)
            if (isPieceHanging(chessBoard, x, y)) {
                int hangingValue = getPieceValue(piece) * 200; // Massive penalty for hanging pieces
                if (isupper(piece)) {
                    score += hangingValue; // White piece hanging is bad for white
                } else {
                    score -= hangingValue; // Black piece hanging is good for white
                }
            }
            
            switch (tolower(piece)) {
                case 'p': 
                    pieceValue = VALUE_PAWN;
                    
                    // Check for passed pawn
                    bool passed = true;
                    int direction = isupper(piece) ? -1 : 1;
                    for (int py = y + direction; py >= 0 && py < 8; py += direction) {
                        for (int px = (x > 0 ? x-1 : 0); px <= (x < 7 ? x+1 : 7); px++) {
                            char enemyPiece = chessBoard->board[py][px].piece;
                            if (enemyPiece == (isupper(piece) ? 'p' : 'P')) {
                                passed = false;
                                break;
                            }
                        }
                        if (!passed) break;
                    }
                    if (passed && (isupper(piece) ? y > 3 : y < 4)) {
                        pieceValue += VALUE_PASSED_PAWN;
                    }
                    
                    // Check for doubled pawns
                    bool doubled = false;
                    for (int py = 0; py < 8; py++) {
                        if (py != y && chessBoard->board[py][x].piece == piece) {
                            doubled = true;
                            break;
                        }
                    }
                    if (doubled) pieceValue -= PENALTY_DOUBLED_PAWN;
                    
                    // Check for isolated pawns
                    bool isolated = true;
                    for (int px = x-1; px <= x+1; px += 2) {
                        if (px >= 0 && px < 8) {
                            for (int py = 0; py < 8; py++) {
                                if (chessBoard->board[py][px].piece == piece) {
                                    isolated = false;
                                    break;
                                }
                            }
                        }
                        if (!isolated) break;
                    }
                    if (isolated) pieceValue -= PENALTY_ISOLATED_PAWN;
                    
                    positionalValue = isupper(piece) ? pawnTable[7-y][x] : pawnTable[y][x];
                    break;
                    
                case 'n': 
                    pieceValue = VALUE_KNIGHT;
                    positionalValue = isupper(piece) ? knightTable[7-y][x] : knightTable[y][x];
                    break;
                    
                case 'b': 
                    pieceValue = VALUE_BISHOP;
                    if (isupper(piece)) whiteBishops++; else blackBishops++;
                    // Bishops prefer long diagonals
                    if ((x == y) || (x + y == 7)) positionalValue += 10;
                    break;
                    
                case 'r': 
                    pieceValue = VALUE_ROOK;
                    if (isupper(piece)) whiteRooks++; else blackRooks++;
                    
                    // Bonus for rook on open file
                    bool openFile = true;
                    for (int ry = 0; ry < 8; ry++) {
                        if (chessBoard->board[ry][x].piece == 'p' || chessBoard->board[ry][x].piece == 'P') {
                            openFile = false;
                            break;
                        }
                    }
                    if (openFile) positionalValue += 20;
                    break;
                    
                case 'q': 
                    pieceValue = VALUE_QUEEN;
                    // Queen centralization in endgame
                    if (isEndgame && x >= 2 && x <= 5 && y >= 2 && y <= 5) {
                        positionalValue += 10;
                    }
                    
                    // Penalty for early queen development
                    if (!isEndgame && (isupper(piece) ? y < 6 : y > 1)) {
                        positionalValue -= 30; // Discourage early queen moves
                    }
                    break;
                    
                case 'k': 
                    pieceValue = VALUE_KING;
                    
                    // Use different king tables for different game phases
                    if (isEndgame) {
                        positionalValue = isupper(piece) ? kingTableEndgame[7-y][x] : kingTableEndgame[y][x];
                    } else {
                        positionalValue = isupper(piece) ? kingTableOpening[7-y][x] : kingTableOpening[y][x];
                    }
                    
                    // Castling bonus
                    if (isupper(piece) && chessBoard->whiteCastled) {
                        positionalValue += VALUE_CASTLED_KING;
                    } else if (!isupper(piece) && chessBoard->blackCastled) {
                        positionalValue += VALUE_CASTLED_KING;
                    }
                    break;
            }
            
            // Check if piece is defended/safe
            bool isPieceWhite = isupper(piece);
            int attackers = countAttackers(chessBoard, x, y, !isPieceWhite);
            int defenders = countAttackers(chessBoard, x, y, isPieceWhite) - 1; // Don't count the piece itself
            
            if (attackers > defenders && attackers > 0) {
                // Piece is under attack with insufficient defense
                pieceValue -= (getPieceValue(piece) * (attackers - defenders)) / 4;
            }
            
            int totalValue = (pieceValue * 100 + positionalValue) / 100;
            
            if (isupper(piece)) {
                score -= totalValue;
            } else {
                score += totalValue;
            }
        }
    }
    
    // Bishop pair bonus
    if (whiteBishops >= 2) score -= BONUS_BISHOP_PAIR;
    if (blackBishops >= 2) score += BONUS_BISHOP_PAIR;
    
    // Connected rooks bonus (simplified version)
    if (whiteRooks >= 2) score -= BONUS_CONNECTED_ROOKS / 2;
    if (blackRooks >= 2) score += BONUS_CONNECTED_ROOKS / 2;
    
    // Mobility bonus (but less important than tactics)
    MOVE moves[200];
    int moveCount;
    generateMoves(chessBoard, moves, &moveCount, false);
    score += moveCount / 2; // Reduced mobility weight
    
    generateMoves(chessBoard, moves, &moveCount, true);
    score -= moveCount / 2; // Reduced mobility weight
    
    return score;
}

BOARD* copyBoard(BOARD *original)
{
    BOARD *copy = malloc(sizeof(BOARD));
    memcpy(copy, original, sizeof(BOARD));
    return copy;
}

void freeBoard(BOARD *board)
{
    free(board);
}

bool makeMove(BOARD *chessBoard, MOVE move)
{
    int startX = move.from % 10, startY = move.from / 10;
    int endX = move.to % 10, endY = move.to / 10;
    
    if (move.isCastling) {
        // Handle castling
        bool kingside = (endX > startX);
        
        // Move king
        chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
        chessBoard->board[startY][startX].piece = ' ';
        chessBoard->board[startY][startX].live = false;
        
        // Move rook
        if (kingside) {
            chessBoard->board[startY][endX-1] = chessBoard->board[startY][7];
            chessBoard->board[startY][7].piece = ' ';
            chessBoard->board[startY][7].live = false;
        } else {
            chessBoard->board[startY][endX+1] = chessBoard->board[startY][0];
            chessBoard->board[startY][0].piece = ' ';
            chessBoard->board[startY][0].live = false;
        }
        
        // Update king position and castling rights
        if (isupper(move.movedPiece)) {
            chessBoard->whiteKing.x = endX;
            chessBoard->whiteKing.y = endY;
            chessBoard->whiteCanCastleKingside = false;
            chessBoard->whiteCanCastleQueenside = false;
            chessBoard->whiteCastled = true;
        } else {
            chessBoard->blackKing.x = endX;
            chessBoard->blackKing.y = endY;
            chessBoard->blackCanCastleKingside = false;
            chessBoard->blackCanCastleQueenside = false;
            chessBoard->blackCastled = true;
        }
    } else if (move.isEnPassant) {
        // Handle en passant
        chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
        chessBoard->board[startY][startX].piece = ' ';
        chessBoard->board[startY][startX].live = false;
        
        // Remove captured pawn
        int capturedPawnY = isupper(move.movedPiece) ? endY + 1 : endY - 1;
        chessBoard->board[capturedPawnY][endX].piece = ' ';
        chessBoard->board[capturedPawnY][endX].live = false;
    } else {
        // Regular move
        chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
        chessBoard->board[startY][startX].piece = ' ';
        chessBoard->board[startY][startX].live = false;
        chessBoard->board[endY][endX].live = true;
        
        // Handle pawn promotion
        if (move.promotionPiece != ' ') {
            chessBoard->board[endY][endX].piece = move.promotionPiece;
        }
    }
    
    // Update king position for regular moves
    if (tolower(move.movedPiece) == 'k' && !move.isCastling) {
        if (isupper(move.movedPiece)) {
            chessBoard->whiteKing.x = endX;
            chessBoard->whiteKing.y = endY;
            chessBoard->whiteCanCastleKingside = false;
            chessBoard->whiteCanCastleQueenside = false;
        } else {
            chessBoard->blackKing.x = endX;
            chessBoard->blackKing.y = endY;
            chessBoard->blackCanCastleKingside = false;
            chessBoard->blackCanCastleQueenside = false;
        }
    }
    
    // Update castling rights if rook moves
    if (tolower(move.movedPiece) == 'r') {
        if (startX == 0 && startY == 0) chessBoard->blackCanCastleQueenside = false;
        if (startX == 7 && startY == 0) chessBoard->blackCanCastleKingside = false;
        if (startX == 0 && startY == 7) chessBoard->whiteCanCastleQueenside = false;
        if (startX == 7 && startY == 7) chessBoard->whiteCanCastleKingside = false;
    }
    
    return true;
}

void undoMove(BOARD *chessBoard, MOVE move)
{
    int startX = move.from % 10, startY = move.from / 10;
    int endX = move.to % 10, endY = move.to / 10;
    
    if (move.isCastling) {
        // Undo castling
        bool kingside = (endX > startX);
        
        // Move king back
        chessBoard->board[startY][startX] = chessBoard->board[endY][endX];
        chessBoard->board[endY][endX].piece = ' ';
        chessBoard->board[endY][endX].live = false;
        
        // Move rook back
        if (kingside) {
            chessBoard->board[startY][7] = chessBoard->board[startY][endX-1];
            chessBoard->board[startY][endX-1].piece = ' ';
            chessBoard->board[startY][endX-1].live = false;
        } else {
            chessBoard->board[startY][0] = chessBoard->board[startY][endX+1];
            chessBoard->board[startY][endX+1].piece = ' ';
            chessBoard->board[startY][endX+1].live = false;
        }
        
        // Restore king position (castling rights would need game state stack to restore properly)
        if (isupper(move.movedPiece)) {
            chessBoard->whiteKing.x = startX;
            chessBoard->whiteKing.y = startY;
        } else {
            chessBoard->blackKing.x = startX;
            chessBoard->blackKing.y = startY;
        }
    } else if (move.isEnPassant) {
        // Undo en passant
        chessBoard->board[startY][startX] = chessBoard->board[endY][endX];
        chessBoard->board[endY][endX].piece = ' ';
        chessBoard->board[endY][endX].live = false;
        
        // Restore captured pawn
        int capturedPawnY = isupper(move.movedPiece) ? endY + 1 : endY - 1;
        chessBoard->board[capturedPawnY][endX].piece = move.capturedPiece;
        chessBoard->board[capturedPawnY][endX].live = true;
    } else {
        // Undo regular move
        chessBoard->board[startY][startX] = chessBoard->board[endY][endX];
        chessBoard->board[endY][endX].piece = move.capturedPiece;
        chessBoard->board[endY][endX].live = (move.capturedPiece != ' ');
        
        // Restore original piece if it was a promotion
        if (move.promotionPiece != ' ') {
            chessBoard->board[startY][startX].piece = move.movedPiece;
        }
    }
    
    // Restore king position for regular moves
    if (tolower(move.movedPiece) == 'k' && !move.isCastling) {
        if (isupper(move.movedPiece)) {
            chessBoard->whiteKing.x = startX;
            chessBoard->whiteKing.y = startY;
        } else {
            chessBoard->blackKing.x = startX;
            chessBoard->blackKing.y = startY;
        }
    }
}

void generateMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite)
{
    *moveCount = 0;
    
    // Generate castling moves first
    generateCastlingMoves(chessBoard, moves, moveCount, isWhite);
    
    // Generate en passant moves
    generateEnPassantMoves(chessBoard, moves, moveCount, isWhite);
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char piece = chessBoard->board[y][x].piece;
            if (piece == ' ') continue;
            
            bool isPieceWhite = isupper(piece);
            if (isPieceWhite != isWhite) continue;
            
            int from = y * 10 + x;
            
            for (int ty = 0; ty < 8; ty++) {
                for (int tx = 0; tx < 8; tx++) {
                    int to = ty * 10 + tx;
                    if (from == to) continue;
                    
                    if (moveChecker(from, to, chessBoard)) {
                        // Check for pawn promotion
                        if (tolower(piece) == 'p' && (ty == 0 || ty == 7)) {
                            generatePromotionMoves(chessBoard, moves, moveCount, isWhite, from, to);
                        } else {
                            MOVE move;
                            move.from = from;
                            move.to = to;
                            move.movedPiece = piece;
                            move.capturedPiece = chessBoard->board[ty][tx].piece;
                            move.promotionPiece = ' ';
                            move.isCastling = false;
                            move.isEnPassant = false;
                            move.score = 0;
                            
                            // Test if this move leaves king in check by making the move temporarily
                            BOARD *testBoard = copyBoard(chessBoard);
                            makeMove(testBoard, move);
                            bool leavesKingInCheck = moveLeavesKingInCheck(testBoard, isWhite ? 0 : 1);
                            freeBoard(testBoard);
                            
                            if (!leavesKingInCheck) {
                                moves[(*moveCount)++] = move;
                                if (*moveCount >= 200) {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int minimax(BOARD *chessBoard, int depth, int alpha, int beta, bool maximizingPlayer)
{
    if (depth == 0) {
        return evaluateBoard(chessBoard);
    }
    
    MOVE moves[200];
    int moveCount;
    generateMoves(chessBoard, moves, &moveCount, maximizingPlayer);
    
    if (moveCount == 0) {
        if (isInCheck(chessBoard, maximizingPlayer)) {
            return maximizingPlayer ? -INFINITY : INFINITY;
        } else {
            return 0;
        }
    }
    
    // Order moves for better alpha-beta pruning
    orderMoves(chessBoard, moves, moveCount);
    
    if (maximizingPlayer) {
        int maxEval = -INFINITY;
        for (int i = 0; i < moveCount; i++) {
            makeMove(chessBoard, moves[i]);
            int eval = minimax(chessBoard, depth - 1, alpha, beta, false);
            undoMove(chessBoard, moves[i]);
            
            maxEval = (eval > maxEval) ? eval : maxEval;
            alpha = (alpha > eval) ? alpha : eval;
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return maxEval;
    } else {
        int minEval = INFINITY;
        for (int i = 0; i < moveCount; i++) {
            makeMove(chessBoard, moves[i]);
            int eval = minimax(chessBoard, depth - 1, alpha, beta, true);
            undoMove(chessBoard, moves[i]);
            
            minEval = (eval < minEval) ? eval : minEval;
            beta = (beta < eval) ? beta : eval;
            if (beta <= alpha) break; // Alpha-beta pruning
        }
        return minEval;
    }
}

void ai_playPiece(int *AI_SCORE, BOARD *chessBoard)
{
    MOVE moves[200];
    int moveCount;
    generateMoves(chessBoard, moves, &moveCount, false);
    
    if (moveCount == 0) {
        printf("AI has no legal moves!\n");
        return;
    }
    
    // Opening book for first few moves
    if (chessBoard->moveCount <= 6) {
        MOVE openingMove = getOpeningMove(chessBoard);
        if (openingMove.from != -1) {
            makeMove(chessBoard, openingMove);
            *AI_SCORE = evaluateBoard(chessBoard);
            
            int fromX = openingMove.from % 10, fromY = openingMove.from / 10;
            int toX = openingMove.to % 10, toY = openingMove.to / 10;
            printf("AI plays: %c%d%c%d (opening book)\n", 
                   'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY);
            return;
        }
    }
    
    // Order moves for better tactical play
    orderMoves(chessBoard, moves, moveCount);
    
    MOVE bestMove = moves[0];
    int bestScore = -INFINITY;
    
    for (int i = 0; i < moveCount; i++) {
        makeMove(chessBoard, moves[i]);
        int score = minimax(chessBoard, MAX_DEPTH - 1, -INFINITY, INFINITY, true);
        undoMove(chessBoard, moves[i]);
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = moves[i];
        }
    }
    
    makeMove(chessBoard, bestMove);
    *AI_SCORE = bestScore;
    
    int fromX = bestMove.from % 10, fromY = bestMove.from / 10;
    int toX = bestMove.to % 10, toY = bestMove.to / 10;
    
    // Show what type of move was made
    if (bestMove.capturedPiece != ' ') {
        printf("AI plays: %c%d%c%d (captures %c)\n", 
               'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY, bestMove.capturedPiece);
    } else if (bestMove.isCastling) {
        printf("AI plays: %c%d%c%d (castling)\n", 
               'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY);
    } else if (bestMove.promotionPiece != ' ') {
        printf("AI plays: %c%d%c%d (promotes to %c)\n", 
               'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY, bestMove.promotionPiece);
    } else {
        printf("AI plays: %c%d%c%d\n", 
               'a' + fromX, 8 - fromY, 'a' + toX, 8 - toY);
    }
}

bool isInCheck(BOARD *chessBoard, bool isWhite)
{
    return moveLeavesKingInCheck(chessBoard, isWhite ? 0 : 1);
}

bool hasLegalMoves(BOARD *chessBoard, bool isWhite)
{
    MOVE moves[200];
    int moveCount;
    generateMoves(chessBoard, moves, &moveCount, isWhite);
    return moveCount > 0;
}

int gameCheck(BOARD *chessBoard)
{
    bool whiteHasMoves = hasLegalMoves(chessBoard, true);
    bool blackHasMoves = hasLegalMoves(chessBoard, false);
    bool whiteInCheck = isInCheck(chessBoard, true);
    bool blackInCheck = isInCheck(chessBoard, false);
    
    if (!whiteHasMoves) {
        if (whiteInCheck) {
            printf("Checkmate! Black wins!\n");
            return 1; // checkmate
        } else {
            printf("Stalemate!\n");
            return 2; // stalemate
        }
    }
    
    if (!blackHasMoves) {
        if (blackInCheck) {
            printf("Checkmate! White wins!\n");
            return 1; // checkmate
        } else {
            printf("Stalemate!\n");
            return 2; // stalemate
        }
    }
    
    return 0; // game continues
}

bool moveLeavesKingInCheck(BOARD *chessBoard, int color)    // 0 for white, 1 for black
{
    /*
    **********************************
    TODO: implement this POS fucntion
    **********************************

        --return true if the move leaves the king in check

    */
    if (color == 0)
    {
        int kingX = chessBoard->whiteKing.x, compX;
        int kingY = chessBoard->whiteKing.y, compY;

        // checks up
        if (kingY - 1 >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX].piece == 'k')
            {
                return true;
            }
            compY = kingY - 1;
            while(compY >= 0)
            {
                if ((chessBoard->board[compY][kingX].piece >= 65 && chessBoard->board[compY][kingX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][kingX].piece == 'r' ||
                            chessBoard->board[compY][kingX].piece == 'q'
                )
                {
                    return true;
                }
                compY--;
            }
        }

        // checks down

        if (kingY + 1 <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX].piece == 'k')
            {
                return true;
            }
            compY = kingY + 1;
            while (compY <= 7)
            {
                if ((chessBoard->board[compY][kingX].piece >= 65 && chessBoard->board[compY][kingX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][kingX].piece == 'r' ||
                            chessBoard->board[compY][kingX].piece == 'q'
                )
                {
                    return true;
                }
                compY++;
            }
        }

        // checks right

        if (kingX + 1 <= 7)
        {
            if (chessBoard->board[kingY][kingX + 1].piece == 'k')
            {
                return true;
            }
            compX = kingX + 1;
            while (compX <= 7)
            {
                if ((chessBoard->board[kingY][compX].piece >= 65 && chessBoard->board[kingY][compX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[kingY][compX].piece == 'r' ||
                            chessBoard->board[kingY][compX].piece == 'q'
                )
                {
                    return true;
                }
                compX++;
            }
        }

        // checks left

        if (kingX - 1 >= 0)
        {
            if (chessBoard->board[kingY][kingX - 1].piece == 'k')
            {
                return true;
            }
            compX = kingX - 1;
            while (compX >= 0)
            {
                if ((chessBoard->board[kingY][compX].piece >= 65 && chessBoard->board[kingY][compX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[kingY][compX].piece == 'r' ||
                            chessBoard->board[kingY][compX].piece == 'q'
                )
                {
                    return true;
                }
                compX--;
            }
        }

        // forward right diagonal

        if ((kingX + 1) <= 7 && (kingY - 1) >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX + 1].piece == 'k' || chessBoard->board[kingY - 1][kingX + 1].piece == 'p')
            {
                return true;
            }
            compX = kingX + 1, compY = kingY - 1;
            while (compX <= 7 && compY >= 0)
            {
                if ((chessBoard->board[compY][compX].piece >= 65 && chessBoard->board[compY][compX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'q' ||
                            chessBoard->board[compY][compX].piece == 'b'    
                )
                {
                    return true;
                }
                compX++, compY--;
            }
        }

        // forward left diagonal

        if ((kingX - 1) >= 0 && (kingY - 1) >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX - 1].piece == 'k' || chessBoard->board[kingY - 1][kingX - 1].piece == 'p')
            {
                return true;
            }
            compX = kingX - 1, compY = kingY - 1;
            while (compX >= 0 && compY >= 0)
            {
                if ((chessBoard->board[compY][compX].piece >= 65 && chessBoard->board[compY][compX].piece <= 90))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'q' ||
                            chessBoard->board[compY][compX].piece == 'b'
                )
                {
                    return true;
                }
                compX--, compY--;
            }
        }

        //backward right diagonal

        if ((kingX + 1) <= 7 && (kingY + 1) <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX + 1].piece == 'k')
            {
                return true;
            }
            compX = kingX + 1, compY = kingY + 1;
            while (compX <= 7 && compY <= 7)
            {
                if (chessBoard->board[compY][compX].piece >= 65 && chessBoard->board[compY][compX].piece <= 90)
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'q' ||
                            chessBoard->board[compY][compX].piece == 'b'
                )
                {
                    return true;
                }
                compX++, compY++;
            }
        }

        // check for knight moves

        int knightMoves[8][2] = {
            {-2, -1}, {-2, +1}, {-1, -2}, {-1, +2},
            {+1, -2}, {+1, +2}, {+2, -1}, {+2, +1}
        };

        for (int i = 0; i < 8; i++) {
            int nx = kingX + knightMoves[i][1];
            int ny = kingY + knightMoves[i][0];

            if (nx >= 0 && nx <= 7 && ny >= 0 && ny <= 7) {
                if (chessBoard->board[ny][nx].piece == 'n') {
                    return true;
                }
            }
        }

        //backward left diagonal

        if ((kingX - 1) >= 0 && (kingY + 1) <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX - 1].piece == 'k')
            {
                return true;
            }
            compX = kingX - 1, compY = kingY + 1;
            while (compX >= 0 && compY <= 7)
            {
                if (chessBoard->board[compY][compX].piece >= 65 && chessBoard->board[compY][compX].piece <= 90)
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'q' ||
                            chessBoard->board[compY][compX].piece == 'b'
                )
                {
                    return true;
                }
                compX--, compY++;
            }
        }

        // if the functino does not return true
        return false;
    }
    else
    {
        int kingX = chessBoard->blackKing.x, compX;
        int kingY = chessBoard->blackKing.y, compY;

        // checks up
        if (kingY - 1 >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX].piece == 'K')
            {
                return true;
            }
            compY = kingY - 1;
            while(compY >= 0)
            {
                if ((chessBoard->board[compY][kingX].piece >= 97 && chessBoard->board[compY][kingX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][kingX].piece == 'R' ||
                            chessBoard->board[compY][kingX].piece == 'Q'
                )
                {
                    return true;
                }
                compY--;
            }
        }

        // checks down
        if (kingY + 1 <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX].piece == 'K')
            {
                return true;
            }
            compY = kingY + 1;
            while (compY <= 7)
            {
                if ((chessBoard->board[compY][kingX].piece >= 97 && chessBoard->board[compY][kingX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][kingX].piece == 'R' ||
                            chessBoard->board[compY][kingX].piece == 'Q'
                )
                {
                    return true;
                }
                compY++;
            }
        }

        // checks right
        if (kingX + 1 <= 7)
        {
            if (chessBoard->board[kingY][kingX + 1].piece == 'K')
            {
                return true;
            }
            compX = kingX + 1;
            while (compX <= 7)
            {
                if ((chessBoard->board[kingY][compX].piece >= 97 && chessBoard->board[kingY][compX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[kingY][compX].piece == 'R' ||
                            chessBoard->board[kingY][compX].piece == 'Q'
                )
                {
                    return true;
                }
                compX++;
            }
        }

        // checks left
        if (kingX - 1 >= 0)
        {
            if (chessBoard->board[kingY][kingX - 1].piece == 'K')
            {
                return true;
            }
            compX = kingX - 1;
            while (compX >= 0)
            {
                if ((chessBoard->board[kingY][compX].piece >= 97 && chessBoard->board[kingY][compX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[kingY][compX].piece == 'R' ||
                            chessBoard->board[kingY][compX].piece == 'Q'
                )
                {
                    return true;
                }
                compX--;
            }
        }

        // forward right diagonal (up-right for black)
        if ((kingX + 1) <= 7 && (kingY - 1) >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX + 1].piece == 'K')
            {
                return true;
            }
            compX = kingX + 1, compY = kingY - 1;
            while (compX <= 7 && compY >= 0)
            {
                if ((chessBoard->board[compY][compX].piece >= 97 && chessBoard->board[compY][compX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'Q' ||
                            chessBoard->board[compY][compX].piece == 'B'    
                )
                {
                    return true;
                }
                compX++, compY--;
            }
        }

        // forward left diagonal (up-left for black)
        if ((kingX - 1) >= 0 && (kingY - 1) >= 0)
        {
            if (chessBoard->board[kingY - 1][kingX - 1].piece == 'K')
            {
                return true;
            }
            compX = kingX - 1, compY = kingY - 1;
            while (compX >= 0 && compY >= 0)
            {
                if ((chessBoard->board[compY][compX].piece >= 97 && chessBoard->board[compY][compX].piece <= 122))
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'Q' ||
                            chessBoard->board[compY][compX].piece == 'B'
                )
                {
                    return true;
                }
                compX--, compY--;
            }
        }

        //backward right diagonal (down-right for black)
        if ((kingX + 1) <= 7 && (kingY + 1) <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX + 1].piece == 'K' || chessBoard->board[kingY + 1][kingX + 1].piece == 'P')
            {
                return true;
            }
            compX = kingX + 1, compY = kingY + 1;
            while (compX <= 7 && compY <= 7)
            {
                if (chessBoard->board[compY][compX].piece >= 97 && chessBoard->board[compY][compX].piece <= 122)
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'Q' ||
                            chessBoard->board[compY][compX].piece == 'B'
                )
                {
                    return true;
                }
                compX++, compY++;
            }
        }

        // check for knight moves
        int knightMoves[8][2] = {
            {-2, -1}, {-2, +1}, {-1, -2}, {-1, +2},
            {+1, -2}, {+1, +2}, {+2, -1}, {+2, +1}
        };

        for (int i = 0; i < 8; i++) {
            int nx = kingX + knightMoves[i][1];
            int ny = kingY + knightMoves[i][0];

            if (nx >= 0 && nx <= 7 && ny >= 0 && ny <= 7) {
                if (chessBoard->board[ny][nx].piece == 'N') {
                    return true;
                }
            }
        }

        //backward left diagonal (down-left for black)
        if ((kingX - 1) >= 0 && (kingY + 1) <= 7)
        {
            if (chessBoard->board[kingY + 1][kingX - 1].piece == 'K' || chessBoard->board[kingY + 1][kingX - 1].piece == 'P')
            {
                return true;
            }
            compX = kingX - 1, compY = kingY + 1;
            while (compX >= 0 && compY <= 7)
            {
                if (chessBoard->board[compY][compX].piece >= 97 && chessBoard->board[compY][compX].piece <= 122)
                {
                    break;
                }
                else if (
                            chessBoard->board[compY][compX].piece == 'Q' ||
                            chessBoard->board[compY][compX].piece == 'B'
                )
                {
                    return true;
                }
                compX--, compY++;
            }
        }

        return false;
    }


}

bool moveChecker(int coordStart, int coordDestination, BOARD *chessBoard)
{
    // first check if the move is valid (in terms of if it's a move that can be done by the piece)
    // then check king safety if the move is played
    // REMEMBER EN PASSANT, CASTLING, AND PAWN FIRST MOVE
    int startX = coordStart % 10, startY = coordStart / 10;
    int endX = coordDestination % 10, endY = coordDestination / 10;
    char piece = chessBoard->board[startY][startX].piece;
    char destination = chessBoard->board[endY][endX].piece;


    if (piece >= 65 && piece <= 90)
    {
        // white pieces
        switch (piece)
        {
            case 'P':   //white pawn
                // Move forward one
                if (startX == endX && startY - endY == 1 && checkEmpty(startX, endY, chessBoard))
                {
                    break;
                }
                // Move forward two from starting position
                if (startY == 6 && startX == endX && startY - endY == 2)
                {
                    if (checkEmpty(startX, startY - 1, chessBoard) && checkEmpty(startX, endY, chessBoard))
                    {
                        break;
                    }
                }
                // checks for whether or not take is possible
                if ((startY - endY == 1 && abs(endX - startX) == 1))
                {
                    if (!checkEmpty(endX, endY, chessBoard) && (chessBoard->board[endY][endX].piece >= 97 && chessBoard->board[endY][endX].piece <= 122))
                    {
                        break;
                    }
                }
                // ******** TODO: handle en passant *********
                return false;
            case 'R':   //white rook
                if (startX == endX && startY != endY)
                {
                    // vertical
                    int rookVertical = endY - startY;
                    int step = (rookVertical > 0) ? 1 : -1;
                    for (int y = startY + step; y != endY; y += step)
                    {
                        if (!checkEmpty(startX, y, chessBoard))
                            return false;
                    }
                }
                else if (startY == endY && startX != endX)
                {
                    // horizontal
                    int rookHorizontal = endX - startX;
                    int step = (rookHorizontal > 0) ? 1 : -1;
                    for (int x = startX + step; x != endX; x += step)
                    {
                        if (!checkEmpty(x, startY, chessBoard))
                            return false;
                    }
                }
                else {
                    return false; // ❗ invalid rook direction
                }

                if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                    break;
                return false;
            case 'N':   //white knight
            {
                int dx = abs(endX - startX);
                int dy = abs(endY - startY);
                if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2))
                {
                    if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                    {
                        break;
                    }
                }
                return false;
            }
            case 'B':   //white bishop
            {
                if (abs(endY - startY) != abs(endX - startX))
                    return false; // not a diagonal move

                int bishopVertical = endY - startY, bishopHorizontal = endX - startX;
                int stepVertical = (bishopVertical > 0) ? 1 : -1;
                int stepHorizontal = (bishopHorizontal > 0) ? 1 : -1;

                int x = startX + stepHorizontal;
                int y = startY + stepVertical;
                while (x != endX && y != endY)
                {
                    if (!checkEmpty(x, y, chessBoard))
                        return false;
                    x += stepHorizontal;
                    y += stepVertical;
                }

                // final square must be empty or contain black piece
                if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                    break;

                return false;
            }
            case 'Q':
            {
                // Diagonal like a bishop
                if (abs(endY - startY) == abs(endX - startX))
                {
                    int dx = (endX > startX) ? 1 : -1;
                    int dy = (endY > startY) ? 1 : -1;
                    int x = startX + dx;
                    int y = startY + dy;

                    while (x != endX && y != endY)
                    {
                        if (!checkEmpty(x, y, chessBoard)) return false;
                        x += dx;
                        y += dy;
                    }

                    if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                        break;
                }

                // Straight line like a rook
                else if (startX == endX || startY == endY)
                {
                    if (startX == endX)
                    {
                        int step = (endY > startY) ? 1 : -1;
                        for (int y = startY + step; y != endY; y += step)
                        {
                            if (!checkEmpty(startX, y, chessBoard)) return false;
                        }
                    } else if (startY == endY)
                    {
                        int step = (endX > startX) ? 1 : -1;
                        for (int x = startX + step; x != endX; x += step)
                        {
                            if (!checkEmpty(x, startY, chessBoard)) return false;
                        }
                    }

                    if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                        break;
                }

                return false; // neither a valid bishop nor rook move
            }
            case 'K':   //white king
                /*
                This is kind of a really shit way of doing this for the king moveChecker as it is pretty much
                the queen movecChecker copy and pasted but also checking for whether or not the king only moves 1 square.

                ** THINGS THAT HAVE TO BE IMPLEMENTED LATER **
                    - a way to check if the move will put the king in check (after the attackMapChecker is finished)
                    - a way to check if the move castles the king and a rook (have to check for checks on the way to the rook
                      and whether or not the king has been moved before castling).
                */

                // Diagonal like a bishop
                if (abs(endY - startY) == abs(endX - startX) && abs(endY - startY) == 1)
                {
                    int dx = (endX > startX) ? 1 : -1;
                    int dy = (endY > startY) ? 1 : -1;
                    int x = startX + dx;
                    int y = startY + dy;

                    while (x != endX && y != endY)
                    {
                        if (!checkEmpty(x, y, chessBoard)) return false;
                        x += dx;
                        y += dy;
                    }

                    if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                        break;
                }

                // Straight line like a rook
                else if ((startX == endX && abs(endY - startY) == 1)|| (startY == endY) && abs(endX - startX) == 1)
                {
                    if (startX == endX) {
                        int step = (endY > startY) ? 1 : -1;
                        for (int y = startY + step; y != endY; y += step)
                        {
                            if (!checkEmpty(startX, y, chessBoard)) return false;
                        }
                    } else if (startY == endY) {
                        int step = (endX > startX) ? 1 : -1;
                        for (int x = startX + step; x != endX; x += step)
                        {
                            if (!checkEmpty(x, startY, chessBoard)) return false;
                        }
                    }

                    if (destination == ' ' || (destination >= 'a' && destination <= 'z'))
                        break;
                }

                return false; // neither a valid bishop nor rook move
        }
    }

    /*
    There was originally a switch statement for black pieces here, but I feel like that would take up too much time and resources during runtime.
    I think instead of simulating all 64x64 possibilities, we should somehow generate a list of only legal moves first which is also ranked somehow.    
    */
    else if (piece >= 97 && piece <= 122)
    {
        // black pieces
        switch (piece)
        {
            case 'p':   //black pawn
                // Move forward one (down for black)
                if (startX == endX && endY - startY == 1 && checkEmpty(startX, endY, chessBoard))
                {
                    break;
                }
                // Move forward two from starting position
                if (startY == 1 && startX == endX && endY - startY == 2)
                {
                    if (checkEmpty(startX, startY + 1, chessBoard) && checkEmpty(startX, endY, chessBoard))
                    {
                        break;
                    }
                }
                // checks for whether or not take is possible
                if ((endY - startY == 1 && abs(endX - startX) == 1))
                {
                    if (!checkEmpty(endX, endY, chessBoard) && (chessBoard->board[endY][endX].piece >= 65 && chessBoard->board[endY][endX].piece <= 90))
                    {
                        break;
                    }
                }
                return false;
            case 'r':   //black rook
                if (startX == endX && startY != endY)
                {
                    // vertical
                    int rookVertical = endY - startY;
                    int step = (rookVertical > 0) ? 1 : -1;
                    for (int y = startY + step; y != endY; y += step)
                    {
                        if (!checkEmpty(startX, y, chessBoard))
                            return false;
                    }
                }
                else if (startY == endY && startX != endX)
                {
                    // horizontal
                    int rookHorizontal = endX - startX;
                    int step = (rookHorizontal > 0) ? 1 : -1;
                    for (int x = startX + step; x != endX; x += step)
                    {
                        if (!checkEmpty(x, startY, chessBoard))
                            return false;
                    }
                }
                else {
                    return false;
                }

                if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                    break;
                return false;
            case 'n':   //black knight
            {
                int dx = abs(endX - startX);
                int dy = abs(endY - startY);
                if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2))
                {
                    if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                    {
                        break;
                    }
                }
                return false;
            }
            case 'b':   //black bishop
            {
                if (abs(endY - startY) != abs(endX - startX))
                    return false;

                int bishopVertical = endY - startY, bishopHorizontal = endX - startX;
                int stepVertical = (bishopVertical > 0) ? 1 : -1;
                int stepHorizontal = (bishopHorizontal > 0) ? 1 : -1;

                int x = startX + stepHorizontal;
                int y = startY + stepVertical;
                while (x != endX && y != endY)
                {
                    if (!checkEmpty(x, y, chessBoard))
                        return false;
                    x += stepHorizontal;
                    y += stepVertical;
                }

                if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                    break;

                return false;
            }
            case 'q':   //black queen
            {
                // Diagonal like a bishop
                if (abs(endY - startY) == abs(endX - startX))
                {
                    int dx = (endX > startX) ? 1 : -1;
                    int dy = (endY > startY) ? 1 : -1;
                    int x = startX + dx;
                    int y = startY + dy;

                    while (x != endX && y != endY)
                    {
                        if (!checkEmpty(x, y, chessBoard)) return false;
                        x += dx;
                        y += dy;
                    }

                    if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                        break;
                }

                // Straight line like a rook
                else if (startX == endX || startY == endY)
                {
                    if (startX == endX)
                    {
                        int step = (endY > startY) ? 1 : -1;
                        for (int y = startY + step; y != endY; y += step)
                        {
                            if (!checkEmpty(startX, y, chessBoard)) return false;
                        }
                    } else if (startY == endY)
                    {
                        int step = (endX > startX) ? 1 : -1;
                        for (int x = startX + step; x != endX; x += step)
                        {
                            if (!checkEmpty(x, startY, chessBoard)) return false;
                        }
                    }

                    if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                        break;
                }

                return false;
            }
            case 'k':   //black king
                // Diagonal like a bishop
                if (abs(endY - startY) == abs(endX - startX) && abs(endY - startY) == 1)
                {
                    int dx = (endX > startX) ? 1 : -1;
                    int dy = (endY > startY) ? 1 : -1;
                    int x = startX + dx;
                    int y = startY + dy;

                    while (x != endX && y != endY)
                    {
                        if (!checkEmpty(x, y, chessBoard)) return false;
                        x += dx;
                        y += dy;
                    }

                    if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                        break;
                }

                // Straight line like a rook
                else if ((startX == endX && abs(endY - startY) == 1)|| (startY == endY) && abs(endX - startX) == 1)
                {
                    if (startX == endX) {
                        int step = (endY > startY) ? 1 : -1;
                        for (int y = startY + step; y != endY; y += step)
                        {
                            if (!checkEmpty(startX, y, chessBoard)) return false;
                        }
                    } else if (startY == endY) {
                        int step = (endX > startX) ? 1 : -1;
                        for (int x = startX + step; x != endX; x += step)
                        {
                            if (!checkEmpty(x, startY, chessBoard)) return false;
                        }
                    }

                    if (destination == ' ' || (destination >= 'A' && destination <= 'Z'))
                        break;
                }

                return false;
        }
    }
    
    if (!moveLeavesKingInCheck(chessBoard, (piece >= 65 && piece <= 90) ? 0 : 1))
    {
        return true;
    }
    return false;
}

bool checkEmpty(int x, int y, BOARD *chessBoard)
{
    if (chessBoard->board[y][x].piece == ' ')
    {
        return true;
    }
    return false;
}

bool isSquareAttacked(BOARD *chessBoard, int x, int y, bool byWhite)
{
    // Check if square (x,y) is attacked by pieces of the given color
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            char piece = chessBoard->board[sy][sx].piece;
            if (piece == ' ') continue;
            
            bool isPieceWhite = isupper(piece);
            if (isPieceWhite != byWhite) continue;
            
            // Check if this piece can attack the target square
            int from = sy * 10 + sx;
            int to = y * 10 + x;
            
            if (moveChecker(from, to, chessBoard)) {
                return true;
            }
        }
    }
    return false;
}

int getPieceValue(char piece)
{
    switch (tolower(piece)) {
        case 'p': return VALUE_PAWN;
        case 'n': return VALUE_KNIGHT;
        case 'b': return VALUE_BISHOP;
        case 'r': return VALUE_ROOK;
        case 'q': return VALUE_QUEEN;
        case 'k': return VALUE_KING;
        default: return 0;
    }
}

int countAttackers(BOARD *chessBoard, int x, int y, bool isWhite)
{
    int count = 0;
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            char piece = chessBoard->board[sy][sx].piece;
            if (piece == ' ') continue;
            
            bool isPieceWhite = isupper(piece);
            if (isPieceWhite != isWhite) continue;
            
            int from = sy * 10 + sx;
            int to = y * 10 + x;
            
            if (moveChecker(from, to, chessBoard)) {
                count++;
            }
        }
    }
    return count;
}

bool isPieceHanging(BOARD *chessBoard, int x, int y)
{
    char piece = chessBoard->board[y][x].piece;
    if (piece == ' ') return false;
    
    bool isPieceWhite = isupper(piece);
    
    // Count attackers and defenders
    int attackers = countAttackers(chessBoard, x, y, !isPieceWhite);
    int defenders = countAttackers(chessBoard, x, y, isPieceWhite);
    
    // If more attackers than defenders, piece might be hanging
    if (attackers > defenders) {
        return true;
    }
    
    // Even if equal, check if lowest value attacker < piece value
    if (attackers > 0) {
        int pieceValue = getPieceValue(piece);
        
        // Find lowest value attacker
        int lowestAttackerValue = VALUE_KING;
        for (int sy = 0; sy < 8; sy++) {
            for (int sx = 0; sx < 8; sx++) {
                char attackPiece = chessBoard->board[sy][sx].piece;
                if (attackPiece == ' ') continue;
                
                bool isAttackerWhite = isupper(attackPiece);
                if (isAttackerWhite == isPieceWhite) continue;
                
                int from = sy * 10 + sx;
                int to = y * 10 + x;
                
                if (moveChecker(from, to, chessBoard)) {
                    int attackerValue = getPieceValue(attackPiece);
                    if (attackerValue < lowestAttackerValue) {
                        lowestAttackerValue = attackerValue;
                    }
                }
            }
        }
        
        if (lowestAttackerValue < pieceValue) {
            return true;
        }
    }
    
    return false;
}

int evaluateCaptures(BOARD *chessBoard, MOVE move)
{
    if (move.capturedPiece == ' ') return 0;
    
    int capturedValue = getPieceValue(move.capturedPiece);
    int capturingValue = getPieceValue(move.movedPiece);
    
    // Basic MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
    int score = capturedValue * 10 - capturingValue;
    
    // Bonus if the captured piece was hanging
    int toX = move.to % 10, toY = move.to / 10;
    if (isPieceHanging(chessBoard, toX, toY)) {
        score += 100; // Big bonus for capturing hanging pieces
    }
    
    return score;
}

void orderMoves(BOARD *chessBoard, MOVE moves[], int moveCount)
{
    // Simple move ordering: captures first, then other moves
    for (int i = 0; i < moveCount; i++) {
        moves[i].score = 0;
        
        // Prioritize captures
        if (moves[i].capturedPiece != ' ') {
            moves[i].score += evaluateCaptures(chessBoard, moves[i]);
        }
        
        // Prioritize promotions
        if (moves[i].promotionPiece != ' ') {
            moves[i].score += getPieceValue(moves[i].promotionPiece) * 8;
        }
        
        // Prioritize castling
        if (moves[i].isCastling) {
            moves[i].score += 50;
        }
        
        // Penalty for moving pieces to attacked squares
        int toX = moves[i].to % 10, toY = moves[i].to / 10;
        bool movedPieceWhite = isupper(moves[i].movedPiece);
        if (isSquareAttacked(chessBoard, toX, toY, !movedPieceWhite)) {
            moves[i].score -= getPieceValue(moves[i].movedPiece) / 2;
        }
    }
    
    // Sort moves by score (highest first)
    for (int i = 0; i < moveCount - 1; i++) {
        for (int j = i + 1; j < moveCount; j++) {
            if (moves[j].score > moves[i].score) {
                MOVE temp = moves[i];
                moves[i] = moves[j];
                moves[j] = temp;
            }
        }
    }
}

bool canCastle(BOARD *chessBoard, bool isWhite, bool kingside)
{
    if (isWhite) {
        if (kingside && !chessBoard->whiteCanCastleKingside) return false;
        if (!kingside && !chessBoard->whiteCanCastleQueenside) return false;
        if (isInCheck(chessBoard, true)) return false; // Can't castle out of check
        
        // Check squares between king and rook are empty
        if (kingside) {
            if (!checkEmpty(5, 7, chessBoard) || !checkEmpty(6, 7, chessBoard)) return false;
            // Check that king doesn't pass through check
            BOARD *testBoard = copyBoard(chessBoard);
            testBoard->board[7][5] = testBoard->board[7][4]; // Move king one square
            testBoard->board[7][4].piece = ' ';
            testBoard->whiteKing.x = 5;
            bool passesThoughCheck = moveLeavesKingInCheck(testBoard, 0);
            freeBoard(testBoard);
            if (passesThoughCheck) return false;
        } else {
            if (!checkEmpty(1, 7, chessBoard) || !checkEmpty(2, 7, chessBoard) || !checkEmpty(3, 7, chessBoard)) return false;
            BOARD *testBoard = copyBoard(chessBoard);
            testBoard->board[7][3] = testBoard->board[7][4]; // Move king one square
            testBoard->board[7][4].piece = ' ';
            testBoard->whiteKing.x = 3;
            bool passesThoughCheck = moveLeavesKingInCheck(testBoard, 0);
            freeBoard(testBoard);
            if (passesThoughCheck) return false;
        }
    } else {
        if (kingside && !chessBoard->blackCanCastleKingside) return false;
        if (!kingside && !chessBoard->blackCanCastleQueenside) return false;
        if (isInCheck(chessBoard, false)) return false;
        
        if (kingside) {
            if (!checkEmpty(5, 0, chessBoard) || !checkEmpty(6, 0, chessBoard)) return false;
            BOARD *testBoard = copyBoard(chessBoard);
            testBoard->board[0][5] = testBoard->board[0][4];
            testBoard->board[0][4].piece = ' ';
            testBoard->blackKing.x = 5;
            bool passesThoughCheck = moveLeavesKingInCheck(testBoard, 1);
            freeBoard(testBoard);
            if (passesThoughCheck) return false;
        } else {
            if (!checkEmpty(1, 0, chessBoard) || !checkEmpty(2, 0, chessBoard) || !checkEmpty(3, 0, chessBoard)) return false;
            BOARD *testBoard = copyBoard(chessBoard);
            testBoard->board[0][3] = testBoard->board[0][4];
            testBoard->board[0][4].piece = ' ';
            testBoard->blackKing.x = 3;
            bool passesThoughCheck = moveLeavesKingInCheck(testBoard, 1);
            freeBoard(testBoard);
            if (passesThoughCheck) return false;
        }
    }
    return true;
}

void generateCastlingMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite)
{
    if (isWhite) {
        // White kingside castling
        if (canCastle(chessBoard, true, true)) {
            MOVE move;
            move.from = 74; // e1
            move.to = 76;   // g1
            move.movedPiece = 'K';
            move.capturedPiece = ' ';
            move.promotionPiece = ' ';
            move.isCastling = true;
            move.isEnPassant = false;
            move.score = 0;
            moves[(*moveCount)++] = move;
        }
        
        // White queenside castling
        if (canCastle(chessBoard, true, false)) {
            MOVE move;
            move.from = 74; // e1
            move.to = 72;   // c1
            move.movedPiece = 'K';
            move.capturedPiece = ' ';
            move.promotionPiece = ' ';
            move.isCastling = true;
            move.isEnPassant = false;
            move.score = 0;
            moves[(*moveCount)++] = move;
        }
    } else {
        // Black kingside castling
        if (canCastle(chessBoard, false, true)) {
            MOVE move;
            move.from = 4;  // e8
            move.to = 6;    // g8
            move.movedPiece = 'k';
            move.capturedPiece = ' ';
            move.promotionPiece = ' ';
            move.isCastling = true;
            move.isEnPassant = false;
            move.score = 0;
            moves[(*moveCount)++] = move;
        }
        
        // Black queenside castling
        if (canCastle(chessBoard, false, false)) {
            MOVE move;
            move.from = 4;  // e8
            move.to = 2;    // c8
            move.movedPiece = 'k';
            move.capturedPiece = ' ';
            move.promotionPiece = ' ';
            move.isCastling = true;
            move.isEnPassant = false;
            move.score = 0;
            moves[(*moveCount)++] = move;
        }
    }
}

void generateEnPassantMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite)
{
    if (chessBoard->enPassantFile == -1) return;
    
    int targetFile = chessBoard->enPassantFile;
    int targetRank = chessBoard->enPassantRank;
    
    // Check for pawns that can capture en passant
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            char piece = chessBoard->board[y][x].piece;
            if (piece == ' ') continue;
            
            bool isPieceWhite = isupper(piece);
            if (isPieceWhite != isWhite) continue;
            
            if (tolower(piece) == 'p') {
                // Check if this pawn can capture en passant
                if (abs(x - targetFile) == 1) {
                    int correctRank = isWhite ? 3 : 4; // White pawns on rank 5, black on rank 4
                    if (y == correctRank) {
                        MOVE move;
                        move.from = y * 10 + x;
                        move.to = targetRank * 10 + targetFile;
                        move.movedPiece = piece;
                        move.capturedPiece = isWhite ? 'p' : 'P';
                        move.promotionPiece = ' ';
                        move.isCastling = false;
                        move.isEnPassant = true;
                        move.score = 0;
                        moves[(*moveCount)++] = move;
                    }
                }
            }
        }
    }
}

void generatePromotionMoves(BOARD *chessBoard, MOVE moves[], int *moveCount, bool isWhite, int from, int to)
{
    char pawn = isWhite ? 'P' : 'p';
    char captured = chessBoard->board[to/10][to%10].piece;
    
    char promotionPieces[] = {'Q', 'R', 'B', 'N'};
    for (int i = 0; i < 4; i++) {
        MOVE move;
        move.from = from;
        move.to = to;
        move.movedPiece = pawn;
        move.capturedPiece = captured;
        move.promotionPiece = isWhite ? promotionPieces[i] : tolower(promotionPieces[i]);
        move.isCastling = false;
        move.isEnPassant = false;
        move.score = 0;
        moves[(*moveCount)++] = move;
    }
}

bool isPinnedPiece(BOARD *chessBoard, int piecePos, bool isWhite)
{
    int pieceX = piecePos % 10, pieceY = piecePos / 10;
    int kingX = isWhite ? chessBoard->whiteKing.x : chessBoard->blackKing.x;
    int kingY = isWhite ? chessBoard->whiteKing.y : chessBoard->blackKing.y;
    
    // Check if piece is on the same line as king
    bool sameLine = (pieceX == kingX) || (pieceY == kingY) || (abs(pieceX - kingX) == abs(pieceY - kingY));
    if (!sameLine) return false;
    
    // Temporarily remove the piece and see if king is in check
    char originalPiece = chessBoard->board[pieceY][pieceX].piece;
    chessBoard->board[pieceY][pieceX].piece = ' ';
    chessBoard->board[pieceY][pieceX].live = false;
    
    bool kingInCheck = moveLeavesKingInCheck(chessBoard, isWhite ? 0 : 1);
    
    // Restore the piece
    chessBoard->board[pieceY][pieceX].piece = originalPiece;
    chessBoard->board[pieceY][pieceX].live = true;
    
    return kingInCheck;
}

MOVE getOpeningMove(BOARD *chessBoard)
{
    MOVE invalidMove;
    invalidMove.from = -1;
    invalidMove.to = -1;
    
    // Expanded opening book with common openings
    
    if (chessBoard->moveCount == 0) {
        // First move options
        MOVE moves[] = {
            {61, 41, 'p', ' ', ' ', false, false, 0}, // e2-e4 
            {61, 51, 'p', ' ', ' ', false, false, 0}, // e2-e3
            {51, 31, 'p', ' ', ' ', false, false, 0}, // d2-d4
            {51, 41, 'p', ' ', ' ', false, false, 0}  // d2-d3
        };
        
        MOVE moves_available[200];
        int moveCount;
        generateMoves(chessBoard, moves_available, &moveCount, false);
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < moveCount; j++) {
                if (moves_available[j].from == moves[i].from && moves_available[j].to == moves[i].to) {
                    return moves[i];
                }
            }
        }
    }
    
    if (chessBoard->moveCount == 2) {
        // Second move - develop pieces
        MOVE moves[] = {
            {1, 22, 'n', ' ', ' ', false, false, 0},  // Nb8-c6
            {6, 25, 'n', ' ', ' ', false, false, 0},  // Ng8-f6
            {1, 32, 'n', ' ', ' ', false, false, 0},  // Nb8-d7
            {6, 27, 'n', ' ', ' ', false, false, 0}   // Ng8-h6
        };
        
        MOVE moves_available[200];
        int moveCount;
        generateMoves(chessBoard, moves_available, &moveCount, false);
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < moveCount; j++) {
                if (moves_available[j].from == moves[i].from && moves_available[j].to == moves[i].to) {
                    return moves[i];
                }
            }
        }
    }
    
    if (chessBoard->moveCount == 4) {
        // Third move - bishops and more development
        MOVE moves[] = {
            {2, 25, 'b', ' ', ' ', false, false, 0},  // Bc8-f5
            {5, 24, 'b', ' ', ' ', false, false, 0},  // Bf8-e7
            {5, 23, 'b', ' ', ' ', false, false, 0},  // Bf8-d6
            {2, 33, 'b', ' ', ' ', false, false, 0}   // Bc8-d7
        };
        
        MOVE moves_available[200];
        int moveCount;
        generateMoves(chessBoard, moves_available, &moveCount, false);
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < moveCount; j++) {
                if (moves_available[j].from == moves[i].from && moves_available[j].to == moves[i].to) {
                    return moves[i];
                }
            }
        }
    }
    
    if (chessBoard->moveCount == 6) {
        // Fourth move - castling or more development
        MOVE moves[] = {
            {4, 6, 'k', ' ', ' ', true, false, 0},    // Castling kingside
            {4, 2, 'k', ' ', ' ', true, false, 0},    // Castling queenside
            {3, 22, 'q', ' ', ' ', false, false, 0}   // Queen development
        };
        
        MOVE moves_available[200];
        int moveCount;
        generateMoves(chessBoard, moves_available, &moveCount, false);
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < moveCount; j++) {
                if (moves_available[j].from == moves[i].from && moves_available[j].to == moves[i].to) {
                    return moves[i];
                }
            }
        }
    }
    
    return invalidMove;
}

void printBoard(BOARD *chessBoard)
{
    printf("  A B C D E F G H\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%i|", 8 - i);
        for (int k = 0; k < 8; k++)
        {
            printf("%c|", chessBoard->board[i][k].piece);
        }
        printf("\n");
    }
    printf("\n");
}