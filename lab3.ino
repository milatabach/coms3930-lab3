/**************************************************************************
HTTP Request Example
Fetch a random color palette from colormind.io and draw the colors on the display

To fetch a new color, press either button on the LILYGO (GPIO 0 or 35)     
**************************************************************************/
#include "TFT_eSPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// TODO: replace with your own SSID & Password
const char* ssid = "Barnard Guest";
const char* password = "";

#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

volatile bool leftButtonPressed = false;
volatile bool rightButtonPressed = false;

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  
  // setup our display
  tft.init();
  tft.setRotation(1);  // 1 = landscape, 2 = portrait
  tft.fillScreen(TFT_BLACK);

  Serial.print("display dimensions: ");
  Serial.print(tft.width());
  Serial.print(" x ");
  Serial.println(tft.height());

  // connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Send the HTTP POST request
  if (WiFi.status() == WL_CONNECTED) {
    insult();
  } else {
    Serial.println("WiFi not connected");
  }

  // setup our buttons
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT), pressedLeftButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), pressedRightButton, FALLING);
}


void insult() {
    HTTPClient http;

    String url = "https://evilinsult.com/generate_insult.php?lang=en&type=json";
    http.begin(url);

    // Set the content type to JSON
    http.addHeader("Content-Type", "application/json");

    // JSON data to send in the request
    String jsonData = "{\"model\":\"default\"}";

    // Send the request
    int httpResponseCode = http.POST(jsonData);

    // Check the response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
      JSONVar responseJSON = JSON.parse(response);

      if (JSON.typeof(responseJSON) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      String insultText = (const char*)responseJSON["insult"];
      Serial.println("Insult: " + insultText);

      // Display the insult on the screen
        tft.fillScreen(0x5043);   // maroon
        tft.setTextColor(TFT_WHITE); 
        tft.setTextSize(1);  
        
        int maxWidth = 100;  // Adjust based on your screen's width
        int cursorX = 10;
        int cursorY = 10;
        String line = "";
        
        for (int i = 0; i < insultText.length(); i++) {
            line += insultText[i];
            int lineWidth = tft.textWidth(line, 2);
            if (lineWidth > maxWidth || insultText[i] == '\n') {
                tft.drawString(line, cursorX, cursorY, 2);
                cursorY += 20;  // Move to the next line
                line = "";
            }
        }

        if (line.length() > 0) {
            tft.drawString(line, cursorX, cursorY, 2);
        }



        delay(300);
        drawRandomHearts();

    } else {
      Serial.println("Error on sending POST request");
    }

    // Free resources
    http.end();
}

void pressedLeftButton() {
  leftButtonPressed = true;
}

void pressedRightButton() {
  rightButtonPressed = true;
}

void loop() {
  // fetch colors when either button is pressed
  if (leftButtonPressed) {
    insult();
    leftButtonPressed = false;
  }
  if (rightButtonPressed) {
    insult();
    rightButtonPressed = false;
  }
}

void drawRandomHearts() {
    int numHearts = 4; // Number of hearts drawn
    uint16_t colors[] = {0xb886, 0x9865, 0xe0a8, 0xb886, 0xf6da, 0x70c4};

    for (int i = 0; i < numHearts; ++i) {  
        int randomX = rand() % (tft.width());
        int randomY = rand() % (tft.height()); // The hearts should only appear in the top half to not overlap with the characters
          int randomColorIndex = rand() % (sizeof(colors) / sizeof(colors[0]));

        drawHeart(randomX, randomY, colors[randomColorIndex]);
        delay(500);
    }
}

void drawHeart(int x, int y, uint16_t color) {
  tft.fillCircle(x, y, 4, color);        
  tft.fillCircle(x + 8, y, 4, color);    
  tft.fillTriangle(x - 4, y, x + 12, y, x + 4, y + 10, color);  
}

