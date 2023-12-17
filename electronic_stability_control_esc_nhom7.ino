#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

int in1 = 6; 
int in2 = 5; 
int in3 = 4; 
int in4 = 3;
int EnB = 2; //điều khiển speed bánh trái trước
int EnA= 7; //điều khiển speed bánh phải trước
int in1s = 12;
int in2s = 11;
int in3s = 10;
int in4s = 9;
int EnBs = 8; //điều khiển speed bánh trái sau
int EnAs= 13; //điều khiển speed bánh phải sau
void setup()
{

  //khởi tạo các chân output
  pinMode(22, OUTPUT);  //Bánh trước trái
  pinMode(24, OUTPUT); //Bánh trước phải
  pinMode(26, OUTPUT); //Bánh sau trái
  pinMode(28, OUTPUT); //Bánh sau phải 
  
  pinMode(EnA, OUTPUT);
  pinMode(EnB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(EnAs, OUTPUT);
  pinMode(EnBs, OUTPUT);
  pinMode(in1s, OUTPUT);
  pinMode(in2s, OUTPUT);
  pinMode(in3s, OUTPUT);
  pinMode(in4s, OUTPUT);

  // cấu hình timer 2 để thiếp lập tốc độ của xe với OCR2A là thanh ghi 8 bit (0-255) lưu giá trị tốc độ xe được khởi tạo là 255
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS22);
  OCR2A = 255;
  // khởi tạo lcd
  lcd.begin(16, 2);
  lcd.setCursor(5, 0);
  lcd.print("WELCOME");
  lcd.setCursor(3, 1);
  lcd.print("NHOM 7 DSC");
  // Khởi tạo Serial Monitor
  Serial.begin(9600);
  Serial.println("Welcome");
  delay(1000); // delay 1s
}
void directionControl(int a) {
    // Set motors to maximum speed
    // For PWM maximum possible values are 0 to 255
    analogWrite(EnA, a);
    analogWrite(EnB, a);
    analogWrite(EnAs, a);
    analogWrite(EnBs, a);
    // Turn on motor A & B
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in1s, HIGH);
    digitalWrite(in2s, LOW);
    digitalWrite(in3s, HIGH);
    digitalWrite(in4s, LOW);
}

void loop()
{
  // Đọc giá trị từ các cảm biến analog
  int Steering = analogRead(A0); // cảm biến góc lái của vô lăng
  int K = analogRead(A1);  // cảm biến góc lái
  int Speed = analogRead(A2);  // cảm biến tốc độ xe
  int Old_Speed;

  Serial.print("K: ");
  Serial.println(K);
  Serial.print("Steering: ");
  Serial.println(Steering);
  Serial.print("Speed: ");
  Serial.println(Speed);
  Serial.print("Old_Speed: ");
  Serial.println(Old_Speed);


  lcd.setCursor(0, 0);
  lcd.print("NORMAL MODE     ");
  Serial.print("NORMAL MODE. SPEED: ");
  lcd.setCursor(0, 1);
  lcd.print("SPEED");
  lcd.setCursor(7, 1);
  lcd.print(OCR2A);
  Serial.println(OCR2A);

  if (OCR2A <= 99)
  {
    lcd.setCursor(9, 1);
    lcd.print("      ");
  }
  else
  {
    lcd.setCursor(11, 1);
    lcd.print("        ");
  }
  /*
     với Old_Speed không thay đổi và cảm biến góc lái
     trong khoảng từ (450;550) thì xe ở trạng thái NORMAL
     nghĩa là không xảy ra thừa hoặc thiếu lái,
     khi đó sẽ liên tục đọc giá trị K
  */
  while (Old_Speed == analogRead(A2) && (K > 450 && K < 550))
  {
    K = analogRead(A1);
    Serial.print("K: ");
    Serial.println(K);
    lcd.setCursor(10, 1);
    lcd.print("      ");
  }
  // set tốc độ xe
  if (Speed >= 0 && Speed < 200)
  {
    OCR2A = 50;
  }
  else if (Speed > 200 && Speed <= 400)
  {
    OCR2A = 100;
  }
  else if (Speed > 400 && Speed <= 600)
  {
    OCR2A = 150;
  }
  else if (Speed > 600 && Speed <= 800)
  {
    OCR2A = 200;
  }
  else if (Speed > 800 && Speed <= 1024)
  {
    OCR2A = 255;
  }
  directionControl(OCR2A);
  delay(1000);
  // Lưu giá trị tốc độ ban đầu để điều chỉnh lại tốc độ sau khi giảm tốc 
  OCR2B = OCR2A;
  // gán cho counter = 0 để khi có thừa lái hoặc thiếu lái thì sẽ gán Old_Speed = Speed, sau đó khi thay đổi K trong khỏang
  // cho phép sẽ kiểu tra nếu tốc độ xe giữ nguyên thì về trạng thái bình thường

  // trường hợp K < 450 nghĩa là đang thừa lái
  int counter = 0;
  while (K < 450)
  {
    if (counter == 0)
    {
      Old_Speed = Speed;
      counter += 1;
    }

    Steering = analogRead(A0);
    Serial.print("Steering: ");
    Serial.println(Steering);
    Serial.print("OVERSTEER DTCTED. SPEED: ");
    Serial.println(OCR2A);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("OVERSTEER DTCTED");
    lcd.setCursor(0, 1);
    lcd.print("SPEED");
    lcd.setCursor(7, 1);
    lcd.print(OCR2A);
    lcd.setCursor(12, 1);
    lcd.print("BKHN");

    /* Nếu cảm biến vô lăng nhỏ hơn 512 nghĩa là đang thừa lái bên trái, đèn bên phải trước sẽ báo hiệu
      và ngược lại .........lớn.....................................phải,........trái.......................
      Sau đó xe sẽ giảm tốc độ
    */
    if (Steering < 512)
    {
      digitalWrite(24, HIGH);
      digitalWrite(22, LOW);
      // giảm tốc độ xe
      speedControlRightBefore();
      delay(200);
    }
    if (Steering >= 512)
    {
      digitalWrite(22, HIGH);
      digitalWrite(24, LOW);
      // giảm tốc độ xe
      speedControlLeftBefore();
      delay(200);
    }
    K = analogRead(A1);
  }
  // khi được điều chỉnh thừa lái xong, xe về trạng thái normal 2 đèn báo phía trước sẽ về trạng thái off
  digitalWrite(22, LOW);
  digitalWrite(24, LOW);

  counter = 0;

  // trường hợp đang thiếu lái
  while (K > 550)
  {
    if (counter == 0)
    {
      Old_Speed = Speed;
      counter += 1;
    }

    Steering = analogRead(A0);
    Serial.print("Steering: ");
    Serial.println(Steering);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("UNDERSTEER DTCTED");
    lcd.setCursor(0, 1);
    lcd.print("SPEED");
    lcd.setCursor(7, 1);
    Serial.print("UNDERSTEER DTCTED. SPEED: ");
    Serial.println(OCR2A);
    lcd.print(OCR2A);
    lcd.setCursor(12, 1);
    lcd.print("BKHN");

    /* Nếu cảm biến vô lăng nhỏ hơn 512 nghĩa là đang thiếu lái  bên trái, đèn bên trái sau sẽ báo hiệu
      và ngược lại .........lớn......................................phải,.........phải......................
      Sau đó xe sẽ giảm tốc độ
    */
    if (Steering < 512)
    {
      digitalWrite(26, HIGH); 
      digitalWrite(28, LOW);
      speedControlLeftAfter();
      delay(200);
    }
    if (Steering >= 512)
    {
      digitalWrite(28, HIGH);
      digitalWrite(26, LOW);
      speedControlRightAfter();
      delay(200);
    }
    K = analogRead(A1);
  }
  // khi được điều chỉnh thiếu lái xong, xe về trạng thái normal 2 đèn báo phía sau sẽ về trạng thái off
  digitalWrite(26, LOW);
  digitalWrite(28, LOW);
}
void speedControlLeftBefore() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in1s, HIGH);
    digitalWrite(in2s, LOW);
    digitalWrite(in3s, HIGH);
    digitalWrite(in4s, LOW);
    
    analogWrite(EnB, OCR2A);
    analogWrite(EnA, OCR2B);
    analogWrite(EnBs, OCR2B);
    analogWrite(EnAs, OCR2B);
    if (OCR2A > 24)
    {
      OCR2A = OCR2A - 25;
    }
    else 
    {
      delay(1000);
      OCR2A = OCR2B;
    }
}
void speedControlRightBefore() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in1s, HIGH);
    digitalWrite(in2s, LOW);
    digitalWrite(in3s, HIGH);
    digitalWrite(in4s, LOW);

    analogWrite(EnA, OCR2A);
    analogWrite(EnB, OCR2B);
    analogWrite(EnBs, OCR2B);
    analogWrite(EnAs, OCR2B);
    if (OCR2A > 24)
    {
      OCR2A = OCR2A - 25;
    }
    else 
    {
      delay(1000);
      OCR2A = OCR2B;
    }
}
void speedControlLeftAfter() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in1s, HIGH);
    digitalWrite(in2s, LOW);
    digitalWrite(in3s, HIGH);
    digitalWrite(in4s, LOW);
    
    analogWrite(EnBs, OCR2A);
    analogWrite(EnA, OCR2B);
    analogWrite(EnB, OCR2B);
    analogWrite(EnAs, OCR2B);
    if (OCR2A > 24)
    {
      OCR2A = OCR2A - 25;
    }
    else 
    {
      delay(1000);
      OCR2A = OCR2B;
    }
}
void speedControlRightAfter() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in1s, HIGH);
    digitalWrite(in2s, LOW);
    digitalWrite(in3s, HIGH);
    digitalWrite(in4s, LOW);
    // Decelerate from maximum speed to zero

    analogWrite(EnAs, OCR2A);
    analogWrite(EnBs, OCR2B);
    analogWrite(EnA, OCR2B);
    analogWrite(EnB, OCR2B);
    if (OCR2A > 24)
    {
      OCR2A = OCR2A - 25;
    }
    else 
    {
      delay(1000);
      OCR2A = OCR2B;
    }
}