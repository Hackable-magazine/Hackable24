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

// nouvelles valeurs des registres
char newsid[25];
// précédentes valeurs des registres
char oldsid[25];
// drapeau de disponibilité des données
bool dataready = false;
// tampon de lecteur
// 25 registres, 2 caractères par octet
char buffer[50];
// index pour la lecture série
int idx = 0;

void poke(char addr, char val)
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
  delay(1);
  digitalWrite(CSPIN, HIGH);
}

void resetSID()
{
  char addr;

  // Reset SID
  digitalWrite(RESETPIN, LOW);
  delay(20);
  digitalWrite(RESETPIN, HIGH);
  delay(20);

  // Tous les registres inscriptibles à 0
  for(addr = 0; addr < 25; addr++)
    poke(addr, 0);
}

// mise à jour de registre
void updateSID()
{
  for (int i = 0; i < 25; i++) {
    // on n'écrit que si la valeur est différente
    if (oldsid[i] != newsid[i]) {
      oldsid[i] = newsid[i];
      poke(i, newsid[i]);
    }
  }
}

// hex vers 4bits
char decode(char ch)
{
  if (ch >= 'A')
    return ch - 55;
  else
    return ch - 48;
}

// lecture des données
void readData()
{
  if (Serial.available() > 0) {
      char ch = Serial.read();
      // demande de reset par le script Python ?
      if (ch == 'R') {
        resetSID();
        return;
      }
      // données pour le registres ?
      if (ch == '!') {
        // données complètes ?
        if (idx == 50) {
          // décodage
          for (int i = 0; i < 25; i++) {
            char highnibble = buffer[i * 2];
            char lownibble = buffer[i * 2 + 1];
            newsid[i] = (decode(highnibble) << 4) | decode(lownibble);
          }
          // données prêtes
          dataready = true;
        }
        idx = 0;
      } else {
        // accumulation
        if (idx < 50)
          buffer[idx] = ch;
        idx++;
      }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // setup reset
  pinMode(RESETPIN, OUTPUT);
  digitalWrite(RESETPIN, HIGH);

  // Setup SID chip select
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  // setup data lines
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  // setup address lines
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);

  delay(20);
  resetSID();
  delay(20);
}

void loop()
{
  // si données pas prêtes, continuer à lire
  if (!dataready)
    readData();

  // si données prêtes écrire dans le SID
  if (dataready == true) {
    dataready = false;
    updateSID();
  }
}
