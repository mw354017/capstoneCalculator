/*******************************************************************
 *                  Capstone Calculator
 * An arduino based calculator created as a part of our Capstone.
 * 
 * By: Matthew Wagner, Noah Blain, Donovan Booker, Brandon Garner
 *******************************************************************/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

double nothing = 3.4028235*pow(10,38);                                                                              // This value has been set to indicate that nothing is stored in the operand

LiquidCrystal_I2C lcd(0x27,20,4);                                                                                   // Set the LCD address to 0x27 for a 16 chars and 2 line display

struct operand {
  double realComponent = nothing;
  double imaginaryComponent = nothing;
  int  parenthesesDepth = 0;
};

struct operation {
  char op = NULL;
  int parenthesesDepth = 0;
};

char getKey();                                                                                                      // Returns the input from the keypad
operand parseInput(char totalInput[32]);                                                                            // Makes the input numbers and ops, and calls orderOfOps
operand orderOfOps(operand numbers[32], operation ops[31], int sizeNumbers, int sizeOps, int parenthesesLevel);     // Calls calculate on numbers and ops in the correct order
operand calculate(operand left, char op, operand right);                                                            // Returns an answer for an operation preformed on 2 complex numbers
void errorHandler(char code[]);                                                                                     // Prints the Error code and performs a reset

bool reset=1; // If true, then we must reset the calculator, clearing all values

void setup() {
  Serial.begin(9600);
  
  pinMode(2, OUTPUT); // Turn on Power LED
  digitalWrite(2,HIGH);
  
  pinMode(4, INPUT_PULLUP); // Setup shift button
  
  pinMode(5, OUTPUT); // Setup keypad rows
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  
  pinMode(9, INPUT_PULLUP); // Setup keypad collumns
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

  lcd.init(); // Initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  
  lcd.print("Downloading More"); // Just for lolz
  lcd.setCursor(0,1);
  lcd.print("Deditated Wam");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.clear();

  byte negativeChar[] = {
    B00000,
    B00000,
    B00000,
    B01110,
    B00000,
    B00000,
    B00000,
    B00000,
  };
  lcd.createChar(0, negativeChar);
}

void loop() {
  // Preform a reset
  operand answer;
  char totalInput[32] = {NULL};
  int i= 0 ;
  reset = 0;
  int cursorLocation = 0;
  lcd.clear();
  lcd.blink();
  char input=NULL;
  while(reset == 0) { // Get new input and act accordingly
    input = getKey(); 
    if(input == NULL) {} // Do nothing
    else if(input=='X') { // 'X' clears the screen
      reset = 1;
    } else if(input=='x') { // 'x' is backspace
      if(cursorLocation != 0) { // Don't backspace if there is nothing there
        cursorLocation--;
        if(cursorLocation < 16) { // Move the cursor back, and wrap around the screen
          lcd.setCursor(cursorLocation,0);
        } else {
          lcd.setCursor(cursorLocation-15,1);
        }
        lcd.print(' ');
        if(cursorLocation < 16) {
          lcd.setCursor(cursorLocation,0);
        } else {
          lcd.setCursor(cursorLocation-15,1);
        }
        totalInput[i-1] = NULL; // Remove the relavent input
        i--;
      }
    } else if(input == '<') { // Move cursor backward
      if(cursorLocation != 0) {
        cursorLocation--;
        i--;
        if(cursorLocation < 16) {
          lcd.setCursor(cursorLocation,0);
        } else {
          lcd.setCursor(cursorLocation-15,1);
        }
      }
    } else if(input == '>'){ // Move cursor forward
      if(cursorLocation <= i) {
        cursorLocation++;
        i++;
        if(cursorLocation < 16) {
            lcd.setCursor(cursorLocation,0);
        } else {
          lcd.setCursor(cursorLocation-15,1);
        }
      }
    } else if(input == 'A') { // The previous answer NOT YET IMPLEMENTED
      //itoa
    } else { // The character needs printed
      cursorLocation++;
      if(cursorLocation == 16) {
        lcd.setCursor(0,1); //second row
      }
      if(input == '_') { // '_' is used for subtraction, displayed as '-'
        lcd.print('-');                  
      } else if(input == '-') { // '-' is used for negative numbers, displayed as the custom character
        lcd.write(0);
      } else {
        lcd.print(input);
      }
      if(input == '=') { // Calculate and print the answer
        answer = parseInput(totalInput);
        if(reset == 0) { // Only displays the answer if an error didn't occur
          lcd.clear();
          lcd.print("= ");
          if(answer.realComponent != 0 && answer.realComponent != nothing) {  // Only display the relavent components with proper precision
            if(answer.imaginaryComponent == 0 || answer.imaginaryComponent == nothing){
              lcd.print(answer.realComponent,5);
            } else {         
               lcd.print(answer.realComponent);
            }
            
            if(answer.imaginaryComponent != 0 && answer.imaginaryComponent != nothing) {
              lcd.print(" + ");
            }
          }
          if(answer.imaginaryComponent != 0 && answer.imaginaryComponent != nothing ) {
            lcd.print("j");
            if(answer.realComponent == 0 || answer.realComponent == nothing){
              lcd.print(answer.imaginaryComponent,5);
            } else {         
               lcd.print(answer.imaginaryComponent);
            }
          } else if(answer.imaginaryComponent == 0 && answer.realComponent != 0) {
            lcd.print("0");
          }
          delay(200); // Limit accidental double presses of '='
          while(getKey() == NULL) {} // Wait for an input before resetting
          reset = 1;
        }
      } else { // Just store the character
        totalInput[i]=input;
        i++;
      } 
      delay(200); // Limit accidental double presses
    }
  }
}

operand parseInput(char totalInput[32]) 
{
  char numbers1[32] = {NULL};   // Fills up with characters to be converted to a double and stored in numbers2
  operand numbers2[32];         // Stores all the doubles with parenthesesDepth
  operation ops[31];            // Stores all our operation characters with parenthesesDepth
  int k=0;                      // Cursor for numbers1
  int l=0;                      // Cursor for numbers2
  int m=0;                      // Cursor for ops
  bool isNegative = 0;
  bool isReal = 1;
  double tempNumber = 0;
  int parenthesesLevel = 0;     // The current parentheseLevel
  for(int i=0; i<=31; i++) {
    if(totalInput[i] == '-') {
      isNegative = 1;
      if(k!=0) { // Negative symbol must be at the beginning of a number
        errorHandler("Syntax ERROR");
      }
    } else if(totalInput[i] == 'j') {
      isReal = 0;
      if(k!=0) { // j must be at the beginning of a number
        errorHandler("Syntax ERROR");
      }
    } else if(totalInput[i] == '(') {
      parenthesesLevel++;
    } else if(totalInput[i] == ')') {
      parenthesesLevel--;
      if (parenthesesLevel < 0){
        errorHandler("Syntax ERROR");
      }
    } else if((totalInput[i] >= 0x30 && totalInput[i] <= 0x39) || totalInput[i] == '.') { // If it's a number or '.', store the character
        numbers1[k] = totalInput[i];
        k++;
    } else if(totalInput[i] == '+' ||  totalInput[i] == '_' || totalInput[i] == '*' || totalInput[i] == '/' || totalInput[i] == '^') { // If it's an operation
      k=0;
      tempNumber = atof(numbers1); // Convert and store the number that was built
      if (isNegative){
        tempNumber = -1*tempNumber;
      }
      if (isReal) {
        numbers2[l].realComponent = tempNumber;
      } else {
        numbers2[l].imaginaryComponent = tempNumber;
      }
      isReal = 1;      
      numbers2[l].parenthesesDepth = parenthesesLevel;
      for(int i=0; i<32; i++) {     // Clear numbers1
        numbers1[i] = {NULL};
      }
      l++;
      ops[m].op = totalInput[i];    // Store the operation
      ops[m].parenthesesDepth = parenthesesLevel;
      m++;
      if(totalInput[i-1] == '+' ||  totalInput[i-1] == '_' || totalInput[i-1] == '*' || totalInput[i-1] == '/' || totalInput[i-1] == '^') { // Operations can't be back to back
        errorHandler("Syntax ERROR");
      }
    } else if(totalInput[i] == NULL) { // We are at the end of the array
      if (parenthesesLevel != 0){
        errorHandler("Syntax ERROR");
      }
      k=0;
      tempNumber = atof(numbers1); // Convert and store the number that was built
      if (isNegative){
        tempNumber = -1*tempNumber;
      }
      if (isReal) {
        numbers2[l].realComponent = tempNumber;
      } else {
        numbers2[l].imaginaryComponent = tempNumber;
      }
      numbers2[l].parenthesesDepth = parenthesesLevel;
      l++; 
      m++;
      return orderOfOps(numbers2, ops, l, m, 0); // Calculate and return the answer
    }
  }
}

operand orderOfOps(operand numbers[32], operation ops[31], int sizeNumbers, int sizeOps, int parenthesesLevel)
{
  int leftNum=0;
  int rightNum=0;
  operand trash;    // The return value of orderOfOps is not important until the final answer has been reached
  
  for(int i=0; i < sizeNumbers; i++) {     // If there is a greater parenthesesDepth than our current level, call orderOfOps with the next level
    if (numbers[i].parenthesesDepth > parenthesesLevel) {
      trash = orderOfOps(numbers, ops, sizeNumbers, sizeOps, parenthesesLevel+1);
      i=sizeNumbers;
    }
  }
  
  for(int i=0; i < sizeOps; i++) {  // Look for a '^'
    if(ops[i].op == '^' && ops[i].parenthesesDepth == parenthesesLevel) {
      int leftNum = i;
      int rightNum = i+1;
      while(numbers[leftNum].realComponent == nothing && numbers[leftNum].imaginaryComponent == nothing) {  // Find the number to the left
        leftNum--;
      }
      while(numbers[rightNum].realComponent == nothing && numbers[rightNum].imaginaryComponent == nothing) { // Find the number to the right
        rightNum++;
      }
      //if(numbers[leftNum].parenthesesDepth == parenthesesLevel && numbers[rightNum].parenthesesDepth == parenthesesLevel) {
        numbers[leftNum] = calculate(numbers[leftNum], ops[i].op, numbers[rightNum]); // Calculate the answer, storing it to the left
        numbers[rightNum].realComponent = nothing; //clear the value to the right, and the operation
        numbers[rightNum].imaginaryComponent = nothing; 
        ops[i].op = NULL;
      //}
    }
  }
  for(int i=0; i < sizeOps; i++) { // Look for a '*' or '/'
    if((ops[i].op == '*' || ops[i].op == '/') && ops[i].parenthesesDepth == parenthesesLevel) {
      int leftNum = i;
      int rightNum = i+1;
      while(numbers[leftNum].realComponent == nothing && numbers[leftNum].imaginaryComponent == nothing) { 
        leftNum--;
      }
      while(numbers[rightNum].realComponent == nothing && numbers[rightNum].imaginaryComponent == nothing) {
        rightNum++;
      }
      numbers[leftNum] = calculate(numbers[leftNum], ops[i].op, numbers[rightNum]);
      numbers[rightNum].realComponent = nothing;
      numbers[rightNum].imaginaryComponent = nothing;        
      ops[i].op = NULL;
    }
  }
  for(int i=0; i < sizeOps; i++){ // Look for a '+' or '-'
    if((ops[i].op == '+' || ops[i].op == '_')  && ops[i].parenthesesDepth == parenthesesLevel) {
      int leftNum = i;
      int rightNum = i+1;
      while(numbers[leftNum].realComponent == nothing && numbers[leftNum].imaginaryComponent == nothing) {
        leftNum--;
      }
      while(numbers[rightNum].realComponent == nothing && numbers[rightNum].imaginaryComponent == nothing) {
        rightNum++;
      }
        numbers[leftNum] = calculate(numbers[leftNum], ops[i].op, numbers[rightNum]);
        numbers[rightNum].realComponent = nothing;
        numbers[rightNum].imaginaryComponent = nothing;        
        ops[i].op = NULL;
    }
  }  
 
  for(int i=0; i < sizeNumbers; i++) { // Decriment the parenthesesDepth of all answers
    if (numbers[i].parenthesesDepth == parenthesesLevel) {
      numbers[i].parenthesesDepth--;
    }
  }
  return numbers[0];
}

operand calculate(operand left, char op, operand right)
{
  double divisor = 0;
  if (left.realComponent == nothing){
    left.realComponent = 0;
  }
  if (left.imaginaryComponent == nothing){
    left.imaginaryComponent = 0;
  }
  if (right.realComponent == nothing){
    right.realComponent = 0;
  }    
  if (right.imaginaryComponent == nothing){
    right.imaginaryComponent = 0;
  }  
    
  operand resultant;
  switch(op) {
    case '+':
      resultant.realComponent = left.realComponent+right.realComponent;
      resultant.imaginaryComponent = left.imaginaryComponent+right.imaginaryComponent;
      break;
    case '_':
      resultant.realComponent = left.realComponent-right.realComponent;
      resultant.imaginaryComponent = left.imaginaryComponent-right.imaginaryComponent;
      break;
    case '*': // FOIL the components
      resultant.realComponent = left.realComponent*right.realComponent-left.imaginaryComponent*right.imaginaryComponent;
      resultant.imaginaryComponent = left.imaginaryComponent*right.realComponent+right.imaginaryComponent*left.realComponent;
      break;
    case '/': // Performs complex division
      divisor = pow(right.realComponent,2)+pow(right.imaginaryComponent,2);
      resultant.realComponent = (left.realComponent*right.realComponent+left.imaginaryComponent*right.imaginaryComponent)/divisor;
      resultant.imaginaryComponent =(left.imaginaryComponent*right.realComponent-left.realComponent*right.imaginaryComponent)/divisor; 
      if(resultant.realComponent != resultant.realComponent || resultant.imaginaryComponent != resultant.imaginaryComponent) {
        errorHandler("/ by 0 ERROR");
      }
      break;
    case '^': // Exponents currently only work with real numbers
      if(left.imaginaryComponent == 0 && right.imaginaryComponent == 0){
        resultant.realComponent = pow(left.realComponent,right.realComponent);  
        if(resultant.realComponent != resultant.realComponent) {
          left.realComponent = -1*left.realComponent;               //flips the polarity of left so the calculation can occur
          resultant.realComponent = 0;
          resultant.imaginaryComponent = pow(left.realComponent,right.realComponent);
          resultant.imaginaryComponent = 1/resultant.imaginaryComponent;   //I don't know why the answer is the inverse of what it should be. This flips it back.       
        }         
      } else {
        errorHandler("Does not compute");
      }
      break;
  }
  return resultant;
}

char getKey()
{
  char key[16]={'1','2','3','+','4','5','6','_','7','8','9','*','X','0','=','/'};
  if(!digitalRead(4)) // Check if using shift
  {
    key[0]='(';
    key[1]=')';
    key[3]='j';
    key[4]='<';
    key[5]='>';
    key[7]='-';
    key[11]='^';
    key[12]='x';
    key[14]='A';
    key[15]='.';
  }
  for(int i=5; i<=8; i++) { // Scan keypad rows
    digitalWrite(i, LOW);
    for(int j=9; j<=12; j++) { // Scan keypad columns
      if(!digitalRead(j)){
        while(!digitalRead(j)){} // Wait for key to be released
        digitalWrite(i,HIGH);
        return key[j-9+4*(i-5)];
      }
    }
    digitalWrite(i,HIGH);
  }
  return NULL; // Default the input to NULL
}

void errorHandler(char errorCode[])
{
  if(!reset) {  // Skip this error if an error has already been triggered
    lcd.clear();
    lcd.print(errorCode);
    reset = 1;  // Trigger a reset
    delay(200);
    while(getKey() == NULL){} // Wait for an input
  }
}
