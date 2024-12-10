#define SWITCH 6
#define rows_A 1
#define rows_B 2
#define rows_C 3
#define cols_A 4
#define cols_B 5
#define cols_C 6
#define data_pin A5
#include "esp_task_wdt.h"
// The following definitions must be adjusted for the system to run properly

// Represents sensor readings, when it is powered and no pieces are in viscinity
#define empty_field 600
// Minimal difference between an occupied field and an empty field
#define difference 30
// Margin used when classifying a field as an "empty_field"
#define min_diff 10
// Minimal difference between a field occupied by a black piece and a white piece, roughly 2 times the "difference"
#define colorswitch 200
// Field occupied by a black piece
#define BO 440
// Field occupied by a white piece
#define WO 760
// Unoccupied field
#define NO 600
// Button pin
#define button 12

int turn = 0;
String tobesent = "xxxx";
int old_board[8][8]  =
{
  {BO, BO, BO, BO, BO, BO, BO, BO},
  {BO, BO, BO, BO, BO, BO, BO, BO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {WO, WO, WO, WO, WO, WO, WO, WO},
  {WO, WO, WO, WO, WO, WO, WO, WO},
};

int new_board[8][8] =
{
  {BO, BO, BO, BO, BO, BO, BO, BO},
  {BO, BO, BO, BO, BO, WO, BO, BO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {WO, WO, WO, WO, WO, WO, WO, WO},
  {WO, WO, WO, WO, WO, WO, WO, NO},
};

int tempboard[8][8] =
{
  {BO, BO, BO, BO, BO, BO, BO, BO},
  {BO, BO, BO, BO, BO, WO, BO, BO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {NO, NO, NO, NO, WO, NO, NO, NO},
  {NO, NO, NO, NO, NO, NO, NO, NO},
  {WO, WO, WO, WO, NO, WO, WO, WO},
  {WO, WO, WO, WO, WO, WO, WO, NO},
};
/*
The following section of the code (enclosed by a line of slashes -> /////) has originally been created by user rom3 and posted
on hackster.io (https://www.hackster.io/rom3/arduino-uno-micromax-chess-030d7c). Every comment found within this section belongs
to rom3. The base of their solution is shared as an open-source project by H.G. Muller, who posted his work here:https://home.hccnet.nl/h.g.muller/max-src2.html
This project bases heavily on rom3's adaptation of the original chess engine, but has been encapsulated to the size
of a function (named send_to_engine), which is more convenient to use in this project.
*/
//////////////////////////////////////////////////////////////////
#define W while
#define M 0x88
#define S 128
#define I 8000
#define MYRAND_MAX 65535     /* 16bit pseudo random generator */

long  N, T;                  /* N=evaluated positions+S, T=recursion limit */
short Q, O, K, R, k=16;      /* k=moving side */
char *p, c[5], Z;            /* p=pointer to c, c=user input, computer output, Z=recursion counter */
char L,
w[]={0,2,2,7,-1,8,12,23},                             /* relative piece values    */
o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
     7,-1,11,6,8,3,6,                                 /* 1st dir. in o[] per piece*/
     6,3,5,7,4,5,3,6};                                /* initial piece setup      */
													  /* board is left part, center-pts table is right part, and dummy */  

char b[]={     
  22, 19, 21, 23, 20, 21, 19, 22, 28, 21, 16, 13, 12, 13, 16, 21,
  18, 18, 18, 18, 18, 18, 18, 18, 22, 15, 10,  7,  6,  7, 10, 15,
   0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
   0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
   0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
   0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
   9,  9,  9,  9,  9,  9,  9,  9, 22, 15, 10,  7,  6,  7, 10, 15,
  14, 11, 13, 15, 12, 13, 11, 14, 28, 21, 16, 13, 12, 13, 16, 21, 0
};

char bk[16*8+1];
unsigned int seed=0;
uint32_t  byteBoard[8];
char sym[17] = {".?pnkbrq?P?NKBRQ"};
int mn=1;
char lastH[5], lastM[5];
unsigned short ledv=1;
String inputString = "";
bool stringComplete = false;  // whether the string is complete

String send_to_engine(String tempString)
{
  int r;

  c[0] = tempString.charAt(0);
  c[1] = tempString.charAt(1);
  c[2] = tempString.charAt(2);
  c[3] = tempString.charAt(3);
  c[4] = 0;

  K = *c - 16 * c[1] + 799, L = c[2] - 16 * c[3] + 799; /* parse entered move */
  N = 0;
  T = 0x3F;                                 /* T=Computer Play strength */
  bkp();                                    /* Save the board just in case */
  r = D(-I, I, Q, O, 1, 3);                 /* Check & do the human movement */
  if ( !(r > -I + 1) )
  {
    Serial.println("Lose ");
    gameOver();
  }
  if (k == 0x10)
  {                          /* The flag turn must change to 0x08 */
    return "No valid move";
  }
  strcpy(lastH, c);                         /* Valid human movement */

  K = I;
  N = 0;
  T = 0x3F;                                 /* T=Computer Play strength */
  r = D(-I, I, Q, O, 1, 3);                 /* Think & do*/
  if ( !(r > -I + 1) )
  {
    Serial.println("Lose*");
    gameOver();
  }

  strcpy(lastM, c);                         /* Valid ARDUINO movement */
  r = D(-I, I, Q, O, 1, 3);
  if ( !(r > -I + 1) )
  {
    Serial.print("Nie wiem co to ale m: ");
    Serial.println(lastM);
    gameOver();
  }
  return lastM;
}

void getserialchar() {
  while (Serial.available() > 0) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

unsigned short myrand(void)
{
  unsigned short r = (unsigned short)(seed % MYRAND_MAX);
  return r = ((r << 11) + (r << 7) + r) >> 1;
}

short D(short q, short l, short e, unsigned char E, unsigned char z, unsigned char n)
{
  short m, v, i, P, V, s;
  unsigned char t, p, u, x, y, X, Y, H, B, j, d, h, F, G, C;
  signed char r;
  if (++Z > 30) {                                   /* stack underrun check */
    --Z; return e;
  }
  q--;                                          /* adj. window: delay bonus */
  k ^= 24;                                      /* change sides             */
  d = Y = 0;                                    /* start iter. from scratch */
  X = myrand() & ~M;                            /* start at random field    */
  W(d++ < n || d < 3 ||                         /* iterative deepening loop */
    z & K == I && (N < T & d < 98 ||            /* root: deepen upto time   */
                   (K = X, L = Y & ~M, d = 3)))                /* time's up: go do best    */
  { x = B = X;                                   /* start scan at prev. best */
    h = Y & S;                                   /* request try noncastl. 1st*/
    P = d < 3 ? I : D(-l, 1 - l, -e, S, 0, d - 3); /* Search null move         */
    m = -P < l | R > 35 ? d > 2 ? -I : e : -P;   /* Prune or stand-pat       */
    ++N;                                         /* node count (for timing)  */
    do {
      u = b[x];                                   /* scan board looking for   */
      if (u & k) {                                /*  own piece (inefficient!)*/
        r = p = u & 7;                             /* p = piece type (set r>0) */
        j = o[p + 16];                             /* first step vector f.piece*/
        W(r = p > 2 & r < 0 ? -r : -o[++j])        /* loop over directions o[] */
        { A:                                        /* resume normal after best */
          y = x; F = G = S;                         /* (x,y)=move, (F,G)=castl.R*/
          do {                                      /* y traverses ray, or:     */
            H = y = h ? Y ^ h : y + r;               /* sneak in prev. best move */
            if (y & M)break;                         /* board edge hit           */
            m = E - S & b[E] && y - E < 2 & E - y < 2 ? I : m; /* bad castling             */
            if (p < 3 & y == E)H ^= 16;              /* shift capt.sqr. H if e.p.*/
            t = b[H]; if (t & k | p < 3 & !(y - x & 7) - !t)break; /* capt. own, bad pawn mode */
            i = 37 * w[t & 7] + (t & 192);           /* value of capt. piece t   */
            m = i < 0 ? I : m;                       /* K capture                */
            if (m >= l & d > 1)goto C;               /* abort on fail high       */
            v = d - 1 ? e : i - p;                   /* MVV/LVA scoring          */
            if (d - !t > 1)                          /* remaining depth          */
            { v = p < 6 ? b[x + 8] - b[y + 8] : 0;    /* center positional pts.   */
              b[G] = b[H] = b[x] = 0; b[y] = u | 32;  /* do move, set non-virgin  */
              if (!(G & M))b[F] = k + 6, v += 50;     /* castling: put R & score  */
              v -= p - 4 | R > 29 ? 0 : 20;           /* penalize mid-game K move */
              if (p < 3)                              /* pawns:                   */
              { v -= 9 * ((x - 2 & M || b[x - 2] - u) + /* structure, undefended    */
                          (x + 2 & M || b[x + 2] - u) - 1  /*        squares plus bias */
                          + (b[x ^ 16] == k + 36))          /* kling to non-virgin King */
                     - (R >> 2);                       /* end-game Pawn-push bonus */
                V = y + r + 1 & S ? 647 - p : 2 * (u & y + 16 & 32); /* promotion or 6/7th bonus */
                b[y] += V; i += V;                     /* change piece, add score  */
              }
              v += e + i; V = m > q ? m : q;          /* new eval and alpha       */
              C = d - 1 - (d > 5 & p > 2 & !t & !h);
              C = R > 29 | d < 3 | P - I ? C : d;     /* extend 1 ply if in check */
              do
                s = C > 2 | v > V ? -D(-l, -V, -v,     /* recursive eval. of reply */
                                       F, 0, C) : v;    /* or fail low if futile    */
              W(s > q&++C < d); v = s;
              if (z && K - I && v + I && x == K & y == L) /* move pending & in root:  */
              { Q = -e - i; O = F;                     /*   exit if legal & found  */
                R += i >> 7; --Z; return l;            /* captured non-P material  */
              }
              b[G] = k + 6; b[F] = b[y] = 0; b[x] = u; b[H] = t; /* undo move,G can be dummy */
            }
            if (v > m)                               /* new best, update max,best*/
              m = v, X = x, Y = y | S & F;            /* mark double move with S  */
            if (h) {
              h = 0;  /* redo after doing old best*/
              goto A;
            }
            if (x + r - y | u & 32 |                 /* not 1st step,moved before*/
                p > 2 & (p - 4 | j - 7 ||             /* no P & no lateral K move,*/
                         b[G = x + 3 ^ r >> 1 & 7] - k - 6     /* no virgin R in corner G, */
                         || b[G ^ 1] | b[G ^ 2])               /* no 2 empty sq. next to R */
               )t += p < 5;                           /* fake capt. for nonsliding*/
            else F = y;                              /* enable e.p.              */
          } W(!t);                                  /* if not capt. continue ray*/
        }
      }
    } W((x = x + 9 & ~M) - B);                 /* next sqr. of board, wrap */
C: if (m > I - M | m < M - I)d = 98;           /* mate holds to any depth  */
    m = m + I | P == I ? m : 0;                  /* best loses K: (stale)mate*/
    if (z && d > 2)
    { *c = 'a' + (X & 7); c[1] = '8' - (X >> 4); c[2] = 'a' + (Y & 7); c[3] = '8' - (Y >> 4 & 7); c[4] = 0;
      char buff[150];
    }
  }                                             /*    encoded in X S,8 bits */
  k ^= 24;                                      /* change sides back        */
  --Z; return m += m < e;                       /* delayed-loss bonus       */
}
void gameOver()
{
  for (;;);
}
void bkp()
{
  for (int i = 0; i < 16 * 8 + 1; i++) {
    bk[i] = b[i];
  }
}
//////////////////////////////////////////////////////////////////////

// Sets every field of a chosen board to 0
void clearBoard(int (*array)[8])
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            array[i][j] = 0;
        }
    }
}

// Displays the current state of the chosen board
void printBoard(int (*array)[8])
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            Serial.print(array[i][j]);
            Serial.print("  ");
        }
        Serial.print("\n\n");
    }
    Serial.print("\n\n\n");
}

// Transfers the first declared board to the second one. Primarily used to update the older board and should look like this: transferBoard(new_board, old_board)
void transferBoard(int (*array1)[8], int (*array2)[8])
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            array2[i][j]= array1[i][j];
        }
    }
}

//Focus on tile specified with x(0-7) and y(0-7) with the "reading" variable toggling between sensor reading (1) or turning LED on (0)
int focus(int x, int y, int reading)
{
  bitRead(x, 2) == 1? digitalWrite(rows_A, HIGH) : digitalWrite(rows_A, LOW);
  bitRead(x, 1) == 1? digitalWrite(rows_B, HIGH) : digitalWrite(rows_B, LOW);
  bitRead(x, 0) == 1? digitalWrite(rows_C, HIGH) : digitalWrite(rows_C, LOW);
  
  bitRead(y, 2) == 1? digitalWrite(cols_A, HIGH) : digitalWrite(cols_A, LOW);
  bitRead(y, 1) == 1? digitalWrite(cols_B, HIGH) : digitalWrite(cols_B, LOW);
  bitRead(y, 0) == 1? digitalWrite(cols_C, HIGH) : digitalWrite(cols_C, LOW);

  if(reading==1)return analogRead(data_pin);
  else return 0;
}

//Get sensor readings from the entire board and put them in the chosen board (new_board)
void getReadings(int (*array)[8])
{
	digitalWrite(SWITCH, HIGH);
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            array[i][j] = focus(i,j,1);
        }
    }
	digitalWrite(SWITCH, LOW);
}

// Creates a notation move, depending on changes between the old and new board, possible board changes described at the end of the code.
// The task is to indentify the coordinates of two key fields - the starting field of the move (FROM, from_letter, from_digit) and
// the ending field (TO, to_letter, to_digit).
void moveConversion(int (*array1)[8], int (*array2)[8])
{
  String from_letter = "";
  String to_letter = "";
  int from_digit = 0;
  int to_digit = 0;
    for (int i = 0; i < 8; ++i)
    {
      for (int j = 0; j < 8; ++j)
      {
        // FROM: if a tile represents a lower (black) or higher (white) readings in older board, there is a piece there.
        // Now if the newer board shows an empty tile in this place, this means a piece was removed from the tile.
        // Thus, this solution is universal for both the "free move" and the "capture" scenario.
        if((abs(array1[i][j] - empty_field) >= difference) && (abs(array2[i][j] - empty_field) <= min_diff))
        {
          if (j==0) from_letter = "a";
          if (j==1) from_letter = "b";
          if (j==2) from_letter = "c";
          if (j==3) from_letter = "d";
          if (j==4) from_letter = "e";
          if (j==5) from_letter = "f";
          if (j==6) from_letter = "g";
          if (j==7) from_letter = "h";
          from_digit = 8-i;
          Serial.print("Found the FROM move: ");
          Serial.print(from_letter);
          Serial.println(from_digit);
          tobesent[0] = from_letter[0];
          tobesent[1] = from_digit+'0';
        }
        
        // TO: conditions vary between scenarios and will be considered separately:
        
        // FREE MOVE: if a place appears empty in the first board and shows a change in readings in the second board,
        // it means a piece has just appeared there due to a free move.
        if((abs(array1[i][j] - empty_field) <= min_diff) && (abs(array2[i][j] - empty_field) >= difference))
        {
          if (j==0) to_letter = "a";
          if (j==1) to_letter = "b";
          if (j==2) to_letter = "c";
          if (j==3) to_letter = "d";
          if (j==4) to_letter = "e";
          if (j==5) to_letter = "f";
          if (j==6) to_letter = "g";
          if (j==7) to_letter = "h";
          to_digit = 8-i;
          Serial.print("Found the TO move (free move): ");
          Serial.print(to_letter);
          Serial.println(to_digit);
          tobesent[2] = to_letter[0];
          tobesent[3] = to_digit+'0';
        }
        
        // CAPTURE: if a tile readings turned opposite - e.g. been high and turned low or vice versa, it means the piece
        // occupying this tile has just been captured. In other words, a capture event occured if the difference in readings
        // is comparable to 2x difference variable value (or the colorswitch variable).
        if(abs(array1[i][j] - array2[i][j]) >= colorswitch)
        {
          if (j==0) to_letter = "a";
          if (j==1) to_letter = "b";
          if (j==2) to_letter = "c";
          if (j==3) to_letter = "d";
          if (j==4) to_letter = "e";
          if (j==5) to_letter = "f";
          if (j==6) to_letter = "g";
          if (j==7) to_letter = "h";
          to_digit = 8-i;
          Serial.print("Found the TO move (capture): ");
          Serial.print(to_letter);
          Serial.println(to_digit);
          tobesent[2] = to_letter[0];
          tobesent[3] = to_digit+'0';
        }
        // At this point, both FROM and TO spots should be declared with no conflicts, and the "tobesent" variable is fully constructed
      }
    } 
    Serial.print("Move constructed: ");
    Serial.print(tobesent);
    tobesent = "xxxx";
}

// A visual cue for the user, portraying the opponent's move
void oppMove(String move)
{
  int coordFx = 0;
  int coordFy = move[1] - '0';
  int coordTx = 0;
  int coordTy = move[3] - '0';
  coordFy--;
  coordTy--;
  
  if (move[0] == 'a') coordFx = 0;
  if (move[0] == 'b') coordFx = 1;
  if (move[0] == 'c') coordFx = 2;
  if (move[0] == 'd') coordFx = 3;
  if (move[0] == 'e') coordFx = 4;
  if (move[0] == 'f') coordFx = 5;
  if (move[0] == 'g') coordFx = 6;
  if (move[0] == 'h') coordFx = 7;
  
  if (move[2] == 'a') coordTx = 0;
  if (move[2] == 'b') coordTx = 1;
  if (move[2] == 'c') coordTx = 2;
  if (move[2] == 'd') coordTx = 3;
  if (move[2] == 'e') coordTx = 4;
  if (move[2] == 'f') coordTx = 5;
  if (move[2] == 'g') coordTx = 6;
  if (move[2] == 'h') coordTx = 7;
  
  for(int i=0; i<3; i++)
  {
    focus(coordFx, coordFy, 0);
    delay(500);
    focus(coordTx, coordTy, 0);
    delay(500);
  }
  Serial.print("Received OPP move: ");
  Serial.println(move);
  Serial.print(coordFx);
  Serial.print(" ");
  Serial.println(coordFy);
  Serial.print(coordTx);
  Serial.print(" ");
  Serial.println(coordTy);
  Serial.println();
}

// Flash every diode in a sequence, used as a signal that the game has started or ended
void flashySequence()
{
    for (int i = 0; i < 8; ++i)
    {
      for (int j = 0; j < 8; ++j)
      {
		focus(i, j, 0);
		delay(50);		
	  }
	}
}


// Game routine, invoked everytime a button is pressed
void gameRoutine()
{
	if(turn==0) // Player's turn, button pressed after making a move
	{
		turn = 1;
		// Scan the board to find out what has changed
		getReadings(new_board);
		// Construct the move
		moveConversion(old_board, new_board);
		// Signalize opponent's move, calculated by the engine
		oppMove(send_to_engine(tobesent));
	}
	else // Opponent's turn
	{
		turn = 0;
		// Update the boards and scan the table to update it after opponent's move has been made
		transferBoard(new_board, old_board);
		getReadings(new_board);
		// Now wait for the player to make a move and press button again
	}
}


void setup()
{
	/*
	Setup tutorial:
	1. Clear both of the boards just in case, then fill the old board with actual sensor readings
	2. Set up Serial protocol and every mandatory variable
	3. Set up every used port
	4. Greet the user with a flashy LED sequence
	*/
	esp_task_wdt_delete(NULL);
	Serial.begin(9600);
	lastH[0] = 0;
	pinMode(data_pin, INPUT_PULLUP);
	pinMode(SWITCH, OUTPUT);
	pinMode(rows_A, OUTPUT);
	pinMode(rows_B, OUTPUT);
	pinMode(rows_C, OUTPUT);
	pinMode(cols_A, OUTPUT);
	pinMode(cols_B, OUTPUT);
	pinMode(cols_C, OUTPUT);

	digitalWrite(SWITCH, LOW);
	digitalWrite(rows_A, LOW);
	digitalWrite(rows_B, LOW);
	digitalWrite(rows_C, LOW);
	digitalWrite(cols_A, LOW);
	digitalWrite(cols_B, LOW);
	digitalWrite(cols_C, LOW);

	delay(1000);
	clearBoard(old_board);
	clearBoard(new_board);
	getReadings(old_board);
	delay(500);
}

void loop()
{
  /*
  Game's algorithm
  1. Signalize the start of the game using a flashy LED sequence
  2. Wait for the player to make a move
  3. Player presses the button
  4. Get all sensor readings
  5. Construct the move and send it to the engine, wait for the response
  6. Translate the response move to a visual LED cue
  7. Wait for the player to make the opponents move and press the button again
  8. Go back to point 2.
  ...
  9. Game ended, play another flashy LED sequence
  */
  delay(1000);
  flashySequence();
  
  while(1) // Unless no winner has emerged
  {
    // If player presses the button, take action depending on the state of the turn variable
    if(digitalRead(button)==LOW)
    {
      gameRoutine();
      delay(1000);
    }
  }
}

/*
	TO AND FROM SPOTS ALGORITHM EXPLAINED
	Let's consider the following symbols and their meaning, as a reference to the sensor readings value:
	^ - high value, occuring when a WHITE piece is detected,
	v - low value, occuring when a BLACK piece is detected,
	o - empty field value, occuring when no piece is detected,
	> - transition from the state observed in the old board to the state oberved in the new board
	This gives us the possibility to create a chart that covers every possible change in the board:
	
	---------------------------------------
	|  FREE MOVE  |  CAPTURE  | DIRECTION |
	---------------------------------------
	|             |           |           |
	|    v > o    |   v > o   |    FROM   |
	|             |           |           |
	|    o > v    |   ^ > v   |     TO    |
	|             |           |           |
	---------------------------------------
	|             |           |           |
	|    ^ > o    |   ^ > o   |    FROM   |
	|             |           |           |
	|    o > ^    |   v > ^   |     TO    |
	|             |           |           |
	---------------------------------------
	
	Basing on this chart, appropriate IF statements can be constructed, without any risk of them colliding.
	Two additional values are crucial - the minimal difference in readings between and empty and an occupied tile (difference),
	and the minimal difference that helps detect empty tiles (min_diff). Both of these should be adjusted experimentally.
*/