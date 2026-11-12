// 定義接腳 (根據右側電路圖)
const int BUTTON_PIN = 2;
const int RED_PIN = 10;
const int GREEN_PIN = 11;
const int BLUE_PIN = 9;

// 模式列舉：常亮模式、閃爍模式、呼吸燈模式
enum Mode { CONSTANT, BLINK, BREATH };
Mode currentMode = CONSTANT;

// 顏色列舉：紫、紅、藍、綠、黃、粉
enum Color { PURPLE, RED, BLUE, GREEN, YELLOW, PINK };
Color currentColor = PURPLE;

// 按鈕長短按檢測變數
unsigned long pressStartTime = 0;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 長短按判定門檻 (毫秒)
const unsigned long LONG_PRESS_TIME = 600; 

// 呼吸燈與閃爍用的計時變數
unsigned long previousMillis = 0;
int breathVal = 0;
int breathDir = 1;
bool blinkState = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 內建上拉，配合模組訊號
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  setColor(PURPLE);
}

void loop() {
  handleButton();
  updateLightEffect();
}

// 處理按鈕長按切換模式、短按切換顏色
void handleButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // 按下按鈕瞬間
      if (currentButtonState == LOW) {
        pressStartTime = millis();
      } 
      // 放開按鈕瞬間
      else {
        unsigned long pressDuration = millis() - pressStartTime;
        
        if (pressDuration >= LONG_PRESS_TIME) {
          // 【長按】切換模式：常亮 -> 閃爍 -> 呼吸 -> 常亮
          if (currentMode == CONSTANT) currentMode = BLINK;
          else if (currentMode == BLINK) currentMode = BREATH;
          else if (currentMode == BREATH) currentMode = CONSTANT;
        } else {
          // 【短按】切換顏色：紫 -> 紅 -> 藍 -> 綠 -> 黃 -> 粉 -> 循環
          if (currentColor == PURPLE) currentColor = RED;
          else if (currentColor == RED) currentColor = BLUE;
          else if (currentColor == BLUE) currentColor = GREEN;
          else if (currentColor == GREEN) currentColor = YELLOW;
          else if (currentColor == YELLOW) currentColor = PINK;
          else if (currentColor == PINK) currentColor = PURPLE;
        }
      }
    }
  }
  lastButtonState = reading;
}

// 根據當前模式與顏色更新燈光狀態
void updateLightEffect() {
  unsigned long currentMillis = millis();

  switch (currentMode) {
    case CONSTANT:
      setColor(currentColor);
      break;

    case BLINK:
      // 每 300 毫秒切換一次閃爍
      if (currentMillis - previousMillis >= 300) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
      }
      if (blinkState) {
        setColor(currentColor);
      } else {
        setRGB(255, 255, 255); // 共陽極 255 代表全滅
      }
      break;

    case BREATH:
      // 呼吸燈漸變速度
      if (currentMillis - previousMillis >= 15) {
        previousMillis = currentMillis;
        breathVal += breathDir * 3;
        if (breathVal >= 255 || breathVal <= 0) {
          breathDir = -breathDir;
          breathVal = constrain(breathVal, 0, 255);
        }
      }
      setBreathColor(currentColor, breathVal);
      break;
  }
}

// 設定特定顏色的基本輸出 (針對共陽極：0最亮，255全滅)
void setColor(Color col) {
  switch (col) {
    case PURPLE: setRGB(120, 255, 30);  break; // 紅+藍 = 紫
    case RED:    setRGB(0, 255, 255);   break; // 純紅
    case BLUE:   setRGB(255, 255, 0);   break; // 純藍
    case GREEN:  setRGB(255, 0, 255);   break; // 純綠
    case YELLOW: setRGB(0, 50, 255);    break; // 紅+綠 = 黃
    case PINK:   setRGB(0, 150, 100);   break; // 亮粉紅
  }
}

// 支援呼吸燈明暗變化的顏色輸出
void setBreathColor(Color col, int intensity) {
  // intensity 從 0 (全亮) 到 255 (全暗) 進行縮放
  int r = 255, g = 255, b = 255;
  switch (col) {
    case PURPLE: r = map(intensity, 0, 255, 120, 255); g = 255; b = map(intensity, 0, 255, 30, 255); break;
    case RED:    r = map(intensity, 0, 255, 0, 255);   g = 255; b = 255; break;
    case BLUE:   r = 255; g = 255; b = map(intensity, 0, 255, 0, 255); break;
    case GREEN:  r = 255; g = map(intensity, 0, 255, 0, 255);   b = 255; break;
    case YELLOW: r = map(intensity, 0, 255, 0, 255);   g = map(intensity, 0, 255, 50, 255); b = 255; break;
    case PINK:   r = map(intensity, 0, 255, 0, 255);   g = map(intensity, 0, 255, 150, 255); b = map(intensity, 0, 255, 100, 255); break;
  }
  setRGB(r, g, b);
}

// 底層硬體 PWM 輸出控制
void setRGB(int rRed, int gGreen, int bBlue) {
  analogWrite(RED_PIN, rRed);
  analogWrite(GREEN_PIN, gGreen);
  analogWrite(BLUE_PIN, bBlue);
}