#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= PINOS =================
#define STEP_PIN    3
#define DIR_PIN     2

#define MS1_PIN     7   // M0
#define MS2_PIN     6   // M1
#define MS3_PIN     5   // M2

#define ENC_S1      8
#define ENC_S2      9
#define BOTAO_CORTE 11

// ================= MOTOR =================
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// ================= CONTROLE =================
int lastS1;
int medida_cm = 5;
bool emCorte = false;

// ================= CALIBRAÇÃO =================
#define STEPS_POR_CM  20.43

#define VEL_MAXIMA    250      // steps/seg
#define ACELERACAO    100      // steps/seg²

// =======================================================

void setup() {

  // -------- Encoder e botão --------
  pinMode(ENC_S1, INPUT_PULLUP);
  pinMode(ENC_S2, INPUT_PULLUP);
  pinMode(BOTAO_CORTE, INPUT_PULLUP);

  lastS1 = digitalRead(ENC_S1);

  // -------- DRV8825 em HALF STEP (1/2) --------
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);

  digitalWrite(MS1_PIN, HIGH);
  digitalWrite(MS2_PIN, LOW);
  digitalWrite(MS3_PIN, LOW);

  // -------- LCD --------
  lcd.init();
  lcd.backlight();

  // -------- Motor --------
  stepper.setMaxSpeed(VEL_MAXIMA);
  stepper.setAcceleration(ACELERACAO);
  stepper.setCurrentPosition(0);

  atualizarDisplay();
}

// =======================================================

void loop() {

  // Ajuste da medida via encoder
  if (!emCorte) {
    int s1 = digitalRead(ENC_S1);
    if (lastS1 == HIGH && s1 == LOW) {
      if (digitalRead(ENC_S2)) medida_cm += 5;
      else if (medida_cm > 5) medida_cm -= 5;
      atualizarDisplay();
    }
    lastS1 = s1;
  }

  // Botão de corte
  if (!emCorte && digitalRead(BOTAO_CORTE) == LOW) {
    executarCorte();
    delay(250); // debounce simples
  }
}

// =======================================================

void executarCorte() {

  emCorte = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tracionando...");

  // -------- Calibração simples --------
  long passosAlvo = (long)(medida_cm * STEPS_POR_CM);

  stepper.setCurrentPosition(0);
  stepper.moveTo(passosAlvo);

  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  atualizarDisplay();
  emCorte = false;
}

// =======================================================

void atualizarDisplay() {

  lcd.setCursor(0, 0);
  lcd.print("Ajuste de medida");

  lcd.setCursor(0, 1);
  lcd.print("                ");

  String t;
  if (medida_cm >= 100) {
    float metros = medida_cm / 100.0;
    t = String(metros, 2) + " mt";
  } else {
    t = String(medida_cm) + " cm";
  }
  lcd.setCursor((16 - t.length()) / 2, 1);
  lcd.print(t);
}
