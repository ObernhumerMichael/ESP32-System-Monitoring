#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600); // Initialize serial communication at 9600 baud
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Debug message: Hello, world!");
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}