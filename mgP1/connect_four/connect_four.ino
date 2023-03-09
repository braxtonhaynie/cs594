//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

// sensor pins
const int trigPin = 2;  
const int echoPin = 3; 
float duration;
float distance;


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
    void print_piece(int player, uint8_t col);
    void win_routine();
    void check_board_count();
  private:
    short board[8][8];
    uint8_t piece_count[8];
    uint8_t p1[8];
    uint8_t p2[8];
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

  // temporary replacement to test dropping pieces
  // player_board[c] &= ~(MSB >> piece_loc);
  player_board[c] &= 0b11111110;
  piece_count[c]++;
  return true;
}

// player should be 1 or 2 to indicate player #
void Game::drop_piece(uint8_t player) {
  uint8_t *player_board;
  uint8_t *other_player_board;
  if (player == 1) {
    player_board = p1;
    other_player_board = p2;
  }
  else if (player == 2) {
    player_board = p2;
    other_player_board = p1;
  }
  else return;

  for (int i = 0; i < 8; i++) {
    int8_t msb = 0b1 << 7;
    int8_t piece_counter = (piece_count[i] != 0) ? msb >> (piece_count[i] - 1) : 0;
    if (piece_counter == 0) continue;
    byte board_col = player_board[i] & other_player_board[i];
    if (board_col == ~(msb >> (piece_count[i] - 1))) continue;

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

  int delay_sensor_check = 0;
  float prev_distance = 0;
  int soundcount = 0;
  int drop_piece_count = 0;
  while (true) {
    // print board and new piece
    print_piece(turn, new_piece_col);
    if (drop_piece_count == 8) {
      drop_piece(1);
      drop_piece(2);
      drop_piece_count = -1;
    }
    drop_piece_count++;

    if  (delay_sensor_check == 50){
      digitalWrite(trigPin, HIGH);  
      delayMicroseconds(2);  
      digitalWrite(trigPin, LOW);  

      duration = pulseIn(echoPin, HIGH);
      prev_distance = distance;
      distance = (duration*.0343)/2;
      
      int diff = distance - prev_distance;

      if (abs(diff) > 70){
        distance = prev_distance;
      }

      if(distance > 300){
        distance = prev_distance;
      }

      delay_sensor_check = 0;
    }
    delay_sensor_check++;
    int sensorValue = analogRead(A0);
    show_board();
    soundcount++;
    if((sensorValue > 590) and (soundcount > 50)){
      if (!add_piece(turn, new_piece_col)) continue;
      turn = (turn == 1) ? 2 : 1;
      soundcount = 0;
      return new_piece_col;
    }
    else if((distance > 0) and (distance <=20) ){
      new_piece_col = 0;
    } 
    else if((distance > 20) and (distance <= 40) ){
      new_piece_col = 1;
    }
    else if ((distance > 40) and (distance <= 60))  {
      new_piece_col = 2;
    }    
    else if ((distance > 60) and (distance <= 80))  {
      new_piece_col = 3;

    } 
    else if ((distance > 80) and (distance <= 100))  {
      new_piece_col = 4;

    } 
    else if ((distance > 100) and (distance <= 120))  {
      new_piece_col = 5;

    } 
    else if ((distance > 120) and (distance <= 140))  {
      new_piece_col = 6;

    } 
    else if ((distance > 140) and (distance <=160))  {
      new_piece_col = 7;
    }
    Serial.println(sensorValue);
  }
}

// prints game board with piece at the top
void Game::print_piece(int player, uint8_t col) {
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
  byte p1_win[8] = {
    0b11111111,
    0b11111011,
    0b11111001,
    0b00000000,
    0b00000000,
    0b11111111,
    0b11111111,
    0b11111111
  };
  byte p2_win[8] = {
    0b11111111,
    0b00111001,
    0b00011000,
    0b00001100,
    0b00100100,
    0b00110000,
    0b00111001,
    0b11111111
  };
  int show_counter = 0;
  while(true) {
    if (show_counter < 200) {
      drop_piece(1);
      drop_piece(2);    
      if (turn == 1) {
        byte row = 0b10000000;
        for(int i = 0; i < 8; i++) {
          digitalWrite(latchPin, LOW);
          // shift out the bits:
          shiftOut(dataPin, clockPin, 1, p2_win[i]);
          shiftOut(dataPin, clockPin, 1, 255);
          shiftOut(dataPin, clockPin, 1, row);
          row >>= 1;
          //take the latch pin high so the LEDs will light up:
          digitalWrite(latchPin, HIGH);
        }
      }
      else {
        byte row = 0b10000000;
        for(int i = 0; i < 8; i++) {
          digitalWrite(latchPin, LOW);
          // shift out the bits:
          shiftOut(dataPin, clockPin, 1, 255);
          shiftOut(dataPin, clockPin, 1, p1_win[i]);
          shiftOut(dataPin, clockPin, 1, row);
          row >>= 1;
          //take the latch pin high so the LEDs will light up:
          digitalWrite(latchPin, HIGH);
        }
      }
    }
    else {
      show_board();
      if(show_counter > 400) show_counter = 0;
    }
    show_counter++;
  }
}

void Game::check_board_count() {
  uint8_t count = 0;
  for(int i = 0; i < 8; i++) {
    if (piece_count[i] == 8) count++;
  }
  if (count == 8) {
    count = 0;
    while(true) {
      if (count < 200)
        show_board();
      else if (count < 400){
        count = 0;
      }
      count++;
    }
  }
}

Game g;
void setup() {
//set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  // sensor input pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin,INPUT);

  Serial.begin(9600);
  g = Game();
  g.clear_board();
}

int delay_adding = 0;
void loop() {
// the game logic is here
  g.show_board();
  int piece_col = g.player_turn();

  if (g.check_for_win(piece_col)) {
    g.win_routine();
  }
  g.check_board_count();
}

