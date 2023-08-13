//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define CR  '\r'
#define LF  '\n'
#define VT  '\v'
#define BS  '\b'
#define FF  '\f'
#define ESC '\e'
#define NUL '\0'


#define lcd_nrows  2
#define lcd_ncols 16


/* LCD buffers and cursor position
*/
char *lcdRows[lcd_nrows];
char lcdBuf[lcd_nrows*lcd_ncols];
int row, col;
int ch;

#define ctrlseq_nfields  2 

/* Control sequence processing
*/
int ctrlseq_p[2];
int ctrlseq_pos;

void LcdPutc(int ch);
void LcdScrollUp(void);
void LcdScrollDown(void);
void LcdBackspace(void);
void LcdClearRow(int);
void LcdHomeAndClear(void);
int LcdCtrlSeq(int ch);
void LcdCtrlSetCursor(void);
void LcdCtrlMoveCursorUp(void);
void LcdCtrlMoveCursorDown(void);
void LcdCtrlMoveCursorRight(void);
void LcdCtrlMoveCursorLeft(void);
void LcdCtrlClearLines(void);
void LcdCtrlClearCharacters(void);



LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
byte cmd; //will hold our sent command

int serial_read() {
  int result = -1;
  while (result == -1) {
    if (Serial.available() > 0) {
      result = Serial.read();
    }
  }
  return result;
}



void setup()
{
   lcd.init();
  // set up serial
  Serial.begin(57600);

  lcd.init();
  lcd.backlight();
  // lcd.setCursor(0, 0);

  //lcd.display();
  //lcd.clear();
  char welcome[32];
  char *ptr = welcome + 5;
  sprintf(welcome, "%dx%d Ready", 16, 2);
  //lcd.write(welcome);
  //lcd.setCursor(0,0);
  // lcd.print(ptr);
  //lcd.home();
  //lcd.print("hello, world!");
  lcd.print(ptr);
  //delay(200);
}


void loop()
{
     cmd = serial_read();

if ( !LcdCtrlSeq(cmd) )
   {
//      LcdPutc(ch);
//    }
        
    switch (cmd) {
      case ESC: //esc
        //lcd.print("setup");
        //lcd.command(serial_read());
        ch=serial_read();
        if (ch==0){
          lcd.write(serial_read());
          }
        if (ch==FF){
          lcd.clear();
          lcd.home();
          }
          if ( ch == CR )
          { col=0;
            lcd.setCursor(col, row);
            }
          if ( ch == 'H' || ch == 'f' )
          {
            LcdCtrlSetCursor();
            ctrlseq_pos = -1;
            }
            
          
        //lcd.write(cmd);
        break;
      case 0xFD:
        // backlight control
        //set_backlight(serial_read());
        break;
      case 0x0D:
        //lcd.write(cmd);
        //lcd.setCursor(0,1);
        // backlight control
        //set_backlight(serial_read());
        //lcd.home();
        break;
      case 0x0A:
        //lcd.write(cmd);
        //lcd.setCursor(0,1);
        // backlight control
        //set_backlight(serial_read());
        break;
      default:
        // By default we write to the LCD
        //lcd.printstr(cmd);
        //lcd.setCursor(0,1);
        lcd.write(cmd);
        break;
}}
}

void LcdCtrlSetCursor(void)
{
  row = ctrlseq_p[0] - 1;
  row = ( row < 0 ) ? 0 : row;            // Default to first row.
  row = ( row >= lcd_nrows ) ? lcd_nrows-1 : row;   // Force to last row.

  col = ctrlseq_p[1] - 1;
  col = ( col < 0 ) ? 0 : col;            // Default to first column.
  col = ( col > lcd_ncols ) ? lcd_ncols : col;    // Force to "just off screen".

  lcd.setCursor(col, row);
}

void LcdHomeAndClear(void)
{
  int i, j;

  lcd.clear();
  for ( i=0; i<lcd_nrows; i++ )
  {
    for ( j=0; j<lcd_ncols; j++ )
    {
      lcdRows[i][j] = NUL;
    }
  }

  row = 0;
  col = 0;
  lcd.setCursor(col, row);
}

int LcdCtrlSeq(int ch)
{
  int eaten = 0;
  if ( ctrlseq_pos < 0 )
  {
    /* Not in a control sequence at the moment
    */
    if ( ch < ' ' )
    {
      /* A control character ...
      */
      eaten = 1;        // We ate the character

      if ( ch == ESC )
      {
        ctrlseq_pos = 0;  // Start of a control sequence
      }
      else
      if ( ch == BS )
      {
        LcdBackspace();
      }
      else
      if ( ch == CR )
      {
        col = 0;
        lcd.setCursor(col, row);
      }
      else
      if ( ch == LF )
      {
        if ( row >= 3 )
        {
          row = 3;    // Defensive
          LcdScrollUp();
        }
        else
        {
          row++;
        }
        lcd.setCursor(col, row);
      }
      else
      if ( ch == VT )
      {
        if ( row <= 0 )
        {
          row = 0;    // Defensive
          LcdScrollDown();
        }
        else
        {
          row--;
        }
        lcd.setCursor(col, row);
      }
      else
      if ( ch == FF )
      {
        LcdHomeAndClear();
      }
      else
      {
        /* Other control characters are passed through
        */
        eaten = 0;
      }
    }
  }
  else
  if ( ctrlseq_pos == 0 )
  {
    /* First character.
     * If it's '[' it could be one of the recognised  control sequences
    */
    if ( ch == '[' )
    {
      ctrlseq_pos = 1;
      ctrlseq_p[0] = -1;
      ctrlseq_p[1] = -1;
      eaten = 1;      // We ate the character
    }
    else
    {
      /* Just use the escaped character as a character. (eatern = 0)
      */
      ctrlseq_pos = -1;
    }
  }
  else
  if ( ctrlseq_pos <= ctrlseq_nfields )
  {
    if ( ch >= '0' && ch <= '9' )
    {
      /* In a numeric field.
      */
      int d = ch - '0';
      if ( ctrlseq_p[ctrlseq_pos-1] < 0 )
      {
        ctrlseq_p[ctrlseq_pos-1] = d;
      }
      else
      {
        ctrlseq_p[ctrlseq_pos-1] = ctrlseq_p[ctrlseq_pos-1] * 10 + d;
      }
    }
    else
    if ( ch == ';' )
    {
      /* Move to next field.
      */
      ctrlseq_pos++;
    }
    else
    if ( ch == 'H' || ch == 'f' )
    {
      LcdCtrlSetCursor();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'A' )
    {
      LcdCtrlMoveCursorUp();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'B' )
    {
      LcdCtrlMoveCursorDown();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'C' )
    {
      LcdCtrlMoveCursorRight();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'D' )
    {
      LcdCtrlMoveCursorLeft();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'J' )
    {
      LcdCtrlClearLines();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    if ( ch == 'K' )
    {
      LcdCtrlClearCharacters();
      ctrlseq_pos = -1;   // ... and that's it
    }
    else
    {
      /* Unknown control sequence - just ignore it.
      */
      ctrlseq_pos = -1;
    }
    eaten = 1;      // We ate the character
  }
  else
  {
    /* Control sequence is too long. Drop it and treat character as raw inputs (eaten = 0).
    */
    ctrlseq_pos = -1;
  }
  return eaten;
}


//void LcdCtrlSetCursor(void)
//{
//  row = ctrlseq_p[0] - 1;
//  row = ( row < 0 ) ? 0 : row;            // Default to first row.
//  row = ( row >= lcd_nrows ) ? lcd_nrows-1 : row;   // Force to last row.
//
//  col = ctrlseq_p[1] - 1;
//  col = ( col < 0 ) ? 0 : col;            // Default to first column.
//  col = ( col > lcd_ncols ) ? lcd_ncols : col;    // Force to "just off screen".
//
//  lcd.setCursor(col, row);
//}

/* LcdCtrlMoveCursorUp() - move the cursor r rows up.
*/
void LcdCtrlMoveCursorUp(void)
{
  int r = ctrlseq_p[0];
  if ( r < 0 )  r = 1;

  row = row - r;
  if ( row < 0 )  row = 0;

  lcd.setCursor(col, row);
}

/* LcdCtrlMoveCursorDown() - move the cursor r rows down.
*/
void LcdCtrlMoveCursorDown(void)
{
  int r = ctrlseq_p[0];
  if ( r < 0 )        r = 1;

  row = row + r;
  if ( row >= lcd_nrows )   row = lcd_nrows-1;

  lcd.setCursor(col, row);
}

/* LcdCtrlMoveCursorRight() - move the cursor c colums right.
*/
void LcdCtrlMoveCursorRight(void)
{
  int c = ctrlseq_p[0];
  if ( c < 0 )        c = 1;

  col = col + c;
  if ( col > lcd_ncols )    col = lcd_ncols;

  lcd.setCursor(col, row);
}

/* LcdCtrlMoveCursorLeft() - move the cursor c colums left.
*/
void LcdCtrlMoveCursorLeft(void)
{
  int c = ctrlseq_p[0];
  if ( c < 0 )    c = 1;

  col = col - c;
  if ( col < 0 )    col = 0;

  lcd.setCursor(col, row);
}

/* LcdCtrlClearLines() - clear lines of the display
 *  ESC[0J   - clear lines above cursor (not including cursor line)
 *  ESC[1J   - clear lines below cursor (including cursor line)
 *  ESC[2J   - clear display
*/
void LcdCtrlClearLines(void)
{
  int i, start = 0, end = lcd_nrows;
  int saverow, savecol;

  saverow = row;
  savecol = col;

  if ( ctrlseq_p[0] <= 0 )
  {
    end = row;
  }
  else
  if ( ctrlseq_p[0] == 1 )
  {
    start = row;
  }
  else
  if ( ctrlseq_p[0] == 2 )
  {
    /* No change
    */
  }
  else
  {
    return;   // Bomb out - invalid code.
  }

  for ( i = start; i < end; i++ )
  {
    LcdClearRow(i);
  }

  /* Put the cursor back where it was.
  */
  row = saverow;
  col = savecol;
  lcd.setCursor(col, row);
}

/* LcdCtrlClearCharacters() - clear characters from the current line.
 *  ESC[0K   - clear to right of cursor (including cursor column)
 *  ESC[1K   - clear to left of cursor (not including cursor column. Xterm might be different)
 *  ESC[2K   - clear line
*/
void LcdCtrlClearCharacters(void)
{
  int i, count;
  int saverow, savecol;

  saverow = row;
  savecol = col;

  if ( ctrlseq_p[0] <= 0 )
  {
    /* Clear to EOL. Cursor already correct.
    */
    count = lcd_ncols - col;
  }
  else
  if ( ctrlseq_p[0] == 1 )
  {
    /* Clear from start of line.
    */
    count = col;
    col = 0;
    lcd.setCursor(col, row);
  }
  else
  if ( ctrlseq_p[0] == 2 )
  {
    /* Clear entire line.
    */
    count = lcd_ncols;
    col = 0;
    lcd.setCursor(col, row);
  }
  else
  {
    return;   // Bomb out - invalid code.
  }

  for ( i = 0; i < count; i++ )
  {
    LcdPutc(' ');
  }

  /* Put the cursor back where it was.
  */
  row = saverow;
  col = savecol;
  lcd.setCursor(col, row);
}


/* LcdBackspace() - set the cursor one column to the left.
*/
void LcdBackspace(void)
{
  col = col - 1;
  if ( col < 0 )        col = 0;        // Default to first column.

  lcd.setCursor(col, row);
}



/* LcdScrollUp() - scroll the display up, blank line at bottom, cursor unchanged
*/
void LcdScrollUp(void)
{
  lcd.clear();        // I hate this!
  char *tmp = lcdRows[0];
  int i, j;

  for ( i = 0; i < (lcd_nrows-1); i++ )
  {
    lcdRows[i] = lcdRows[i+1];
    lcd.setCursor(0, i);
    for ( j=0; j<lcd_ncols && lcdRows[i][j] != NUL; j++ )
    {
      lcd.write(lcdRows[i][j]);
    }
  }

  for ( j=0; j<lcd_ncols; j++ )
  {
    tmp[j] = NUL;
  }

  lcdRows[lcd_nrows-1] = tmp;
}

/* LcdScrollDown() - scroll the display down, blank line at top, cursor unchanged
*/
void LcdScrollDown(void)
{
  lcd.clear();        // I hate this!
  char *tmp = lcdRows[lcd_nrows-1];
  int i, j;

  for ( i = (lcd_nrows-1); i > 0; i-- )
  {
    lcdRows[i] = lcdRows[i-1];
    lcd.setCursor(0, i);
    for ( j=0; j<lcd_ncols && lcdRows[i][j] != NUL; j++ )
    {
      lcd.write(lcdRows[i][j]);
    }
  }

  for ( j=0; j<lcd_ncols; j++ )
  {
    tmp[j] = NUL;
  }

  lcdRows[0] = tmp;
}



/* LcdClearRow() - clear a row
 *
 * The cursor position is save and restored by the caller.
*/
void LcdClearRow(int r)
{
  int i;

  col = 0;
  row = r;
  lcd.setCursor(col, row);

  for (i = 0; i < lcd_ncols; i++ )
  {
    LcdPutc(' ');
  }
}

/* LcdPutc() - write a single character to buffers and to LCD module
*/
void LcdPutc(int ch)
{
  if ( col < lcd_ncols )
  {
#if 0 /*  Apparently not working on lcd module. Need to set F bit? */
    /* Use characters with proper descenders.
    */
    if ( ch == 'g' || ch == 'j' || ch == 'p' || ch == 'q' || ch == 'y' )
    {
      ch += 128;
    }
#endif
    lcdRows[row][col] = ch;
    lcd.write(ch);
    col++;
  }
  else
  {
    /* Ignore everything to the right of column lcd_ncols
    */
  }
}
