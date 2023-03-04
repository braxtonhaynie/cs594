//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

// button pins
int left = 7;
int drop = 6;
int right = 5;

/*
TODO: 
-- fix add_piece
-- document how the functions work
-- 
*/

class Game
{
  public:
    Game() { turn = 1;};
    void clear_board();
    void show_board();
    void print_board();
    void p1_plays();
    void p2_plays();
    int check_for_win(int c);
    int check_col(int c);
    bool add_piece(uint8_t player, int c);
    void drop_piece(uint8_t player);
    int player_turn();
    void print_piece(int player, uint8_t row, uint8_t col);
    void win_routine();
  private:
    short board[8][8];
    uint8_t piece_count[8];
    uint8_t p1[8];
    int8_t p1_piece_count[8];
    uint8_t p2[8];
    int8_t p2_piece_count[8];
    int turn;
};

// clears the game board
void Game::clear_board() {
  for (int i = 0; i < 8; i++) {
    piece_count[8] = 0;
    for (int j = 0; j < 8; j ++) {
      board[i][j] = 0;
    }
  }
  // not sure why but resetting the boards in the upper loop was not working
  for(int i = 0; i < 8; i++) {
    p1[i] = 0xff;
    p2[i] = 0xff;
  }
}

// prints board to LED matrix
void Game::show_board() {
  byte row = 0b10000000;
  for(int i = 0; i < 8; i++) {
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 1, 255);
    shiftOut(dataPin, clockPin, 1, p1[i]);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
  row = 0b10000000;
  for(int i = 0; i < 8; i++) {
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 1, p2[i]);
    shiftOut(dataPin, clockPin, 1, 255);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
}

// print the board to serial output
void Game::print_board() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Serial.print(board[i][j]);
      Serial.print(' ');
    }
    Serial.println();
  }
}

// checks that the given coordinate is within the board
int isValidCoord ( int x, int y ) {
  return x >= 0 && x < 8 && y >= 0 && y < 8;
}

// checks the board for a winner
int Game::check_for_win(int c) {
  int HEIGHT = 8;
  int WIDTH = 8;
    for (int r = 0; r < HEIGHT; r++) { // iterate rows, bottom to top
      for (int c = 0; c < WIDTH; c++) { // iterate columns, left to right
        int player = board[r][c];
        if (player == 0) continue; // don't check empty slots
        if (c + 3 < WIDTH && player == board[r][c+1] && player == board[r][c+2] && player == board[r][c+3]) // look right 
          return player;
        if (r + 3 < HEIGHT) {
          if (player == board[r+1][c] && player == board[r+2][c] && player == board[r+3][c]) // look up 
            return player;
          if (c + 3 < WIDTH && player == board[r+1][c+1] && player == board[r+2][c+2] && player == board[r+3][c+3]) // look up & right
            return player;
          if (c - 3 >= 0 && player == board[r+1][c-1] && player == board[r+2][c-2] && player == board[r+3][c-3]) // look up & left
            return player;
      }
    }
  }
  return 0;
}

// checks the column and returns the location of the next piece
int Game::check_col(int c) {
  // check if column is full
  if (piece_count[c] == 8) return -1;

  // get location of next piece in column
  for (int i = 0; i < 8; i++) {
    if (board[c][i] == 0) return i;
  }
  return -1;
}

// player should be 1 or 2 to indicate player #
bool Game::add_piece(uint8_t player, int c) {
  int piece_loc;
  uint8_t *player_board;
  uint8_t MSB = (0b1 << 7); // don't want to shift 1s down

  if (c < 0 || c > 7) return false;
  piece_loc = check_col(c);
  if (piece_loc == -1) return false;

  // get player board
  if (player == 1) {
    player_board = p1;
  }
  else if (player = 2) {
    player_board = p2;
  }
  else return false;

  // add piece to the board
  board[c][piece_loc] = player;
  player_board[c] &= ~(MSB >> piece_loc);
  Serial.println(player_board[c]);
  Serial.println(~(MSB >> piece_loc));

  return true;
}

/*
// player should be 1 or 2 to indicate player #
void Game::add_piece(uint8_t player, int c) {
  uint8_t *player_board;
  int8_t *piece_count;
  if (player == 1) {
    player_board = p1;
    piece_count = p1_piece_count;
  }
  else if (player = 2) {
    player_board = p2;
    piece_count = p2_piece_count;
  }
  else return;
  if (c > 7 || c < 0 || player_board[c] == 0 || piece_count[c] == 8) return;
  player_board[c] &= 0xfe;
  piece_count[c]++;
}
*/

// player should be 1 or 2 to indicate player #
void Game::drop_piece(uint8_t player) {
  uint8_t *player_board;
  int8_t *piece_count;
  if (player == 1) {
    player_board = p1;
    piece_count = p1_piece_count;
  }
  else if (player = 2) {
    player_board = p2;
    piece_count = p2_piece_count;
  }
  else return;

  for (int i = 0; i < 8; i++) {
    int8_t msb = 0b1 << 7;
    int8_t piece_counter = (piece_count[i] != 0) ? msb >> (piece_count[i] - 1) : 0;
    if (piece_counter == 0) continue;
    if (player_board[i] == ~(msb >> (piece_count[i] - 1))) continue;

    // get location of droping pixel
    byte tmp = player_board[i] | (piece_counter);
    tmp = (tmp << 1) + 1; // shift down and add 1 to shift in 1s
    player_board[i] = tmp & (player_board[i] | ~piece_counter);
  }
}

// allows player to play their turn
int Game::player_turn() {
  // generate new piece in center
  uint8_t new_piece_row = 0b00010000;
  uint8_t new_piece_col = 3;

  int delay_button_check = 0;
  while (true) {
    // print board and new piece
    print_piece(turn, new_piece_row, new_piece_col);

    // player can move the piece with buttons
    if (delay_button_check == 15) {
      if (digitalRead(right) == HIGH && new_piece_col < 7) {
        new_piece_row >>= 1;
        new_piece_col++;
      }
      else if (digitalRead(left) == HIGH && new_piece_col > 0) {
        new_piece_row <<= 1;
        new_piece_col--;
      }
      else if (digitalRead(drop) == HIGH) {
        if (!add_piece(turn, new_piece_col)) continue;
        turn = (turn == 1) ? 2 : 1;
        return new_piece_col;
      }
      delay_button_check = 0;
    }
    delay_button_check++;
  }
}

// prints game board with piece at the top
void Game::print_piece(int player, uint8_t new_row, uint8_t col) {
  byte player_1 = 255;
  byte player_2 = 255;
  if (player == 1) player_1 = p1[col] & 0b11111110;
  else player_2 = p2[col] & 0b11111110;
  byte row = 0b10000000;
  byte print_byte = 0xff;
  for(int i = 0; i < 8; i++) {
    if (i == col && player_1 != 0xff) print_byte = player_1;
    else print_byte = p1[i];
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 1, 255);
    shiftOut(dataPin, clockPin, 1, print_byte);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
  row = 0b10000000;
  for(int i = 0; i < 8; i++) {
    if (i == col && player_2 != 0xff) print_byte = player_2;
    else print_byte = p2[i];
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 1, print_byte);
    shiftOut(dataPin, clockPin, 1, 255);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
}

void Game::win_routine() {

}

Game g;
void setup() {
//set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(right, INPUT);
  pinMode(drop, INPUT);
  pinMode(left, INPUT);
  Serial.begin(9600);
  g = Game();
  g.clear_board();
}

int delay_adding = 0;
void loop() {

  g.show_board();

  // player_turn();
  // this will show the piece at the top of the board
  // it will also get the location of the piece

  int piece_col = g.player_turn();

  Serial.print("checking for a win: ");
  Serial.println(g.check_for_win(piece_col));
  
  /*
  if (delay_adding == 250) {
  if (g.check_col(2) == 0) {
    g.add_piece(2 , 2);
  }
  else if (g.check_col(2) == 1) {
    g.add_piece(1, 2);
  }
  else if (g.check_col(5) == 0) {
    g.add_piece(2, 5);
    g.add_piece(2, 5);
  }
  delay_adding = 0;
  }
  delay_adding++;
  */
}
