#include <LiquidCrystal_PCF8574.h>   // библиотека для работы с LCD 1602 
#include <EEPROM.h>
LiquidCrystal_PCF8574 lcd(0x3f);
int buzzPin = 12;         // Активная писчалка. 1- не гудит. 0 - гудит.
int adcPin=2;

float tankSize = 150.; // Boat tank size
float liters = tankSize; // current volume of water
int charsInBar = 16;     // How many characters used as bar 
int startingPosition = 0; // Starting position of the bar
float litersPerDigit = tankSize / charsInBar;
float currentStoredLiters = 0.0; // volume of water stored in ROM  
int period = 2000; //Периодичность обновления информации на дисплее
unsigned long time_now = 0;
volatile int adcVal =0;  // АЦП Значение
volatile float volt=0;   // Вольты
volatile byte b=0; // количество бипов
bool needToRedraw = false; // If we need total redraw of the screen (for reset)

int p20[8]  = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 };
int p40[8]  = { B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000 };
int p60[8]  = { B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100 };
int p80[8]  = { B11110, B11110, B11110, B11110, B11110, B11110, B11110, B11110 };
int p100[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };

void setupLiters()
{
  EEPROM.get(0,liters);
  currentStoredLiters = liters;
}

void redrawBar()
{
  int pos = startingPosition + (int)( liters / litersPerDigit );
  lcd.setCursor(startingPosition,1);
  for ( int i = startingPosition ; i < pos; i++)
  {
    lcd.setCursor(i,1);
    lcd.write( 4 );
  }
  lcd.setCursor(pos,1);
  int newChar = ( fmod( liters, litersPerDigit) / ( litersPerDigit ) ) * 5;
  lcd.write( newChar );
}
void setupLCD()
{
  lcd.begin(16,2);
  lcd.setBacklight(1);

  lcd.setCursor(0 ,0);
// Opening screen
  lcd.print( "   Hello Yura!  ");
  lcd.setCursor(0 ,1);
  lcd.print("  I am ready!!! " );
  delay(2000);
// End of opening screen
  lcd.createChar(0, p20);
  lcd.createChar(1, p40);
  lcd.createChar(2, p60);
  lcd.createChar(3, p80);
  lcd.createChar(4, p100);
  lcd.clear();
  lcd.setCursor(13 ,0);
  lcd.print( "ltr" );
  lcd.setCursor(5 ,0);
  lcd.print( "V" );
  redrawBar();

}

void setup()
{  
   Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  needToRedraw = false;
  setupLiters();
   setupLCD();
   
   pinMode(buzzPin, OUTPUT);                //
   digitalWrite(buzzPin, HIGH); 
   pinMode(2, INPUT_PULLUP);                //Датчик потока
   pinMode(3, INPUT_PULLUP);                //Кнопка Сброс
    
// Прерывания для датчика потока и кнопки "Сброс/D3"  
   attachInterrupt(0, waterSensorInteruption, FALLING);       //interrupt 0 (pin 2 on the Arduino Uno) 
   attachInterrupt(1, fallingReset, FALLING);
   attachInterrupt(1, resetTankToFull, RISING); //interrupt 1 (pin 3 on the Arduino Uno)

}    // End Setup()

void fallingReset()
{
}

void refreshLCD()
{
  if(needToRedraw)
  {
    redrawBar();
    needToRedraw = false;
  }
  float volts = analogRead(adcPin) * 4.16 * 0.0049;
  lcd.setCursor(0,0);
  if( volts < 10 )
  lcd.print(" ");
  lcd.print(volts, 2);
  
  //Set cursor for litres
  lcd.setCursor(9 ,0);
  
  if( liters < 10 )
    lcd.print("00");
  else if( liters < 100 )
    lcd.print("0");
  lcd.print( (int)liters );

  // refresh bar
  int pos = startingPosition + (int)( liters / litersPerDigit );
  lcd.setCursor( pos,1 );
  int newChar = ( fmod( liters, litersPerDigit ) / ( litersPerDigit ) ) * 5;
  lcd.write( newChar );
  lcd.setCursor( pos+1, 1 ); 
  lcd.print( " " );
}
void refreshStoredLiters()
{
  if( liters < currentStoredLiters || needToRedraw )
  {
    currentStoredLiters = (int)( liters / 5 ) * 5;
    EEPROM.put(0,currentStoredLiters);
    EEPROM.get(0,currentStoredLiters);
  }
}
void loop()
{       
            if( millis() >= time_now + period)
            {
              time_now = millis();
              refreshStoredLiters();
              refreshLCD();
            }     
}//  End Loop
      
void waterSensorInteruption()
{ 
   liters = fabs( liters -= 1.0/450 ); 
}

void resetTankToFull()
{
  liters = tankSize;
  needToRedraw = true;
}

