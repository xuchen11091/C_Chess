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

typedef struct _piece
{
    bool live;
    char piece;
    
} piece;

typedef struct 
{
    int x;
    int y;
} location;

typedef struct _board
{
    piece board[8][8];
    location whiteKing;
    location blackKing;
    bool whiteAttacks[8][8];
    bool blackAttacks[8][8];
} BOARD;

BOARD *boardSetUp(void);    // should be done
void printBoard(BOARD *chessBoard);     // finished
bool playPiece(int coordStart, int coordDestination, BOARD *chessBoard);    // mostly done other than castling and maybe en passant
void startGame();   // done for now but might need modification later on depending on what will be added
void ai_playPiece(int *AI_SCORE, BOARD *chessBoard);
int gameCheck(BOARD *chessBoard);
bool moveChecker(int coordStart, int coordDestination, BOARD *chessBoard);
void updateAttackMap(BOARD *chessBoard);
bool moveParsing(char *move, int coordRecorder[]);     // finished
bool isCastle(int coordStart, int coordDestination, BOARD *chessBoard);
bool checkEmpty(int x, int y, BOARD *chessBoard);

int main(int argc, char *argv[])
{
    startGame();
    BOARD *chessBOARD = boardSetUp(); // note for mr walia: this is just me checking if the board is being printed correctly
    printBoard(chessBOARD);
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
    return n;
}

bool playPiece(int coordStart, int coordDestination, BOARD *chessBoard)
{
    // TODO
    // Return true if the play is possible and return fale if the play is not possible
    if (moveChecker(coordStart, coordDestination, chessBoard) == true)
    {
        if (isCastle(coordStart, coordDestination, chessBoard) == false)
        {
            int startX = coordStart % 10, startY = coordStart / 10;
            int endX = coordDestination % 10, endY = coordDestination / 10;

            char movedPiece = chessBoard->board[startY][startX].piece;

            chessBoard->board[endY][endX] = chessBoard->board[startY][startX];
            chessBoard->board[startY][startX].piece = ' ';
            chessBoard->board[startY][startX].live = false;
            chessBoard->board[endY][endX].live = true;

            if (tolower(movedPiece) == 'k') {
                if (isupper(movedPiece)) {
                    chessBoard->whiteKing.x = endX;
                    chessBoard->whiteKing.y = endY;
                } else {
                    chessBoard->blackKing.x = endX;
                    chessBoard->blackKing.y = endY;
                }
            }
            return true;


        }
        else
        {
            // castle the king and the rook and then return true signaling the move has been played
            return true;
        }

    }
    printf("Inavlid move\n");
    return false;
}

bool isCastle(int coordStart, int coordDestination, BOARD *chessBoard)
{
    // if the move is castling, return true, otherwise return false
    return false;
}

void startGame()
{
    BOARD *gameBoard = boardSetUp();
    bool playChecker = false;
    int playCoordinates[2] = {0,0};

    int ai_score = 0; // at the start of the game the ai_score (evaluation) will be zero. It will increase and decrease based on the moves the opponent plays and 
                      // will be used as an evaluation metric for what moves is best to be played

    printf("Please enter a move in the format of 0A1B, with 0A being the starting POS and 1B as the destination pos.\n");

    while (gameCheck(gameBoard) != 1 && gameCheck(gameBoard) != 2)
    {
        printBoard(gameBoard);
        char userMove[100];
        
        while (playChecker == false)
        {
            // TODO
            // get player input for the start and end coordinates of a piece being played as playerCoordinates
            do
            {
                printf("Move: ");
                scanf("%s", userMove);
            } while (moveParsing(userMove, playCoordinates) == false);

            playChecker = playPiece(playCoordinates[0], playCoordinates[1], gameBoard);
        }
        updateAttackMap(gameBoard);
        playChecker = false;
        ai_playPiece(&ai_score, gameBoard);
        updateAttackMap(gameBoard);
    }
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
    return;
}

void ai_playPiece(int *AI_SCORE, BOARD *chessBoard)
{
    // TODO
    // make the ai play moves based off a simple minimax-esque algorithm that is based off the ai_score (evaluation) <------ use minimax with a depth of 2
    return;
}

int gameCheck(BOARD *chessBoard)
{
    int checker;

    // TODO
    // if the game has not ended, the function will return 0
    // if the game ends in checkmate, the function will return 1
    // if the game ends in stalemate, the function will return 2
    return 0;   // have this function return checker once the code is written
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
                    return true;
                }

                // Move forward two from starting position
                if (startY == 6 && startX == endX && startY - endY == 2)
                {
                    if (checkEmpty(startX, startY - 1, chessBoard) && checkEmpty(startX, endY, chessBoard))
                    {
                        return true;
                    }
                }

                // TODO: handle diagonal captures and en passant
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
                    return true;
                return false;
            case 'N':   //white knight
                return false;
            case 'B':   //white bishop
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
                    return true;

                return false;
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
                        return true;
                }

                // Straight line like a rook
                else if (startX == endX || startY == endY)
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
                        return true;
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
                        return true;
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
                        return true;
                }

                return false; // neither a valid bishop nor rook move
        }
    }

    /*
    There was originally a switch statement for black pieces here, but I feel like that would take up too much time and resources during runtime.
    I think instead of simulating all 64x64 possibilities, we should somehow generate a list of only legal moves first which is also ranked somehow.    
    */

    return true;    //     <--- this is set to true for now just so the code compiles and I can test the move piece function, after everything is implmeneted this will be set to false
}

bool checkEmpty(int x, int y, BOARD *chessBoard)
{
    if (chessBoard->board[y][x].piece == ' ')
    {
        return true;
    }
    return false;
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