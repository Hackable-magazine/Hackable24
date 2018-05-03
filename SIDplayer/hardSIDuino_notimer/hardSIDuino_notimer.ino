#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

// A0 -> A4 = ADD0 -> ADD4

#define RESETPIN 12
#define CSPIN 11

char newsid[25];
char oldsid[25];
bool dataready = false;
char buffer[50];
int idx = 0;
unsigned long lastupdate = 0;
int periode = ((1*1000)/50); // 50hz = 20 ms

void Poke(char addr, char val)
{
  digitalWrite(A0,(addr & 0x1));
  digitalWrite(A1,(addr & 0x2));
  digitalWrite(A2,(addr & 0x4));
  digitalWrite(A3,(addr & 0x8));
  digitalWrite(A4,(addr & 0x10));

  digitalWrite(D0,(val & 0x1));
  digitalWrite(D1,(val & 0x2));
  digitalWrite(D2,(val & 0x4));
  digitalWrite(D3,(val & 0x8));
  digitalWrite(D4,(val & 0x10));
  digitalWrite(D5,(val & 0x20));
  digitalWrite(D6,(val & 0x40));
  digitalWrite(D7,(val & 0x80));

  digitalWrite(CSPIN, LOW);
  digitalWrite(CSPIN, HIGH);
}

void resetSID()
{
  char addr;

  digitalWrite(RESETPIN, LOW);
  delay(20);
  digitalWrite(RESETPIN, HIGH);
  delay(20);

  for(addr = 0; addr < 25; addr++)
    Poke(addr, 0);
  delay(10);
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(RESETPIN, OUTPUT);
  digitalWrite(RESETPIN, HIGH);

  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);

  delay(20);
  resetSID();
  delay(20);
}

char decode(char ch)
{
  if (ch >= 'A')
    return ch - 55;
  else
    return ch - 48;
}

void updateSID()
{
  for (int i = 0; i < 25; i++) {
    if (oldsid[i] != newsid[i]) {
      oldsid[i] = newsid[i];
      Poke(i, newsid[i]);
    }
  }
}

void readData()
{
  if (Serial.available() > 0) {
      char ch = Serial.read();
      if (ch == 'R') {
        resetSID();
        return;
      }
      if (ch == '!') {
        if (idx == 50) {
          for (int i = 0; i < 25; i++) {
            char highnibble = buffer[i * 2];
            char lownibble = buffer[i * 2 + 1];
            newsid[i] = (decode(highnibble) << 4) | decode(lownibble);
          }
          dataready = true;
        }
        idx = 0;
      } else {
        if (idx < 50)
          buffer[idx] = ch;
        idx++;
      }
  }
}

void loop()
{
  if (!dataready)
    readData();

  unsigned long timenow = millis();
  if (timenow - lastupdate >= periode) {
    lastupdate = timenow;
    if (dataready == true) {
      dataready = false;
      updateSID();
    }

    Serial.write('?');
    Serial.flush();
  }
}
