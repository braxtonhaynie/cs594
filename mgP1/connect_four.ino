
//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

class Game
{
  public:
    Game() { turn = 0;};
    void clear_board();
    // void show_board();
    void print_board();
    void p1_plays();
    void p2_plays();
    void add_piece(int c);
    void drop_piece(uint8_t player);
    short board[8][8];
    uint8_t p1[8];
    int8_t p1_piece_shifter[8];
    int8_t p1_piece_count[8];
    uint8_t p2[8];
    int8_t p2_piece_shifter[8];
    int8_t p2_piece_count[8];
    int turn;
};

void Game::clear_board() {
  for (int i = 0; i < 8; i++) {
    p1[i] = 255;
    p1_piece_count[i] = 0;
    p1_piece_shifter[i] = 0;
    p2[i] = 255;
    p2_piece_count[i] = 0;
    p2_piece_shifter[i] = 0;
    for (int j = 0; j < 8; j ++) {
      board[i][j] = 0;
    }
  }
}


void Game::print_board() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Serial.print(board[i][j]);
      Serial.print(' ');
    }
    Serial.println();
  }
}

void Game::add_piece(int c) {
  if (c > 8 || c < 0 || p2[c] == 0 || p2_piece_count[c] == 8) return;
  p2[c] &= 0xfe;
  p2_piece_shifter[c] = p2_piece_shifter[c] | (0b1 << (7-p2_piece_count[c]));
  p2_piece_count[c]++;
}

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
    if (p2[i] == ~(msb >> (piece_count[i] - 1))) continue;

    // get location of droping pixel
    byte tmp = p2[i] | (piece_counter);
    tmp = (tmp << 1) + 1; // shift down and add 1 to shift in 1s
    p2[i] = tmp & (p2[i] | ~piece_counter);
  }
}

short board[8][8] = {
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,1,0,0,0,0},
{0,2,1,1,0,0,0,0},
{2,1,2,2,0,0,0,0},
{1,1,2,2,0,0,0,0}
};

byte p1[8] = {
0b01111111,
0b00111111,
0b11011111,
0b10001111,
0b11111111,
0b11111111,
0b11111111,
0b11111111
};

byte p2[8] = {
0b10011111,
0b11011111,
0b00111111,
0b01111111,
0b11111111,
0b11111111,
0b11111111,
0b11111111
};

byte ok[8] = {
0b10011111,
0b01101001,
0b01101001,
0b10011111,
0b00001111,
0b10010110,
0b01101001,
0b11110000
};

Game g;
void setup() {
//set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
  g = Game();
  g.clear_board();
  g.p1[0] = 0b00111111;
  g.add_piece(2);
}


void show_board() {
  byte row = 0b10000000;
  for(int i = 0; i < 8; i++) {
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 0, 255);
    shiftOut(dataPin, clockPin, 1, g.p1[i]);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
  row = 0b10000000;
  for(int i = 0; i < 8; i++) {
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 1, g.p2[i]);
    shiftOut(dataPin, clockPin, 0, 255);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
}

int drop_count = 0;
void loop() {
  /*
  for(int i = 0; i < 8; i++) {
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, 0, 255);
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
    shiftOut(dataPin, clockPin, 0, 255);
    shiftOut(dataPin, clockPin, 1, row);
    row >>= 1;
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
  }
*/

  show_board();
  if (drop_count == 100) {
    g.drop_piece(2);
    drop_count = 0;
  }
  drop_count++;

  if (g.p2[2] == 0b01111111) {
    g.add_piece(2);
  }
  else if (g.p2[2] == 0b00111111) {
    g.add_piece(2);
  }


}