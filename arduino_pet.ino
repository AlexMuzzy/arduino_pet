#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <TimeLib.h>
#include <avr/eeprom.h>

#define OFF 0 0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

int development;
int happiness;
int fullness;
int total;
int tcounter;
int devage;
int ageSeconds;
int ageMinutes;
int mode;
int gameActionSelected;
int menuActionSelected;
int onloadActionSelected;
bool buttonPressed;
time_t pauseTime;

byte dev0[] = {
  B10000, B01111, B01100, B11001, B11000, B01000, B01100, B00111
};
byte dev1[] = {
  B00000, B11111, B00000, B00000, B10001, B10001, B01110, B11111
};
byte dev2[] = {
  B00001, B11110, B00110, B10011, B00011, B00010, B00110, B11100
};
byte dev3[] = {
  B00000, B00100, B01010, B11001, B10011, B11001, B10011, B01110
};
byte dev4[] = {
  B10111, B01000, B01010, B10000, B10010, B01001, B01100, B00011
};
byte dev5[] = {
  B11101, B00010, B01010, B00001, B01001, B10010, B00110, B11000
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.noBlink();
  lcd.setBacklight(WHITE);
  assignGlobals(true);
}

void assignGlobals(bool load) {
  lcd.clear();
  lcd.createChar(0, dev0);
  lcd.createChar(1, dev1);
  lcd.createChar(2, dev2);
  lcd.createChar(3, dev3);
  lcd.createChar(4, dev4);
  lcd.createChar(5, dev5);

  development = 0;
  fullness = 3;
  happiness = 2;
  setTime(0, 0, 0, 0, 0, 0);
  pauseTime = now();
  total = 0;
  tcounter = 0;
  devage = 0;
  buttonPressed = false;
  if (load) {
    byte modecheck = eeprom_read_byte((uint8_t*)0);
    switch (modecheck) {
      case 0:
        mode = 0;
        break;
      case 1:
        mode = 2;
        break;
    }
  }
  else {
    mode = 0;
  }
  gameActionSelected = 0;
  menuActionSelected = 0;
  onloadActionSelected = 0;
  ageMinutes = 0;
  ageSeconds = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (mode) {
    case 0:
      runGameDisplay();
      break;
    case 1:
      runMenuDisplay();
      break;
    case 2:
      runOnloadSetup();
      break;
    case 3:
      deathPet();
      break;
  }
}
// actions to run on each state change
void onStateChange(int num) {
  if (num == 0) {
    pauseGame(true);
  }
  else {
    pauseGame(false);
  }
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  mode = num;
}

//Start of onloadsetup
void runOnloadSetup() {
  displayOnload();
  runOnloadInterface();
}

void displayOnload() {
  lcd.setCursor(0, 0);
  lcd.print("LOAD SAVE?");
  lcd.setCursor(0, 1);
  lcd.print(">");
}

void runOnloadInterface() {
  displayOnloadActions();
  runOnloadButtons();
}

void displayOnloadActions() {
  lcd.setCursor(1, 1);
  switch (onloadActionSelected) {
    case 0:
      lcd.print("LOAD SAVE");
      break;
    case 1:
      lcd.print("START NEW");
      break;
  }
}

void runOnloadButtons() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    if (!buttonPressed) {
      if (buttons & BUTTON_LEFT) {
        if (onloadActionSelected == 0) {
          onloadActionSelected = 1;
        }
        else {
          onloadActionSelected--;
        }
      }
      else if (buttons & BUTTON_RIGHT) {
        if (onloadActionSelected == 1) {
          onloadActionSelected = 0;
        }
        else {
          onloadActionSelected++;
        }
      }
      else if (buttons & BUTTON_SELECT) {
        switch (onloadActionSelected) {
          case 0:
            loadPet();
            break;
          case 1:
            assignGlobals(false);
        }
      }
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }
}
/*end of onload state

   start of game
*/
void runGame() {
  ageMinutes = minute();
  ageSeconds = second();
  if (second() != tcounter) {
    tcounter = second();
    ++total;
    if (total > 5) {
      ++devage;
    }
    runStats();
  }
}

void runGameDisplay() {
  displayGame();
  runGameInterface();
  runGame();
}

void displayGame() {
  lcd.setBacklight(WHITE);
  lcd.setCursor(0, 0); lcd.print("D:"); lcd.print(development);
  lcd.setCursor(3, 0); lcd.print("H:"); lcd.print(happiness);
  lcd.setCursor(6, 0); lcd.print("F:"); lcd.print(fullness);
  lcd.setCursor(11, 0); char str_time[16]; sprintf(str_time, "%02u:%02u", ageMinutes, ageSeconds); lcd.print(str_time);
  lcd.setCursor(0, 1); lcd.print(">");
  generateCharacter();
}
void runStats() {
  //runHappiness
  if (devage >= 1 && happiness != 0) {
    if (devage % 7 == 0) {
      happiness--;
    }
  }
  //runDevelopment
  if (total == 5) {
    development = 1;
  }
  if (total >= 35 && happiness >= 1 && fullness >= 3) {
    development = 2;
  }
  //runFullness
  if (devage >= 1 && fullness != 0) {
    if (devage % 11 == 0) {
      fullness--;
    }
  }
  //deathTimer
  if (total >= 600) {
    onStateChange(3);
  }
}

void runGameInterface() {
  displayGameActions();
  runGameButtons();
}

void runGameButtons() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    if (!buttonPressed) {
      if (buttons & BUTTON_LEFT) {
        if (gameActionSelected == 0) {
          gameActionSelected = 2;
        }
        else {
          gameActionSelected--;
        }
      }
      else if (buttons & BUTTON_RIGHT) {
        if (gameActionSelected == 2) {
          gameActionSelected = 0;
        }
        else {
          gameActionSelected++;
        }
      }
      else if (buttons & BUTTON_SELECT) {
        if (total >= 5) {
          runGameAction();
        }
        else if (gameActionSelected == 2) {
          runGameAction();
        }
      }
      buttonPressed = true;
    }
  }
  else {
    buttonPressed = false;
  }
}

void displayGameActions() {
  lcd.setCursor(1, 1);
  switch (gameActionSelected) {
    case 0:
      lcd.print("FEED  ");
      break;
    case 1:
      lcd.print("PLAY  ");
      break;
    case 2:
      lcd.print("MENU  ");
      break;
  }
}

void runGameAction() {
  switch (gameActionSelected) {
    case 0:
      if (fullness < 4) {
        fullness++;
        if (fullness == 4) {
          happiness = 0;
        }
      }
      break;
    case 1:
      if (happiness < 2 && fullness >= 2) {
        happiness++;
      }
      break;
    case 2:
      onStateChange(1);
      break;
  }
}
void generateCharacter() {
  if (development == 0) {
    lcd.setCursor(13, 1);
    lcd.write(3);
  }
  if (development == 1) {
    lcd.setCursor(12, 1);
    lcd.write(4);
    lcd.write(5);
  }
  if (development == 2) {
    lcd.setCursor(11, 1);
    lcd.write(0);
    lcd.write(1);
    lcd.write(2);
  }
}

void deathPet() {
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("YOUR PET IS DEAD.");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("RESTARTING...   ");
  delay(1000);
  setup();
}

void pauseGame (bool resumetime) {
  if (!resumetime) {
    setTime(0, ageMinutes, ageSeconds, 0, 0, 0);
    pauseTime = now();
  }
  else if (resumetime) {
    setTime(pauseTime);
  }
}
/*
    End of game

    Start of Menu
*/
void runMenuDisplay() {
  displayMenu();
  runMenuInterface();
}

void runMenuButtons() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    if (!buttonPressed) {
      if (buttons & BUTTON_LEFT) {
        if (menuActionSelected == 0) {
          menuActionSelected = 4;
        }
        else {
          menuActionSelected--;
        }
      }
      else if (buttons & BUTTON_RIGHT) {
        if (menuActionSelected == 4) {
          menuActionSelected = 0;
        }
        else {
          menuActionSelected++;
        }
      }
      else if (buttons & BUTTON_SELECT) {
        if (menuActionSelected == 4) {
          byte modecheck = eeprom_read_byte((uint8_t*)0);
          switch (modecheck) {
            case 0:
              break;
            case 1:
              runMenuAction();
              break;
          }
        } else {
          runMenuAction();
        }
      }
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }
}

void runMenuInterface() {
  runMenuButtons();
  displayMenuActions();
}

void displayMenu() {
  lcd.setBacklight(TEAL);
  lcd.setCursor(0, 0); lcd.print("PET MENU");
  lcd.setCursor(0, 1); lcd.print(">");
  displayMenuActions();
}

void displayMenuActions() {
  lcd.setCursor(1, 1);
  switch (menuActionSelected) {
    case 0:
      lcd.print("SAVE   ");
      break;
    case 1:
      lcd.print("RESET  ");
      break;
    case 2:
      lcd.print("DELETE ");
      break;
    case 3:
      lcd.print("MENU   ");
      break;
    case 4:
      byte modecheck = eeprom_read_byte((uint8_t*)0);
      switch (modecheck) {
        case 0:
          lcd.print("NO SAVE");
          break;
        case 1:
          lcd.print("LOAD   ");
          break;
      }
  }
}

void savePet() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SAVING PET..");
  eeprom_write_byte((uint8_t*)0, 1);
  eeprom_write_byte((uint8_t*)1, development);
  eeprom_write_byte((uint8_t*)2, happiness);
  eeprom_write_byte((uint8_t*)3, fullness);
  eeprom_write_byte((uint8_t*)4, total);
  eeprom_write_dword((uint32_t*)5, ageSeconds);
  eeprom_write_dword((uint32_t*)6, ageMinutes);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PET SAVED");
  delay(1000);
  onStateChange(0);
}

void deletePet() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DELETING PET...");
  eeprom_write_byte((uint8_t*)0, 0);
  lcd.clear();
  lcd.print("PET DELETED");
  delay(1000);
  onStateChange(0);
}

void loadPet() {
  development = eeprom_read_byte((uint8_t*)1);
  happiness = eeprom_read_byte((uint8_t*)2);
  fullness = eeprom_read_byte((uint8_t*)3);
  total = eeprom_read_byte((uint8_t*)4);
  ageSeconds = eeprom_read_dword((uint32_t*)5);
  ageMinutes = eeprom_read_dword((uint32_t*)6);
  setTime (0 , ageMinutes, ageSeconds, 0, 0, 0);
  pauseTime = now();
  if (total >= 5) {
    devage = total - 5;
  }
  else {
    devage = 0;
  }
  onStateChange(0);
}

void runMenuAction() {
  switch (menuActionSelected) {
    case 0:
      savePet();
      break;
    case 1:
      assignGlobals(false);
      break;
    case 2:
      deletePet();
    case 3:
      onStateChange(0);
      break;
    case 4:
      loadPet();
      break;
  }
}
