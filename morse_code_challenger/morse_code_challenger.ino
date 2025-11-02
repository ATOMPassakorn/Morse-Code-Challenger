#include <LiquidCrystal_I2C.h>
#include <WiFiS3.h>

// [การตั้งค่า WiFi]
char ssid[] = "";
char pass[] = "";

WiFiServer server(80);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BTN = 2;
const int BTN_2 = 3;
const int enterBTN = 4;
const int deleteBTN = 5;
const int red = 8;
const int green = 9;
const int blue = 10;
bool firstEnter = true;
bool randomWord = true;
bool gameOver = false;

int col = 0;
int row = 0;
int opCol = 0;
int ctn = 0;

String morseCode = "";
String output = "";
String targetWord = "";

struct MorsePair {
  const char* code;
  char letter;
};

MorsePair morseTable[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'}
};

char decodeMorse(String code) {
  for (int i = 0; i < sizeof(morseTable) / sizeof(morseTable[0]); i++) {
    if (code == morseTable[i].code) {
      return morseTable[i].letter;
    }
  }
  return '?'; // ถ้าไม่เจอ
}

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print(" Hello, Welcome!");
  lcd.setCursor(0,1);
  lcd.print("Morse Code Game!");
  pinMode(BTN, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(enterBTN, INPUT_PULLUP);
  pinMode(deleteBTN, INPUT_PULLUP);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);


  Serial.println("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    //Note: delay(1000) ใน setup ไม่กระทบต่อ Web Server
    delay(1000); 
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Web Server Started.");

  server.begin();

}

void loop()
{
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("/setFirstEnter") != -1) {
      lcd.clear();
      firstEnter = false;  // เปลี่ยนค่า
      client.println("HTTP/1.1 200 OK");
      client.println("Access-Control-Allow-Origin: *"); // ถ้า CORS ปัญหา
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("OK");
    }

    if (req.indexOf("/setTimer") != -1) {
      digitalWrite(red, LOW);
      delay(1000);
      digitalWrite(red, HIGH);
      morseCode = "";
      output = "";
      targetWord = "";
      randomWord = true;
      gameOver = false;
      firstEnter = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" Hello, Welcome!");
      lcd.setCursor(0,1);
      lcd.print("Morse Code Game!");
      
      client.println("HTTP/1.1 200 OK");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("OK");
    }


    // ส่ง JSON ข้อมูลเมื่อมีคำขอ "/data"
    if (req.indexOf("/data") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print("{\"morse\":\"");
      client.print(morseCode);
      client.print("\",\"decoded\":\"");
      client.print(output);
      client.print("\",\"target\":\"");
      client.print(targetWord);
      client.print("\",\"gameOver\":");
      client.print(gameOver ? "true" : "false");
      client.println("}");
      if (gameOver) gameOver = false;

    }

    client.stop();
  }

  if (randomWord){ //สุ่มคำตามอักษรใน struct
    for (int i = 0; i < 5; i++) {
      int index = random(0, sizeof(morseTable) / sizeof(morseTable[0]));
      targetWord += morseTable[index].letter;
    }
    randomWord = false;
  }

  if (output == targetWord){
    firstEnter = true;
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Correct!");
    lcd.setCursor(4,1);
    lcd.print("Good job.");
    digitalWrite(green, LOW);
    targetWord = "";
    output = "";
    ctn = 0;
    opCol = 0;
    delay(1000);
    digitalWrite(green, HIGH);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Hello, Welcome!");
    lcd.setCursor(0,1);
    lcd.print("Morse Code Game!");
    randomWord = true;
    firstEnter = true;
    gameOver = true;
  }

  Serial.println(targetWord);
  Serial.println(firstEnter);
  //Serial.println(sizeof(morseTable) / sizeof(morseTable[0]));

  if (!firstEnter){ //เอาไว้ปริ้นคำว่า code: ตอนเริ่ม
    lcd.setCursor(col,row);
    lcd.print("Code:"+morseCode);
    lcd.setCursor(col,1);
    lcd.print("Text:");
  }

  if (opCol > 16){ //ถ้าเกิน 16 ช่องลบทิ้ง
    opCol = 0;
    lcd.clear();
    lcd.setCursor(0, 1);
  }

  if (firstEnter && (digitalRead(BTN)==LOW || digitalRead(BTN_2)==LOW)) {
    firstEnter = false;
    lcd.clear();
    delay(200);
  }


  if (digitalRead(BTN) == LOW) {
    lcd.setCursor(col,row);
    morseCode += "-";
    ctn++;
    delay(200);
  }
  if (digitalRead(BTN_2) == LOW){
    lcd.setCursor(col,row);
    morseCode += ".";
    ctn++;
    delay(200);
  }

  if(digitalRead(enterBTN) == LOW && digitalRead(deleteBTN) == LOW){
    if (morseCode.length() > 0) {
      morseCode.remove(morseCode.length() - 1); // ลบสัญลักษณ์สุดท้ายในรหัสมอร์ส
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Code:" + morseCode);
      lcd.setCursor(0, 1);
      lcd.print(output);
      delay(300);
    }
  }

  if (digitalRead(deleteBTN) == LOW){//กดพร้อมกันเเล้วลบตัวอักษรตัวนึง
    if (output.length() > 0) {
      output.remove(output.length() - 1); // ลบตัวสุดท้ายออก
      opCol--;
      if (opCol < 0) opCol = 0;
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print(output);
      delay(300);
    }
  }

  if (ctn > 5){
    morseCode = "";
    ctn = 0;
    lcd.clear();
    delay(100);
    lcd.setCursor(0,1);
    lcd.print(output);
  }
  
  if (digitalRead(enterBTN) == LOW){
    lcd.clear();
    lcd.setCursor(5,1);
    delay(200);
    ctn = 0;

    char decoded = decodeMorse(morseCode);
    if (decoded != '?'){
      output += decoded;
    };

    lcd.print(output);
    opCol++;
    morseCode = "";
  }

}