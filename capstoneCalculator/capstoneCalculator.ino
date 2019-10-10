//*******************************************************************
//        Negative Numbers are in the process of being implemented (Noah)
//
//
//*******************************************************************

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

double nothing = 3.4028235*pow(10,38);
bool isReal = 1;                                          //This flag will be be changed to 0 if resultant is calculated to exist in the imaginary plane

LiquidCrystal_I2C lcd(0x27,20,4);                         // Set the LCD address to 0x27 for a 16 chars and 2 line display

struct operand {
  double realComponent = nothing;
  double imaginaryComponent = nothing;
  int  parenthesesDepth = 0;

};
char getKey();                                                            // Returns the input from the keypad
operand parseInput(char totalInput[32]);                                   // Makes the input numbers and ops, and calls orderOfOps
operand orderOfOps(operand numbers[32], char ops[31], int sizeNumbers, int sizeOps, int parenthesesLevel);     // Calls calculate on numbers and ops in the correct order
double calculate(double left, char op, double right);                     // Returns an answer for an operation preformed on 2 numbers
void errorHandler(char code[]);                                           // Prints the Error code and performs a reset



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
  double answer; // Preform a reset
  char totalInput[32] = {NULL};
  int i= 0 ;
  reset = 0;
  int cursorLocation = 0;
  isReal = 1;
  lcd.clear();
  lcd.blink();
  char input=NULL;
  while(reset == 0) {  
    input = getKey(); 
    if(input == NULL) {}
    else if(input=='X') { // 'X' clears the screen
      reset = 1;
    } else if(input=='x') { // 'x' is backspace THIS DOES NOT WORK WITH '<'
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
    } else if(input == '<') { // Move cursor backward THIS DOES NOT WORK WITH BACKSPACE
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
    } else if(input == 'A') { // The previous answer
      //itoa
    } else { // The character needs printed
      cursorLocation++;
      if(cursorLocation == 16) {
        lcd.setCursor(0,1); //second row
      }
      if(input == '_') {
        lcd.print('-');                  
      } else if(input == '-') {
        lcd.write(0);
      } else {
        lcd.print(input);
      }
      if(input == '=') {
        Serial.println("The totalInput array passed to parseInput is:");
        Serial.println(totalInput);
        answer = parseInput(totalInput).realComponent;
        Serial.println('\n');
        if(reset == 0) {
          lcd.clear();
          lcd.print("= ");
          lcd.print(answer);
          if(isReal == 0) {
            lcd.print('j');
          }
          while(getKey() == NULL) {}
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
  operand numbers2[32];
  char numbers1[32] = {NULL};
  char ops[31] = {NULL};
  int k=0;
  int l=0;
  int m=0;
  bool isNegative = 0;
  bool isReal = 1;
  double tempNumber = 0;
  int parenthesesLevel = 0;
  for(int i=0; i<=31; i++){
    if(totalInput[i] == '-') {
      isNegative = 1;
      if(k!=0) {
        errorHandler("Syntax ERROR");
      }
      } else if(totalInput[i] == 'j') {
        isReal = 0;
        if(k!=0) {
        errorHandler("Syntax ERROR");
      }
    } else if(totalInput[i] == '('){
      parenthesesLevel++;
    } else if(totalInput[i] == ')'){
      parenthesesLevel--;
      if (parenthesesLevel < 0){
        errorHandler("Syntax ERROR");
      }
    }  else if((totalInput[i] >= 0x30 && totalInput[i] <= 0x39) || totalInput[i] == '.') { // if it's a number or '.'
    
        numbers1[k] = totalInput[i];
        k++;
        Serial.println("part of a number");
    } else if(totalInput[i] == '+' ||  totalInput[i] == '_' || totalInput[i] == '*' || totalInput[i] == '/' || totalInput[i] == '^') {
      k=0;
      tempNumber = atof(numbers1);
      if (isNegative){
        tempNumber = -1*tempNumber;
      }
      if (isReal) {
        numbers2[l].realComponent = tempNumber;
      } else {
        numbers2[l].imaginaryComponent = tempNumber;
      }
      numbers2[l].parenthesesDepth = parenthesesLevel;
      for(int z=0; z <32; z++) {
        numbers1[z] = {NULL};
      }
      l++;
      ops[m] = totalInput[i];
      m++;
      if(totalInput[i-1] == '+' ||  totalInput[i-1] == '_' || totalInput[i-1] == '*' || totalInput[i-1] == '/' || totalInput[i-1] == '^'){
        errorHandler("Syntax ERROR");
      }
      } else if(totalInput[i] == NULL) {
        if (parenthesesLevel != 0){
          errorHandler("Syntax ERROR");
        }
        k=0;
        Serial.println("The numbers1 array passed to charsToDouble is:");
        Serial.println(numbers1);
        tempNumber = atof(numbers1);
        if (isNegative){
          tempNumber = -1*tempNumber;
        }
        if (isReal) {
          numbers2[l].realComponent = tempNumber;
        } else {
          numbers2[l].imaginaryComponent = tempNumber;
        }
        numbers2[l].parenthesesDepth = parenthesesLevel;
        Serial.println("The numbers2 and ops arrays passed to orderOfOps is:");
        for(int n=0; n<=l; n++) {
          Serial.println(numbers2[n].realComponent);
        }
        l++; 
        m++;
        Serial.println(ops);
        return orderOfOps(numbers2, ops, l, m, 0);
      }
    }
  }

operand orderOfOps(operand numbers[32], char ops[31], int sizeNumbers, int sizeOps, int parenthesesLevel)
{

  int leftNum=0;
  int rightNum=0;
  operand trash;

  for(int i=0; i < sizeNumbers; i++){
    if (numbers[i].parenthesesDepth > parenthesesLevel) {
      trash = orderOfOps(numbers, ops, sizeNumbers, sizeOps, parenthesesLevel+1);
    }
  }
  
    for(int i=0; i < sizeOps; i++){
      if(ops[i] == '^'){
        int leftNum = i;
        int rightNum = i+1;
        while(numbers[leftNum].realComponent == nothing) { // This is where zero breaks because zero as a double = NULL
          leftNum--;
        }
        while(numbers[rightNum].realComponent == nothing) {
          rightNum++;
        }
        if(numbers[leftNum].parenthesesDepth == parenthesesLevel && numbers[rightNum].parenthesesDepth == parenthesesLevel) {
          numbers[leftNum].realComponent = calculate(numbers[leftNum].realComponent, ops[i], numbers[rightNum].realComponent);
          numbers[leftNum].parenthesesDepth--;
          numbers[rightNum].realComponent = nothing;
          ops[i] = NULL;
        }
      }
    }
    for(int i=0; i < sizeOps; i++){
      if(ops[i] == '*' || ops[i] == '/'){
        int leftNum = i;
        int rightNum = i+1;
        while(numbers[leftNum].realComponent == nothing) { // This is where zero breaks because zero as a double = NULL
          leftNum--;
        }
        while(numbers[rightNum].realComponent == nothing) {
          rightNum++;
        }
        if(numbers[leftNum].parenthesesDepth == parenthesesLevel && numbers[rightNum].parenthesesDepth == parenthesesLevel) {
          numbers[leftNum].realComponent = calculate(numbers[leftNum].realComponent, ops[i], numbers[rightNum].realComponent);
          numbers[leftNum].parenthesesDepth--;
          numbers[rightNum].realComponent = nothing;
          ops[i] = NULL;
        }
      }
    }
    for(int i=0; i < sizeOps; i++){
      if(ops[i] == '+' || ops[i] == '_'){
        int leftNum = i;
        int rightNum = i+1;
        while(numbers[leftNum].realComponent == nothing) {
          leftNum--;
        }
        while(numbers[rightNum].realComponent == nothing) {
          rightNum++;
        }
        if(numbers[leftNum].parenthesesDepth == parenthesesLevel && numbers[rightNum].parenthesesDepth == parenthesesLevel) {
          numbers[leftNum].realComponent = calculate(numbers[leftNum].realComponent, ops[i], numbers[rightNum].realComponent);
          numbers[leftNum].parenthesesDepth--;
          numbers[rightNum].realComponent = nothing;
          ops[i] = NULL;
        }
      }
    }  
  return numbers[0];
}

double calculate(double left, char op, double right)
{
  double resultant = 0;
  switch(op) {
    case '+':
      resultant = left+right;
      break;
    case '_':
      resultant = left-right;
      break;
    case '*':
      resultant = left*right;
      break;
    case '/':
      if(right != 0) {
      resultant = left/right;
      } else {
        errorHandler("/ by 0 ERROR");
      }
      break;
    case '^':
    Serial.println("Power operation has recieved the following numbers:");
    Serial.println(right);
    Serial.println(left);
    resultant = pow(left,right);
    if(resultant != resultant) {
      left = -1*left;               //flips the polarity of left so the calculation can occur
      isReal = 0;                   //The original resultant was nonreal
      resultant = pow(left,right);
    }
      /*if(left < 0 && (right < 1 && right > -1)) {
        resultant = pow(left,right);
      } else {
        errorHandler("Not Real Answer");
      }*/
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
    key[4]='<';
    key[5]='>';
    key[7]='-';
    key[11]='^';
    key[12]='x';
    key[14]='A';
    key[15]='.';
  }
  for(int i=5; i<=8; i++){ // Scan keypad rows
    digitalWrite(i, LOW);
    for(int j=9; j<=12; j++){ // Scan keypad columns
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
  lcd.clear();
  lcd.print(errorCode);
  reset = 1;
  delay(200);
  while(getKey() == NULL){}
}
