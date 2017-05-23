

unsigned char buffer[64];  // buffer array for data receive over serial port
int count = 0;             // counter for buffer array
bool rPress = false;
bool gPress = false;
bool keepChecking = true;
#define signalPin1  8
#define signalPin2  9
int G_PIN = 21;
int R_PIN = 20;
int redPin = 2;
int greenPin = 4;
int bluePin = 3;
int yesCount = 0;
int yesCopy;
int noCount = 0;
int noCopy;
int myCount = 0;
int signalLevel;
unsigned long start, current, difference, resetTimer, timeSinceReset;
unsigned long gButtonPress1, gButtonPress2, rButtonPress1, rButtonPress2;


void setup()
{
  start = millis();
  resetTimer = millis();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(signalPin1, OUTPUT);
  pinMode(signalPin2, OUTPUT);
  setColor(255, 0, 0);
  digitalWrite(signalPin1, 0);
  digitalWrite(signalPin2, 0);

  Serial1.begin(9600);
  Serial.begin(9600);
  
  pinMode( G_PIN, INPUT_PULLUP);
  pinMode( R_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(G_PIN), button1_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(R_PIN), button2_ISR, FALLING);
  //uncomment if problems to check status of chip
   checkStatus();
 // analogWrite(signalPin1, 0);
   checkSignal();
   setUpCom();

  setColor(0, 255, 0);
}
  void loop()
  {     
      if(goodSignal())
      {
        setColor(0, 255, 0);
      }
      else
      {
        setColor(255, 0, 0);
      }

      current = millis();
      difference = current - start;
      timeSinceReset = current - resetTimer;
      //rerun initlaization every 25 minutes or whenever overflow happens
      if (timeSinceReset > 1220000 || timeSinceReset < 0)
      {
        analogWrite(redPin, 255);
        analogWrite(greenPin, 150);
        analogWrite(bluePin, 0);
        setUpCom();
        resetTimer = millis();
    
        analogWrite(redPin, 0);
        analogWrite(greenPin, 255);
        analogWrite(bluePin, 0);
      }
      //every 50 days or so millis resets to 0- when this happens just skip one send and start over again
      if (difference < 0)
      {
        start = millis();
      }
      //send every 10 minutes
      if (difference > 600000)
      {
         if(goodSignal())
         {
          analogWrite(redPin, 255);
          analogWrite(greenPin, 150);
          analogWrite(bluePin, 0);
          yesCopy = yesCount;
          noCopy = noCount;
          yesCount = 0;
          sendYes(yesCopy);
          yesCount = 0;
          sendNo(noCopy);
          noCount = 0;
          start = millis();
          analogWrite(redPin, 0);
          analogWrite(greenPin, 255);
          analogWrite(bluePin, 0);
        }
        else
        {
          setColor(255, 0, 0);
          digitalWrite(signalPin1, 0);
          digitalWrite(signalPin2, 0);
          Serial.println("Signal not good enough to send");
          checkSignal();
        }
      }
  }


  void clearBufferArray()
  {
    for (int i = 0; i < count; i++)
    {
      buffer[i] = NULL;
    }
  }

  void sendSMS(String message) {
    Serial1.write("AT+CMGF=1\n\r");
    delay(3000);
    while (Serial1.available())
    {
      buffer[count++] = Serial1.read();
      if (count == 64)break;
    }
    Serial.write(buffer, count);
    clearBufferArray();
    count = 0;

    Serial1.write("AT+CMGS=\"+17752330173\"\n\r");
    delay(3000);
    while (Serial1.available())
    {
      buffer[count++] = Serial1.read();
      if (count == 64)break;
    }
    Serial.write(buffer, count);
    clearBufferArray();
    count = 0;

    Serial1.write(message.c_str());
    outputIncomingMessage();
    Serial1.write((char)26);
    delay(3000);
  }

  void outputIncomingMessage() {
    delay(3000);

    receiveResponse();
    Serial.write(buffer, count);
    clearBufferArray();
    count = 0;
  }



  void checkStatus() {


    Serial1.write("AT\n\r");

    
    outputIncomingMessage();
    Serial1.write("AT+CMEE=2\n\r");
    
    outputIncomingMessage();
    Serial1.write("AT+CPIN?\n\r");

    
    
    outputIncomingMessage();
    Serial1.write("AT+CREG?\n\r");
    outputIncomingMessage();
    Serial1.write("AT+CGATT?\n\r");
    outputIncomingMessage();
    Serial1.write("AT+CSQ\n\r");
    outputIncomingMessage();
  }

  void receiveResponse() {
    while (Serial1.available())
    {
      buffer[count++] = Serial1.read();
      if (count == 64)
        break;
    }
  }

  void setUpCom() {
    Serial1.write("AT+CFUN=1,1\n\r");
    outputIncomingMessage();
    Serial1.write("AT+CSTT\n\r");
    outputIncomingMessage();
    //SET CONNECTION type to GPRS
    Serial1.write("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n\r");
    outputIncomingMessage();
    //set the APN - comes from ting sim card
    Serial1.write("AT+SAPBR=3,1,\"APN\",\"wholesale\"\n\r");
    outputIncomingMessage();
    delay(5000);
    //enable GPRS can take some time thus extra delay
    Serial1.write("AT+SAPBR=1,1\n\r");
    delay(3000);
    outputIncomingMessage();
    //check to see if connection is correct - returns ip address
    Serial1.write("AT+SAPBR=2,1\n\r");
    outputIncomingMessage();

  }


  void sendNo(int quantity) {
    //enable HTTP mode
    Serial1.write("AT+HTTPINIT\n\r");
    outputIncomingMessage();
    //SET HTTP PROFILE IDENTIFIER
    Serial1.write("AT+HTTPPARA=\"CID\",1\n\r");
    outputIncomingMessage();
    //CONNECT TO WEBSITE PHP COMMAND INCLUDED
    Serial1.write("AT+HTTPPARA=\"URL\",\"http://oxysensors.net/addButton.php");
    outputIncomingMessage();
    String message1 = "?ID=0&Vote=NO&Quantity=" + String(quantity);
    message1 = message1 + "\"\n\r";

    Serial1.write(message1.c_str());

    outputIncomingMessage();
    //SEND OUT GET REQUEST
    Serial1.write("AT+HTTPACTION=0\n\r");
    outputIncomingMessage();
    terminateConnection();
  }

  void sendYes(int quantity) {
    //enable HTTP mode
    Serial1.write("AT+HTTPINIT\n\r");
    outputIncomingMessage();
    //SET HTTP PROFILE IDENTIFIER
    Serial1.write("AT+HTTPPARA=\"CID\",1\n\r");
    outputIncomingMessage();
    //CONNECT TO WEBSITE PHP COMMAND INCLUDED
    Serial1.write("AT+HTTPPARA=\"URL\",\"http://oxysensors.net/addButton.php");

    outputIncomingMessage();
    String message1 = "?ID=0&Vote=YES&Quantity=" + String(quantity);
    message1 = message1 + "\"\n\r";

    Serial1.write(message1.c_str());
    outputIncomingMessage();
    //SEND OUT GET REQUEST
    Serial1.write("AT+HTTPACTION=0\n\r");
    outputIncomingMessage();
    Serial1.write("AT+HTTPREAD\n\r");
    terminateConnection();
  }


  void terminateConnection()
  {
    Serial1.write("AT+HTTPTERM\n\r");
    outputIncomingMessage();
  }


  void button1_ISR()
  {
    if (keepChecking)
    {
      keepChecking = false;
    }
    else{
        gButtonPress2 = millis();
        if (gButtonPress2 - gButtonPress1 > 1100)
         {
          yesCount++;
          Serial.println("yes's");
          Serial.println(yesCount);
          gButtonPress1 = millis();
          }
    }
  }



  void button2_ISR()
  {

    if(!keepChecking)
    {
      rButtonPress2 = millis();
  
      if (rButtonPress2 - rButtonPress1 > 1100)
      {
        noCount++;
        Serial.println("no's");
        Serial.println(noCount);
        rButtonPress1 = millis();
      }
    }
  }
  void debug() {

    if (Serial1.available())
    {
      while (Serial1.available())
      {
        buffer[count++] = Serial1.read();
        if (count == 64)break;
      }
      Serial.write(buffer, count);
      clearBufferArray();
      count = 0;
    }
    if (Serial.available())
      Serial1.write(Serial.read());
  }

  int checkSignal() {
    do {
      Serial.println("Checking Signal");
      clearBufferArray();

      Serial1.write("AT+CSQ\n\r");
      for (int i = 0; i < 7; i++)
        Serial1.read();
      delay(1000);

      if (Serial1.available())
        receiveResponse();

      Serial.write(buffer, count);
      count = 0;

      Serial.println(buffer[11]);
      Serial.println(char(buffer[11]));

      if (buffer[11] != 'S')
      {

        Serial.println("Error obtaining Signal");
        digitalWrite(signalPin1, 0);
        digitalWrite(signalPin2, 0);
        keepChecking = true;
      }
      else
      {

        signalLevel = buffer[15] - '0';
        if (buffer[16] != ',')
          signalLevel = signalLevel * 10 + buffer[16] - '0';
        Serial.println(signalLevel);
        if (signalLevel < 9)
        {
          setColor(255, 0, 0);
          digitalWrite(signalPin1, 0);
          digitalWrite(signalPin2, 0);
          keepChecking = true;
          Serial.println("Low Signal B**tch");
        }
        else if (signalLevel < 15)
        {
          setColor(255, 255, 0);
          digitalWrite(signalPin1, 1);
          digitalWrite(signalPin2, 0);
          Serial.println("Ok Signal Dawg but not great");
        }
        else if (signalLevel < 20)
        {
        digitalWrite(signalPin1, 0);
        digitalWrite(signalPin2, 1);
          setColor(100, 255, 0);
          Serial.println("Hey thats pretty good...");
        }

        else if (signalLevel >= 20 && signalLevel <= 30)
        {
          digitalWrite(signalPin1, 1);
          digitalWrite(signalPin2, 1);
          setColor(0, 255, 0);
          Serial.println("You have got the Lionel Richie of signals right now!");
        }

        clearBufferArray();
        delay(1000);
      }
    }while (keepChecking);


  }

  bool goodSignal()
  {
      Serial.println("Checking Signal");
      clearBufferArray();

      Serial1.write("AT+CSQ\n\r");
      for (int i = 0; i < 7; i++)
        Serial1.read();
      delay(1000);

      if (Serial1.available())
        receiveResponse();

      count = 0;

      Serial.println(buffer[11]);
      Serial.println(char(buffer[11]));

      if (buffer[11] != 'S')
      {

        Serial.println("Error obtaining Signal");
        return false;
      }
      else
      {

        signalLevel = buffer[15] - '0';
        if (buffer[16] != ',')
          signalLevel = signalLevel * 10 + buffer[16] - '0';
          Serial.print("Signal Level: ");
        Serial.println(signalLevel);
        if (signalLevel < 9)
        {
          setColor(255, 0, 0);
          Serial.println("Too Low to send");
          return false;
        }
        else 
        {

           setColor(255, 255, 0);
           Serial.println("Signal is good enough to Send");
           return true;
        }

  }
  }

  void setColor(int r, int g, int b)
  {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);

  }


