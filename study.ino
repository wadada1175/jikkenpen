#include <M5StickCPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>

// LINE Notifyの設定
const char* LINE_NOTIFY_TOKEN = "MyTOKEN";

// Wi-Fiの設定
const char* WIFI_SSID = "myssid";
const char* WIFI_PASSWORD = "mypassword";

//加速度センサーの設定
float accX, accY, accZ;
const float MAX_TILT = 0.3;
bool isResting = false;
bool isSoundPlaying = false;

//時間計測の設定
unsigned long time_m = 0; //ボタンが押された時刻
unsigned long time_n = 0; //ボタンが再び押された時刻
unsigned long time_nagasa = 0; //再びボタンを押すまでの時間

void setup() {
    M5.begin();
    M5.IMU.Init();
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);

    connectToWiFi();
}

void loop() {
    M5.update();

    M5.IMU.getAccelData(&accX, &accY, &accZ);

    if (!isResting) {
        M5.Lcd.fillScreen(TFT_BLACK);
        if (accY <= MAX_TILT) {//accYが0.3以下の場合
            if (!isSoundPlaying) {
                playSound();
                isSoundPlaying = true;
            }
            displayCenteredMessage("Just Do It");
            Serial.println("Not worked!");//シリアル送信
        } else {
            if (isSoundPlaying) {
                stopSound();
                isSoundPlaying = false;
            }
            displayCenteredMessage("Working");
            Serial.println("Working!");
        }
    } else {
        int screenWidth = M5.Lcd.width();
        int screenHeight = M5.Lcd.height();
        int textWidth = strlen("Resting") * 12;
        int textHeight = 16;
        int textX = (screenWidth - textWidth) / 2;
        int textY = (screenHeight - textHeight) / 2;

        M5.Lcd.fillScreen(TFT_GREEN);
        M5.Lcd.setCursor(textX, textY);
        M5.Lcd.print("Resting");
        //M5.Lcd.fillScreen(TFT_GREEN);
        //displayCenteredMessage("Resting");
        Serial.println("Resting");
    }

    checkButtonPress();

    delay(1000);
}

void displayCenteredMessage(const char* message) {
    int screenWidth = M5.Lcd.width();
    int screenHeight = M5.Lcd.height();
    int textWidth = strlen(message) * 12;
    int textHeight = 16;
    int textX = (screenWidth - textWidth) / 2;
    int textY = (screenHeight - textHeight) / 2;

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(textX, textY);
    M5.Lcd.print(message);
}

void playSound() {
    M5.Beep.tone(1000);
}

void stopSound() {
    M5.Beep.mute();
}

void checkButtonPress() {
    if (M5.BtnA.wasReleased()) {
        if (isResting) {
            isResting = false;
            time_n = millis(); //ボタンが離された時刻を記録
            time_nagasa = time_n - time_m; //ボタンが押されている時間を計算
            sendLineNotifyMessage(("休憩を終了しました。休憩時間は" + String(time_nagasa / 60000) + "分でした").c_str());
            Serial.println("Start Working");
        } else {
            stopSound();
            isResting = true;
            time_m = millis(); //ボタンが押された時刻を記録
            sendLineNotifyMessage("休憩を開始しました");
            Serial.println("Start Resting");
        }
        delay(200);
    }
}

void connectToWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        displayCenteredMessage("Connecting to WiFi");
    }
    int screenWidth = M5.Lcd.width();
    int screenHeight = M5.Lcd.height();
    int textWidth = strlen("Connected to WiFi") * 12;
    int textHeight = 16;
    int textX = (screenWidth - textWidth) / 2;
    int textY = (screenHeight - textHeight) / 2;
    M5.Lcd.fillScreen(TFT_GREEN);
    M5.Lcd.setCursor(textX, textY);
    M5.Lcd.print("Connected to WiFi");
    Serial.println("Connected to WiFi");
    //displayCenteredMessage("Connected to WiFi");
    M5.Beep.tone(3000);
    delay(1000);
    M5.Beep.mute();
}

void sendLineNotifyMessage(const char* message) {
    HTTPClient http;
    http.begin("https://notify-api.line.me/api/notify");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", String("Bearer ") + LINE_NOTIFY_TOKEN);
    String payload = "message=" + String(message);
    http.POST(payload);
    http.end();
}







