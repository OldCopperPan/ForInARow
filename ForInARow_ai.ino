#include <rgbMatrix.h>


//**************************************** CONFIG ****************************************

// Game Select
byte game       = 1;                   // 0 - Human     1 - Computer
byte gameSelect = 1;                   // 0 - Random    1 - AI (based on simple scoring)

// Rotary Switch       
#define SW 9                           // the number of the pushbutton pin

// Rotary Encoder
#define CLK 12                         // the number of the Clock pin
#define DT 10                          // the number of the Data pin

//****************************************************************************************

// BACKGROUND -  BLACK
byte led_R = 0;
byte led_G = 0;
byte led_B = 0;

// Board - BLUE
byte board_LED_R = 0;
byte board_LED_G = 0;
byte board_LED_B = 2;

// Player 1 - RED
byte playerONE_LED_R = 8;
byte playerONE_LED_G = 0;
byte playerONE_LED_B = 0;

// Player 2 - YELLOW
byte playerTWO_LED_R = 8;
byte playerTWO_LED_G = 8;
byte playerTWO_LED_B = 0;

byte playerTurn = 1;                   // Player 1 starts the game

byte dot[][2]    = { {0, 7}, {0, 6}, {0, 5}, {0, 4}, {0, 3}, {0, 2}, {0, 1}, {0, 0} };  // Matrix top row
byte topLine[4]  = {0,1,0,7};         // Top row of the game (8x8 matrix), used to select columns

byte board[6][7] = {                   // Empty board
     {0, 0, 0, 0, 0, 0, 0},            // Row 1 (Top row)
     {0, 0, 0, 0, 0, 0, 0},            // Row 2
     {0, 0, 0, 0, 0, 0, 0},            // Row 3
     {0, 0, 0, 0, 0, 0, 0},            // Row 4
     {0, 0, 0, 0, 0, 0, 0},            // Row 5
     {0, 0, 0, 0, 0, 0, 0}             // Row 6 (Bottom row) 
};

byte board_copy[6][7] = {              // Empty board copy (to check next AI move)
     {0, 0, 0, 0, 0, 0, 0},            // Row 1 (Top row)
     {0, 0, 0, 0, 0, 0, 0},            // Row 2
     {0, 0, 0, 0, 0, 0, 0},            // Row 3
     {0, 0, 0, 0, 0, 0, 0},            // Row 4
     {0, 0, 0, 0, 0, 0, 0},            // Row 5
     {0, 0, 0, 0, 0, 0, 0}             // Row 6 (Bottom row) 
};

byte game_win = 0;                     // 0 - No Win    1 - Player 1 Win   2 - Player 2 Win

// Rotary Switch       
unsigned long lastButtonPress = 0;     // Button press time passed 

// Rotary Encoder
int counter = 0;                       // counts ClockWise or Counter ClockWise pulses
int currentStateCLK;                   // current CLK status
int lastStateCLK;                      // last state CLK status

void (* resetFunc) (void) = 0;         // Reset Arduino


void setup() {
  Serial.begin(9600);
	pinMode(CLK,INPUT);                  // Rotary Encoder clock pin (CLK)
	pinMode(DT,INPUT);                   // Rotary Encoder data pin (Data)
	pinMode(SW, INPUT_PULLUP);           // Rotary Encoder switch pin (SW)
  RGBMatrixInit();	                   // Initialize SunFounder RGB Matrix
  randomSeed(analogRead(0));           // Randomize using noise from analog pin 0   
	lastStateCLK = digitalRead(CLK);     // Read the initial state of CLK
  byte playerTurn = 1;                 // Player 1 starts the game
  led_R = playerONE_LED_R;             // Pixel RGB - Player 1 Red value
  led_G = playerONE_LED_G;             // Pixel RGB - Player 1 Green value
  led_B = playerONE_LED_B;             // Pixel RGB - Player 1 Blue value
  byte rectangle[4] = {2, 1, 7, 7};    // Board
  draw_rectangle(rectangle, board_LED_R, board_LED_G, board_LED_B);  // Board color BLUE
  image();                             // Write to matrix
  draw_point(dot[counter], led_R, led_G, led_B);                     // Pixel Player 1 starts on top left position
  image();                             // Write to matrix
}




void loop() {
static uint16_t state=0;
 
if (playerTurn == 1)   {   led_R = playerONE_LED_R; led_G = playerONE_LED_G; led_B = playerONE_LED_B;   } // Set pixel colors for Player 1
if (playerTurn == 2)   {   led_R = playerTWO_LED_R; led_G = playerTWO_LED_G; led_B = playerTWO_LED_B;   } // Set pixel colors for Player 2

// Read Rotary Encoder
    state=(state<<1) | digitalRead(CLK) | 0xe000;     // Digital Debounce Filter
    // This is a very compact filter - each time round the loop a new bit is shifted 
    // left (at bit 0). The "or" action with 0xe000 defines the number of iterations 
    // i.e. the top 3 bits are blocked off leaving the rest as useful inputs. The idea 
    // is that you test for the state 0xf000 which can only occur if there was a sequence 
    // of 1 0000 0000 0000 inputs meaning that the signal has been stable for 12 iterations 
    // around the loop i.e. not bouncing around.    

    if (state==0xf000)  {
       state=0x0000;
       if(digitalRead(DT)) {
         draw_point(dot[counter], 0, 0, 0);                                        // Turn off current pixel
         image();                                                                  // Write to matrix
			   counter++;                                                                // Player pixel ClockWise
         if (counter == 7) { counter = 6; }                                        // Column boundary
         draw_point(dot[counter], led_R, led_G, led_B);                            // Turn on next pixel
         image();                                                                  // Write to matrix                                
       }
       else {
         draw_point(dot[counter], 0, 0, 0);                                        // Turn off current pixel
         image();                                                                  // Write to matrix       
			   counter--;                                                                // Player pixel Counter ClockWise
         if (counter == -1) { counter = 0; }                                       // Column boundary
         draw_point(dot[counter], led_R, led_G, led_B);                            // Turn on previous pixel
         image();                                                                  // Write to matrix 
       }
    }

// Read Switch
	int btnState = digitalRead(SW);       	                                         // Read the button state

	//If LOW signal detected, button is pressed
	if (btnState == LOW) {
		//If 50ms have passed since last LOW pulse, it means that the button has been pressed, released and pressed again
	
		if (millis() - lastButtonPress > 50) {
      if (game == 0) {                     // Human game
          draw_point(dot[7], 0, 10, 0);    // Green
          image(); 
      }         
      if (game == 1) {                     // Computer game
          draw_point(dot[7], 0, 10, 10);   // Cyan
          image(); 
      }


      // Check for free row in column
      byte topRow = 0;
      byte row_select = 5;
      byte find_row = 0;
       for (int x=0; x<6; x++) {
           if (board[x][counter] == 0) {   
           row_select = x;
           find_row = 1;                     // Free row
           }
        if (find_row == 0) { topRow = 1; }   // No free row
       }  


      byte playerSwitch = 0;
      if   ( (playerTurn == 1) && (game == 0) && (game_win == 0) && (topRow == 0) )     { 
             board[row_select][counter]      = 1;
             playerSwitch = 1; 
             draw_point(dot[counter], playerTWO_LED_R, playerTWO_LED_G, playerTWO_LED_B); 
             image();             
             check_board(playerTurn);   
             playerTurn = 2;                        
      }

      if   ( (playerTurn == 1) && (game == 1) && (game_win == 0) && (topRow == 0) )      { 
             board[row_select][counter] = 1;             
             playerSwitch = 0; 
             draw_line(topLine, 0, 0, 0);
             image();
             draw_point(dot[counter], playerTWO_LED_R, playerTWO_LED_G, playerTWO_LED_B); 
             image();  
             check_board(playerTurn); 
             playerTurn = 2;                                       
            } 

      if ( (playerTurn == 2) && (playerSwitch == 0) && (game == 0) && (game_win == 0) && (topRow == 0) )  { 
            board[row_select][counter]      = 2;           
            draw_point(dot[counter], playerONE_LED_R, playerONE_LED_G, playerONE_LED_B); 
            image();  
            check_board(playerTurn);  
            playerTurn = 1;                                
            }

      if ( (playerTurn == 2) && (playerSwitch == 0) && (game == 1) && (game_win == 0) )  {
            byte column;

            randomSeed(analogRead(0));           // Randomize using noise from analog pin 0  
            if (gameSelect == 0) {               // Random
               column = pickRandomMove();        // Pick a random column              
            }
            if (gameSelect == 1) {               // AI (based on simple scoring)
               column = pickBestMove();          // Pick best column
            }

            byte openRow = getNextOpenRow(column);
            Serial.print("P2 Column (Best Move): ");
            Serial.println(column);         
            Serial.print("P2 Row: ");
            Serial.println(openRow);    


            board[openRow][column]      = 2;          
            draw_line(topLine, 0, 0, 0);
            image();
            draw_point(dot[counter], playerONE_LED_R, playerONE_LED_G, playerONE_LED_B); 
            image(); 
            check_board(playerTurn);   
            playerTurn = 1;                              
            }

// Check if there is a Win
      if (game_win == 1) {
            draw_point(dot[counter], 0, 0, 0); 
            image();  
            byte game_switch = 0;
            if (game == 1) { game == 0; game_switch = 1; };
            if ( (game == 0) && (game_switch == 0) )   { game = 1; };           
      }
      if (game_win == 2) {
            draw_point(dot[counter], 0, 0, 0); 
            image(); 
            byte game_switch = 0;
            if (game == 1) { game == 0; game_switch = 1; };
            if ( (game == 0) && (game_switch == 0) )   { game = 1; };                     
      }

  draw_board();

  //Serial.println("0123456");
   for (int x=0; x<6; x++) {
      for (int y=0; y<7; y++) {
          //Serial.print(board[x][y]);
      }
   //Serial.println("");       
   }
   //Serial.println("");

	}

  // If there is a game Win, restart Arduino
      if (game_win == 1) {  
         delay(5000); 
         resetFunc(); 
      }
      if (game_win == 2) {             
         delay(5000);
         resetFunc(); 
      }      


		// Remember last button press event
		lastButtonPress = millis();
	}

	// Put in a slight delay to help debounce the reading
	delay(1);
}


void draw_board() {
   for (int x=0; x<6; x++) {
      for (int y=0; y<7; y++) {
           byte y_reverse = 7-y;                                                            // Left to right 
           byte pixel[][2]    = { {x+2, y_reverse} };                                       // Create pixel       
        if (board[x][y] == 1) {
           draw_point(pixel[0], playerONE_LED_R, playerONE_LED_G, playerONE_LED_B);         // Draw pixel Player 1
           image();
        }
        if (board[x][y] == 2) {
           draw_point(pixel[0], playerTWO_LED_R, playerTWO_LED_G, playerTWO_LED_B);         // Draw pixel Player 2
           image();
        }
        if (board[x][y] == 3) {
           draw_point(pixel[0], playerONE_LED_R+40, playerONE_LED_G, playerONE_LED_B);      // Draw Connect 4 Player 1
           image();
        }
         if (board[x][y] == 4) {
           draw_point(pixel[0], playerTWO_LED_R+40, playerTWO_LED_G+40, playerTWO_LED_B);   // Draw Connect 4 Player 2
           image();
        }       
      }
   } 
}



int getBoardScore() {
byte rowScore = 0;
byte score = 0;

// Horizontal
for (int i = 0; i < 6; i++) {
String myRow = "";
   for (int j = 0; j < 7; j++) {
     myRow.concat(board_copy[i][j]);
   }
   rowScore = getScore(myRow);

    if (rowScore > score) { 
       score = rowScore; 
    }
}
//Serial.print("Score: ");
//Serial.println(score);      

// Vertical
rowScore = 0;
for (int i = 0; i < 7; i++) {
String myColumn = "";
for (int j = 0; j < 6; j++) {
   myColumn.concat(board_copy[j][i]);
}
rowScore = getScore(myColumn);
if (rowScore > score) { score = rowScore; }
}


// Diagonal 1
rowScore = 0;
String myDiag = "";
myDiag.concat(board_copy[0][0]);
myDiag.concat(board_copy[1][1]);
myDiag.concat(board_copy[2][2]);
myDiag.concat(board_copy[3][3]);
myDiag.concat(board_copy[4][4]);
myDiag.concat(board_copy[5][5]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[0][1]);
myDiag.concat(board_copy[1][2]);
myDiag.concat(board_copy[2][3]);
myDiag.concat(board_copy[3][4]);
myDiag.concat(board_copy[4][5]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[0][2]);
myDiag.concat(board_copy[1][3]);
myDiag.concat(board_copy[2][4]);
myDiag.concat(board_copy[3][5]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }

// Diagonal 2
myDiag = "";
myDiag.concat(board_copy[1][0]);
myDiag.concat(board_copy[2][1]);
myDiag.concat(board_copy[3][2]);
myDiag.concat(board_copy[4][3]);
myDiag.concat(board_copy[5][4]);
myDiag.concat(board_copy[6][5]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[2][0]);
myDiag.concat(board_copy[3][1]);
myDiag.concat(board_copy[4][2]);
myDiag.concat(board_copy[5][3]);
myDiag.concat(board_copy[6][4]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[3][0]);
myDiag.concat(board_copy[4][1]);
myDiag.concat(board_copy[5][2]);
myDiag.concat(board_copy[6][3]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }

// Diagonal 3
myDiag = "";
myDiag.concat(board_copy[1][5]);
myDiag.concat(board_copy[2][4]);
myDiag.concat(board_copy[3][3]);
myDiag.concat(board_copy[4][2]);
myDiag.concat(board_copy[5][1]);
myDiag.concat(board_copy[6][0]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[2][5]);
myDiag.concat(board_copy[3][4]);
myDiag.concat(board_copy[4][3]);
myDiag.concat(board_copy[5][2]);
myDiag.concat(board_copy[6][1]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[3][5]);
myDiag.concat(board_copy[4][4]);
myDiag.concat(board_copy[5][3]);
myDiag.concat(board_copy[6][2]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }

// Diagonal 4
myDiag = "";
myDiag.concat(board_copy[0][5]);
myDiag.concat(board_copy[1][4]);
myDiag.concat(board_copy[2][3]);
myDiag.concat(board_copy[3][2]);
myDiag.concat(board_copy[4][1]);
myDiag.concat(board_copy[5][0]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[0][4]);
myDiag.concat(board_copy[1][3]);
myDiag.concat(board_copy[2][2]);
myDiag.concat(board_copy[3][1]);
myDiag.concat(board_copy[4][0]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }
myDiag = "";
myDiag.concat(board_copy[0][3]);
myDiag.concat(board_copy[1][2]);
myDiag.concat(board_copy[2][1]);
myDiag.concat(board_copy[3][0]);
rowScore = getScore(myDiag);
if (rowScore > score) { score = rowScore; }


return score;
}

int getScore(String myRow) {
  //Serial.println(myRow);
byte rowLength = myRow.length();

int myScore     = 0;
int myHighScore = 0;


for (int k = 0; k < (rowLength-3); k++) {
  String myWindow = "";
  myWindow = myRow.substring(k, k+4);

  //Serial.println(myWindow);
    if (myWindow.equals("2222")) {
      myScore = 100;
    }
    if (myWindow.equals("1111")) {
        myScore = 95;
    }    
    if (myWindow.equals("0222")) {
      myScore = 20;
    }
    if (myWindow.equals("2022")) {
      myScore = 21;
    }
    if (myWindow.equals("2202")) {
      myScore = 22;
    }
    if (myWindow.equals("2220")) {
      myScore = 23;
    }
    if (myWindow.equals("2202")) {
      myScore = 24;
    } 
    if (myWindow.equals("0022")) {
      myScore = 7;
    }
    if (myWindow.equals("2002")) {
      myScore = 2;
    }
    if (myWindow.equals("2200")) {
      myScore = 8;
    }
    if (myWindow.equals("2020")) {
      myScore = 4;
    }  
    if (myWindow.equals("0202")) {
      myScore = 1;
    }  


    if (myWindow.equals("0111")) {
      myScore = 30;
    }
    if (myWindow.equals("1110")) {
      myScore = 31;
    }

    if (myScore > myHighScore) {myHighScore = myScore;}
}
return myHighScore;
}



String getValidLocations() {
  String validLocations = "";
  for ( int i = 0; i < 7; i++) {
    byte validColumn = isValidLocation(i);
    if (validColumn == 1) {
      validLocations.concat(i);
    }
  }
  return validLocations;
}

byte isValidLocation(int myColumn) {
  byte validColumn = 0;
  if (board[0][myColumn] == 0) {
     validColumn = 1;
  }
return validColumn;
}


byte getNextOpenRow(int myColumn) {
  byte openRow = 5;
   for (int i = 5; i > 0; i--) {
     if (board[i][myColumn] != 0) {
        openRow = i-1;
     }
   }
return openRow;
}


byte pickRandomMove() {
   String validLocations = getValidLocations();
   char column_array[validLocations.length()+1];
   validLocations.toCharArray(column_array, validLocations.length()+1);
   byte bestMove = column_array[random(validLocations.length())]-48;
return bestMove;   
}

byte pickBestMove() {

byte bestMove   = 3;
int boardScore = 0;
int highScore  = -1;
String validLocations = getValidLocations();
char column_array[validLocations.length()+1];
validLocations.toCharArray(column_array, validLocations.length()+1);
//Serial.print("Valid Locations: ");
//Serial.println(validLocations);

// If center column has free slots, select center piece as best move
byte checkValidLocation = isValidLocation(3);
    if (checkValidLocation == 1) {
       byte center_count = 0;
       for (int j = 0; j < 6; j++) {
          if (board[j][3] == 2) { center_count++; }
       }
       boardScore = center_count * 6;

       if (boardScore == 0) { boardScore = 50; }

       if (boardScore > highScore) { 
          highScore = boardScore;
          bestMove = 3;
       }      
    }

for (int i = 0; i< validLocations.length(); i++) {
    int column = column_array[i]-48;
    byte row   = getNextOpenRow(column);
    //Serial.print(" ");
    //Serial.print(column_array[i]);
    //Serial.print("-");
    //Serial.print(row);
    //Serial.println(" ");
    

    // Copy board to temporary board
    for (int k = 0; k < 6; k++) {
      for (int j = 0; j < 7; j++) {
        board_copy[k][j] = board[k][j];
      }
    }

    board_copy[row][column] = 2;
    boardScore = getBoardScore();
    if (boardScore > highScore) { 
        highScore = boardScore;
        bestMove = column;
    }
    board_copy[row][column] = 1;
    boardScore = getBoardScore();
    if (boardScore > highScore) {
       highScore = boardScore;
       bestMove = column;
    }    
    Serial.println();
    Serial.print(column);    
    Serial.print(" - Board: ");
    Serial.print(boardScore);  
    Serial.print(" - High: ");       
    Serial.print(highScore);
    Serial.print(" - Best : ");       
    Serial.println(bestMove);   

}




    // Write board
    for ( int k = 0; k < 6; k++ ) {
      for (int j = 0; j < 7; j++) {
        //Serial.print(board_copy[k][j]);
        //Serial.print(board[k][j]);
      }
    //Serial.println();
    }

//Serial.println("");
return bestMove;
}





void check_board(byte playerTurn) {
byte found_4_in_a_row = 0;
   for (int x=0; x<6; x++) {
      for (int y=0; y<7; y++) {
      
      // Horizontal
      if ( (y<4) && (board[x][y] == playerTurn) && (board[x][y+1] == playerTurn) && (board[x][y+2] == playerTurn) && (board[x][y+3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[x][y]           = playerTurn + 2;
         board[x][y+1]         = playerTurn + 2;
         board[x][y+2]         = playerTurn + 2;
         board[x][y+3]         = playerTurn + 2;                                      
      }

      // Vertical
      if ( (board[x][y] == playerTurn) && (board[x+1][y] == playerTurn) && (board[x+2][y] == playerTurn) && (board[x+3][y] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[x][y]           = playerTurn + 2;
         board[x+1][y]         = playerTurn + 2;
         board[x+2][y]         = playerTurn + 2;
         board[x+3][y]         = playerTurn + 2;                   
      }

      // Diagonal (1)
      if ( (board[0][0] == playerTurn) && (board[1][1] == playerTurn) && (board[2][2] == playerTurn) && (board[3][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[0][0]           = playerTurn + 2;
         board[1][1]           = playerTurn + 2;
         board[2][2]           = playerTurn + 2;
         board[3][3]           = playerTurn + 2;     
         Serial.print("Diagonal 1-1 - Player: ");
         Serial.println(playerTurn);            
      }
      if ( (board[1][1] == playerTurn) && (board[2][2] == playerTurn) && (board[3][3] == playerTurn) && (board[4][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[1][1]           = playerTurn + 2;
         board[2][2]           = playerTurn + 2;
         board[3][3]           = playerTurn + 2;
         board[4][4]           = playerTurn + 2; 
         Serial.print("Diagonal 1-2 - Player: ");
         Serial.println(playerTurn);                         
      }
      if ( (board[2][2] == playerTurn) && (board[3][3] == playerTurn) && (board[4][4] == playerTurn) && (board[5][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[2][2]           = playerTurn + 2;
         board[3][3]           = playerTurn + 2;
         board[4][4]           = playerTurn + 2;
         board[5][5]           = playerTurn + 2;   
         Serial.print("Diagonal 1-3 - Player: ");
         Serial.println(playerTurn);                     
      }
      if ( (board[1][0] == playerTurn) && (board[2][1] == playerTurn) && (board[3][2] == playerTurn) && (board[4][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[1][0]           = playerTurn + 2;
         board[2][1]           = playerTurn + 2;
         board[3][2]           = playerTurn + 2;
         board[4][3]           = playerTurn + 2;
         Serial.print("Diagonal 1-4 - Player: ");
         Serial.println(playerTurn);                          
      }
      if ( (board[2][1] == playerTurn) && (board[3][2] == playerTurn) && (board[4][3] == playerTurn) && (board[5][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[2][1]           = playerTurn + 2;
         board[3][2]           = playerTurn + 2;
         board[4][3]           = playerTurn + 2;
         board[5][4]           = playerTurn + 2;   
         Serial.print("Diagonal 1-5 - Player: ");
         Serial.println(playerTurn);                   
      }
      if ( (board[2][0] == playerTurn) && (board[3][1] == playerTurn) && (board[4][2] == playerTurn) && (board[5][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[2][0]           = playerTurn + 2;
         board[3][1]           = playerTurn + 2;
         board[4][2]           = playerTurn + 2;
         board[5][3]           = playerTurn + 2;   
         Serial.print("Diagonal 1-6 - Player: ");
         Serial.println(playerTurn);                      
      }



      // Diagonal (2)
      if ( (board[0][1] == playerTurn) && (board[1][2] == playerTurn) && (board[2][3] == playerTurn) && (board[3][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[0][1]           = playerTurn + 2;
         board[1][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;
         board[3][4]           = playerTurn + 2;
         Serial.print("Diagonal 2-1 - Player: ");
         Serial.println(playerTurn);                          
      }
      if ( (board[1][2] == playerTurn) && (board[2][3] == playerTurn) && (board[3][4] == playerTurn) && (board[4][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[1][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;
         board[3][4]           = playerTurn + 2;
         board[4][5]           = playerTurn + 2; 
         Serial.print("Diagonal 2-2 - Player: ");
         Serial.println(playerTurn);                         
      }
      if ( (board[2][3] == playerTurn) && (board[3][4] == playerTurn) && (board[4][5] == playerTurn) && (board[5][6] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[2][3]           = playerTurn + 2;
         board[3][4]           = playerTurn + 2;
         board[4][5]           = playerTurn + 2;
         board[5][6]           = playerTurn + 2;  
         Serial.print("Diagonal 2-3 - Player: ");
         Serial.println(playerTurn);                        
      }
      if ( (board[0][2] == playerTurn) && (board[1][3] == playerTurn) && (board[2][4] == playerTurn) && (board[3][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[0][2]           = playerTurn + 2;
         board[1][3]           = playerTurn + 2;
         board[2][4]           = playerTurn + 2;
         board[3][5]           = playerTurn + 2; 
         Serial.print("Diagonal 2-4 - Player: ");
         Serial.println(playerTurn);                            
      }
      if ( (board[1][3] == playerTurn) && (board[2][4] == playerTurn) && (board[3][5] == playerTurn) && (board[4][6] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[1][3]           = playerTurn + 2;
         board[2][4]           = playerTurn + 2;
         board[3][5]           = playerTurn + 2;
         board[4][6]           = playerTurn + 2;
         Serial.print("Diagonal 2-5 - Player: ");
         Serial.println(playerTurn);                            
      }
      if ( (board[0][3] == playerTurn) && (board[1][4] == playerTurn) && (board[2][5] == playerTurn) && (board[3][6] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[0][3]           = playerTurn + 2;
         board[1][4]           = playerTurn + 2;
         board[2][5]           = playerTurn + 2;
         board[3][6]           = playerTurn + 2;  
         Serial.print("Diagonal 2-6 - Player: ");
         Serial.println(playerTurn);                           
      }


      // Diagonal (3)
      if ( (board[5][0] == playerTurn) && (board[4][1] == playerTurn) && (board[3][2] == playerTurn) && (board[2][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[5][0]           = playerTurn + 2;
         board[4][1]           = playerTurn + 2;
         board[3][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;  
         Serial.print("Diagonal 3-1 - Player: ");
         Serial.println(playerTurn);                         
      }
      if ( (board[4][1] == playerTurn) && (board[3][2] == playerTurn) && (board[2][3] == playerTurn) && (board[1][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[4][1]           = playerTurn + 2;
         board[3][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;
         board[1][4]           = playerTurn + 2;  
         Serial.print("Diagonal 3-2 - Player: ");
         Serial.println(playerTurn);                         
      }
      if ( (board[3][2] == playerTurn) && (board[2][3] == playerTurn) && (board[1][4] == playerTurn) && (board[0][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[3][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;
         board[1][4]           = playerTurn + 2;
         board[0][5]           = playerTurn + 2;   
         Serial.print("Diagonal 3-3 - Player: ");
         Serial.println(playerTurn);                       
      }
      if ( (board[4][0] == playerTurn) && (board[3][1] == playerTurn) && (board[2][2] == playerTurn) && (board[1][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[4][0]           = playerTurn + 2;
         board[3][1]           = playerTurn + 2;
         board[2][2]           = playerTurn + 2;
         board[1][3]           = playerTurn + 2;  
         Serial.print("Diagonal 3-4 - Player: ");
         Serial.println(playerTurn);                     
      }
      if ( (board[3][1] == playerTurn) && (board[2][2] == playerTurn) && (board[1][3] == playerTurn) && (board[0][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[3][1]           = playerTurn + 2;
         board[2][2]           = playerTurn + 2;
         board[1][3]           = playerTurn + 2;
         board[0][4]           = playerTurn + 2;  
         Serial.print("Diagonal 3-5 - Player: ");
         Serial.println(playerTurn);                     
      }
      if ( (board[3][0] == playerTurn) && (board[2][1] == playerTurn) && (board[1][2] == playerTurn) && (board[0][3] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[3][0]           = playerTurn + 2;
         board[2][1]           = playerTurn + 2;
         board[1][2]           = playerTurn + 2;
         board[0][3]           = playerTurn + 2; 
         Serial.print("Diagonal 3-6 - Player: ");
         Serial.println(playerTurn);                         
      }



      // Diagonal (4)
      if ( (board[5][1] == playerTurn) && (board[4][2] == playerTurn) && (board[3][3] == playerTurn) && (board[2][4] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[5][1]           = playerTurn + 2;
         board[4][2]           = playerTurn + 2;
         board[3][3]           = playerTurn + 2;
         board[2][4]           = playerTurn + 2; 
         Serial.print("Diagonal 4-1 - Player: ");
         Serial.println(playerTurn);                           
      }
      if ( (board[4][2] == playerTurn) && (board[3][3] == playerTurn) && (board[2][4] == playerTurn) && (board[1][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[4][2]           = playerTurn + 2;
         board[3][3]           = playerTurn + 2;
         board[2][4]           = playerTurn + 2;
         board[1][5]           = playerTurn + 2;
         Serial.print("Diagonal 4-2 - Player: ");
         Serial.println(playerTurn);                          
      }
      if ( (board[3][2] == playerTurn) && (board[2][3] == playerTurn) && (board[1][4] == playerTurn) && (board[0][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[3][2]           = playerTurn + 2;
         board[2][3]           = playerTurn + 2;
         board[1][4]           = playerTurn + 2;
         board[0][5]           = playerTurn + 2;
         Serial.print("Diagonal 4-3 - Player: ");
         Serial.println(playerTurn);                         
      }
      if ( (board[5][2] == playerTurn) && (board[4][3] == playerTurn) && (board[3][4] == playerTurn) && (board[2][5] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[5][2]           = playerTurn + 2;
         board[4][3]           = playerTurn + 2;
         board[3][4]           = playerTurn + 2;
         board[2][5]           = playerTurn + 2;
         Serial.print("Diagonal 4-4 - Player: ");
         Serial.println(playerTurn);                      
      }
      if ( (board[4][3] == playerTurn) && (board[3][4] == playerTurn) && (board[2][5] == playerTurn) && (board[1][6] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[4][3]           = playerTurn + 2;
         board[3][4]           = playerTurn + 2;
         board[2][5]           = playerTurn + 2;
         board[1][6]           = playerTurn + 2;
         Serial.print("Diagonal 4-5 - Player: ");
         Serial.println(playerTurn);                      
      }
      if ( (board[5][3] == playerTurn) && (board[4][4] == playerTurn) && (board[3][5] == playerTurn) && (board[2][6] == playerTurn) && (found_4_in_a_row == 0) ) {
         game_win              = playerTurn;
         found_4_in_a_row      = 1;
         board[5][3]           = playerTurn + 2;
         board[4][4]           = playerTurn + 2;
         board[3][5]           = playerTurn + 2;
         board[2][6]           = playerTurn + 2; 
         Serial.print("Diagonal 4-6 - Player: ");
         Serial.println(playerTurn);                        
      }





   }
 }

}

