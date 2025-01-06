#include <M5StickCPlus2.h>
#include <M5Unified.h> 
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ESPmDNS.h> 
#include "RTClib.h"

#define SCREEN_WIDTH M5.Lcd.width()
#define SCREEN_HEIGHT M5.Lcd.height() 
#define CENTER_X (SCREEN_WIDTH / 2)
#define CENTER_Y (SCREEN_HEIGHT / 2)
float cubeX = 0, cubeY = 0; // Initial position of the cube
float cubeSize = 11; // Size of the cube
float speedX = 0, speedY = 0; // Speed of the cube
const int numCubes = 22; // Number of cubes to display
float cubePositions[numCubes][2]; // Array to hold cube positions
float trianglePositions[15][2]; // Array to hold triangle positions
const int numTriangles = 15; // Number of triangles to display
const float triangleSize = 23; // Size of the triangle
float triangleAngles[15]; // Array to hold the angles for rotation

// network credentials
const char* ssid = "neoncat";
WebServer server(80);
int currentHour = 12; // Default hour
int currentMinute = 0; // Default minute

// Define colors
#define ORANGE 0xFCA0 // Example RGB565 for orange
// #define ORANGE M5.Lcd.color565(255, 165, 0) // Define orange color
// This line is ugly in respect to size of our sketch. 
#define GRAY   0x7BEF // Example RGB565 for gray
#define WHITE  0xFFFF // RGB565 for white

// menu 
const char* menuItems[] = {
"Shake A Day", "WiFi finder", "Scan WiFi", 
"Screensaver", "Menu Color", "Fractal", 
"Essid Spam","24h Time", "12h time", 
"Lines","Qubes","Qubes II"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);
int selectedItem = 0;
int scrollOffset = 0;
const int displayHeight = 160; // Height of M5StickC display
const int itemHeight = 30;  // Height of each menu item
int loopCounter = 0; // Counter for the loop
// Animation variables
int glowDirection = 1; // 1 for increasing, -1 for decreasing
int glowIntensity = 0; // Current glow intensity
long int j = 0; 
// Color variables for fading effect
uint8_t r = 0, g = 0, b = 0;
uint8_t targetR = 0, targetG = 0, targetB = 0;
// DO random message. 
const char* messages[] = {
    "Welcome", "Hello", "All good",  
    "Greetings", "Salutations", "READY", 
    "Fair-tidings", "Cheers", "Hey",
    "Hiya", "Bonjour", "Hola", "Shalom",
    "Friend", "Ola", "Booting", "Thinking",
    "Bonjourno", "Hark", "Wassup", "Konnichiwa",
    "Howdy", "Namaste",  "Cool", "Starting", 
    "Ciao", "Salute", "WELCOME", "Aloha", "Lovin' it"
};

// Fractal vars 
float zoom = 1.0; // Zoom level
float moveX = 0.0; // Horizontal movement
float moveY = 0.0; // Vertical movement

const int totalEssids = 9889; // Total number of random ESSIDs to generate
const int essidLength = random(10, 32); // Length of the random ESSID
const int spamDuration = 10000; // Duration to spam in milliseconds
const int spamInterval = 100; // Interval between spams in milliseconds
const int totalSpams = spamDuration / spamInterval;

void setup() {

  auto cfg = M5.config();
  cfg.output_power = true; 
  M5.begin(cfg);
  StickCP2.begin(); 
    if (!StickCP2.Rtc.isEnabled()) {
        StickCP2.Display.fillScreen(BLACK); 
        StickCP2.Display.setCursor(1,1); 
        StickCP2.Display.println("RTC not found.");
        for (;;) {
            vTaskDelay(500);
        }
    }
        // Initialize the IMU
    if (!StickCP2.Imu.begin()) {
        M5.Lcd.println("IMU initialization failed!");
        while (1);
    }
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(3); 
  M5.Lcd.setTextColor(BLACK); 
  M5.Lcd.fillScreen(BLACK); // Clear screen
  randomSeed(analogRead(0)); 
  intro(); 
  M5.Lcd.fillScreen(BLACK); 
  // Start WiFi
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setTextColor(ORANGE); 
  M5.Lcd.setCursor(1,1); 
  M5.Lcd.println("Starting Access point "); 
  M5.Lcd.printf("%s", ssid); 
  WiFi.softAPdisconnect(true); 
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid,nullptr,1,false,WIFI_AUTH_WPA2_PSK);
  M5.Lcd.println("Access Point Started");
  M5.Lcd.print("IP Address: ");
  M5.Lcd.println(WiFi.softAPIP());
  delay(2199); 
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setCursor(0,0); 
  M5.Lcd.println("Sever setup."); 
  // Define the root URL handler 
  server.on("/", HTTP_GET, handleRoot);
  M5.Lcd.println("/ Root active"); 
  server.on("/setTime", HTTP_POST, handleSetTime);
  server.on("/settime", HTTP_POST, handleSetTime);
  M5.Lcd.println("/setTime page active"); 
  server.onNotFound(handleNotFound);
  // Start the server
  server.begin();
  delay(2100); 
  M5.Lcd.fillScreen(BLACK); 
    // Set initial target color
  targetR = 255;
  targetG = 0;
  targetB = 0;
}

// visual fidget spiner break's effect
void breaks() {
    
    M5.update();
    if (M5.BtnB.pressedFor(2000)) { M5.Lcd.setTextColor(random(0xFFF00)); }   
    if (M5.BtnA.pressedFor(2100)) { M5.Lcd.setTextColor(random(0xFFF00)); }
    M5.update(); 
    if (M5.BtnA.wasPressed()) { M5.Lcd.setTextColor(random(0xFFF00)); } 
    if (M5.BtnB.wasPressed()) { M5.Lcd.setTextColor(random(0xFFF00)); }
    
}

void intro() {
  // Update glow intensity
  glowIntensity += glowDirection * 25; // Change the increment for speed
  if (glowIntensity >= 255) {
    glowIntensity = 255;
    glowDirection = -1; // Change direction
  } else if (glowIntensity <= 0) {
    glowIntensity = 0;
    glowDirection = 1; // Change direction
  }
    
     // Get a random index
    int randomIndex = random(0, sizeof(messages) / sizeof(messages[0]));
   M5.Lcd.fillScreen(BLACK);
   // Not: it's not a true random event. Okay so you can display up to 5 lines but 
   // it will repeat words or skip slections. Sudorandom by default! 
  // Draw the glowing text
  randomSeed(millis()+analogRead(0));
  drawGlowingText(messages[randomIndex], 1, 1);
  randomSeed(analogRead(2)); 
  drawGlowingText(messages[random(1, sizeof(messages) / sizeof(messages[0]))], random(1, 17), 22); 
  //randomSeed(millis());
 // drawGlowingText(messages[random(2, sizeof(messages) / sizeof(messages[0]))], random(1,19), 44);
 // drawGlowingText(messages[randomIndex], random(1, 18), 66);
 // drawGlowingText(messages[randomIndex], random(1, 18), 99);
  // Delay for animation speed
  delay(50);
}

void drawGlowingText(const char* text, int x, int y) {
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i != 0 || j != 0) {
        M5.Lcd.setTextColor(GRAY);
        M5.Lcd.setCursor(x + i, y + j);
        M5.Lcd.print(text);
      }
    }
  }
  for (int i = 0; i<200; i++) {
    breaks();
   uint16_t glowColor_2 = M5.Lcd.color565(173 / 8, 216 / 4, 230 / 8);
  // Draw the blue text
  M5.Lcd.setTextColor(glowColor_2);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(5);
  }
    for (int i = 0; i<440; i++) {
      breaks();
          // Increment the loop counter
    loopCounter++;
      // Check if the loop counter is a multiple of 30
    if (loopCounter % 40 == 0) {
  M5.Lcd.setTextColor(random(0xFFFFF)); 
  }
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(3);
  }
   
  for (int i = 0; i<200; i++) {
    breaks();
  M5.Lcd.setTextColor(GRAY); 
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(7);
  }
  for (int i = 0; i<200; i++) {
    breaks();
  uint16_t glowColor_4 = M5.Lcd.color565(173 ^ 8 % 22, random(216) ^ 4 % 12, 230 ^ 100 % 8);
  // Weird glowing text effect. 
  M5.Lcd.setTextColor(glowColor_4);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(6); 
  }
  for (int i = 0; i<200; i++) {
    breaks();
  M5.Lcd.setTextColor(BLACK); 
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(7);
  }
  // two loops trigger events 
  j++; 
  if (j >= 2) { 
    j = 0; 
       for (int i = 0; i<240; i++) {
          // Increment the loop counter
    loopCounter++;
      // Check if the loop counter is a multiple of 30
    if (loopCounter % 30 == 0) {
  M5.Lcd.setTextColor(random(0xFFFFF)); 
  }
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
  delay(10);
  }
    }
}


void drawMenuItem(const char* text, int x, int y, bool isSelected) {
  // Calculate the fade effect based on the Y position
  uint16_t textColor = BLACK; // Default text color
  if (y < 30) { // Top fade
    int fadeAmount = map(y, 0, 30, 0, 255); // Fade from 0 to 255
    textColor = M5.Lcd.color565(255 - fadeAmount, 255 - fadeAmount, 255 - fadeAmount); // Fading to lighter color
  } else if (y > 130) { // Bottom fade
    int fadeAmount = map(y, 130, 160, 0, 255); // Fade from 0 to 255
    textColor = M5.Lcd.color565(255 - fadeAmount, 255 - fadeAmount, 255 - fadeAmount); // Fading to lighter color
  }

  // Draw glowing effect for the selected item
  if (isSelected) {
    M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(x - 2, y - 2);
    M5.Lcd.print(text);
    M5.Lcd.setCursor(x + 2, y - 2);
    M5.Lcd.print(text);
    M5.Lcd.setCursor(x - 2, y + 2);
    M5.Lcd.print(text);
    M5.Lcd.setCursor(x + 2, y + 2);
    M5.Lcd.print(text);
  }

  // Draw the main text
  M5.Lcd.setTextColor(textColor);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
}
void handleNotFound() {
  String message = "NEONCAT Soft AP mode server.\n\nFile Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void loop() {
//  Our server runs in the background 
server.handleClient();
 M5.update();
  // start trap counter 
  j++;
  // Update background color
  r += (targetR - r) / 20; // Smooth transition towards target color
  g += (targetG - g) / 20;
  b += (targetB - b) / 20;
  server.handleClient();
  // Change target color when reaching the current target
  if (abs(r - targetR) < 2 && abs(g - targetG) < 2 && abs(b - targetB) < 2) {
    // Randomly select a new target color
    targetR = random(0,256);
    targetG = random(0,256);
    targetB = random(0,256);
  } else if (j >=130) {
    targetR = random(100,256);
    targetG = random(100,256);
    targetB = random(100,256);
    j = 0; 
  }
  server.handleClient();
  // Fill the screen with the current background color
  M5.Lcd.fillScreen(M5.Lcd.color565(r, g, b));
  server.handleClient();
  // Draw the menu items
  for (int i = 0; i < menuCount; i++) {
    int y = 30 + (i * itemHeight) - (selectedItem * itemHeight); // Calculate Y position
    if (y >= 0 && y < displayHeight) { // Only draw if within screen bounds
      drawMenuItem(menuItems[i], 10, y, (i == selectedItem));
    }
  }

  // Check for button presses
  if (M5.BtnA.pressedFor(980)) {
    // Move up the menu
    selectedItem--;
    if (selectedItem < 0) {
      selectedItem = menuCount - 1; // Wrap around to the last item
    }
  }
  
  if (M5.BtnB.wasPressed()) {
    // Action for selecting the item
    handleMenuSelection(selectedItem); 
    // New background color value 
    targetR = random(0,256);
    targetG = random(0,256);
    targetB = random(0,256);
  }

  // Move down the menu
  if (M5.BtnA.wasPressed()) {
    selectedItem++;
    if (selectedItem >= menuCount) {
      selectedItem = 0; // Wrap around to the first item
    }
  }
  delay(100); // Adjust speed of scrolling
  server.handleClient();
}

void handleMenuSelection(int selectedItem) {
    M5.Lcd.fillScreen(BLACK);
    //M5.Lcd.setTextColor(WHITE);
    //M5.Lcd.setCursor(10, 10);
    //M5.Lcd.println("Selected: ");
    //M5.Lcd.println(menuItems[selectedItem]);
    //delay(2000); // Display selection for 2 seconds
    // Handle actions based on selected item
    switch (selectedItem) {
        case 0:
            // Action for Item 1
            M5.Lcd.fillScreen(BLACK); 
            shakeAday(); 
            break;
        case 1:
            // Action for Item 2
            M5.Lcd.fillScreen(BLACK); 
            scroll_wifi(); 
            break;
        case 2:
            // Action for Item 3
            M5.Lcd.fillScreen(BLACK); 
            networks();
            break;
        case 3:
            // Action for Item 4
            background_one(); 
            // New background color value 
            targetR = random(0,256);
            targetG = random(0,256);
            targetB = random(0,256);
            break;
        case 4:
            // Action for Item 5
              // Update background color
  r += (targetR - r) / 20; // Smooth transition towards target color
  g += (targetG - g) / 20;
  b += (targetB - b) / 20;

  // Change target color when reaching the current target
  if (abs(r - targetR) < 2 && abs(g - targetG) < 2 && abs(b - targetB) < 2) {
    // Randomly select a new target color
    targetR = random(0,256);
    targetG = random(0,256);
    targetB = random(0,256);
  } else if (j >=130) {
    targetR = random(100,256);
    targetG = random(100,256);
    targetB = random(100,256);
    j = 0; 
  }

  // Fill the screen with the current background color
  M5.Lcd.fillScreen(M5.Lcd.color565(r, g, b));

            targetR = random(0,256);
            targetG = random(0,256);
            targetB = random(0,256);
            break;
        case 5: 
            drawFractal();
            break; 
        case 6:
            essid_spam(); 
            break;
        case 7: 
            thetime(); 
            break; 
        case 8: 
            the12time();
            break;
        case 9:
            M5.Lcd.fillScreen(BLACK); 
            qubex(); 
            break; 
        case 10: 
            obj_hex();
            break; 
        case 11: 
            objtri(); 
            break;
        default:
            // Handle unexpected cases
            
            
            break;
    }
    delay(2000); // Display action for 2 seconds
}


// Shake A day game
const int DICE_SIZE = 44; // Size of each die (increased)
const int DICE_SPACING = 10; // Spacing between dice
const int DOT_SIZE = 4; // Size of each dot
bool allSame(int dice[]) {
    for (int i = 1; i < 6; i++) {
        if (dice[i] != dice[0]) {
            return false; // If any die is different, return false
        }
    }
    return true; // All dice are the same
}
bool allDifferent(int dice[]) {
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            if (dice[i] == dice[j]) {
                return false; // If any two dice are the same, return false
            }
        }
    }
    return true; // All dice are different
}
void shakeAday() {
    int dice[6];
    int score = 0;
while(true) {
  server.handleClient();
    // Roll the dice
    for (int i = 0; i < 6; i++) {
        dice[i] = random(1, 7); // Roll a die (1-6)
    }

    // Display the dice graphically
    displayDice(dice);

    // Check winning conditions
    if (allSame(dice)) {
        score += dice[0] * 50; // All dice are the same
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.println("You win: ");
        M5.Lcd.println(score);
        break; 
    } else if (allDifferent(dice)) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.println("You win:");
        M5.Lcd.println("Shake a Day!");
    }

    delay(4000); // Wait for 2 seconds before next roll
    M5.Lcd.fillScreen(BLACK); // Clear the screen for the next roll
}
}
void drawDots(int x, int y, int value) {
    int offsetX = x + DICE_SIZE / 2; // Center X
    int offsetY = y + DICE_SIZE / 2; // Center Y

    // Draw dots based on the value
    switch (value) {
        case 1:
            M5.Lcd.fillCircle(offsetX, offsetY, DOT_SIZE, BLACK); // Center dot
            break;
        case 2:
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-left dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-right dot
            break;
        case 3:
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-left dot
            M5.Lcd.fillCircle(offsetX, offsetY, DOT_SIZE, BLACK); // Center dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-right dot
            break;
        case 4:
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-left dot
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-left dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-right dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-right dot
            break;
        case 5:
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-left dot
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-left dot
            M5.Lcd.fillCircle(offsetX, offsetY, DOT_SIZE, BLACK); // Center dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-right dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-right dot
            break;
        case 6:
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-left dot
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY, DOT_SIZE, BLACK); // Middle-left dot
            M5.Lcd.fillCircle(offsetX - DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-left dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY - DICE_SIZE / 4, DOT_SIZE, BLACK); // Top-right dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY, DOT_SIZE, BLACK); // Middle-right dot
            M5.Lcd.fillCircle(offsetX + DICE_SIZE / 4, offsetY + DICE_SIZE / 4, DOT_SIZE, BLACK); // Bottom-right dot
            break;
    }
}
void drawDie(int x, int y, int value) {
    // Draw the die as a square
    M5.Lcd.fillRect(x, y, DICE_SIZE, DICE_SIZE, WHITE);
    M5.Lcd.drawRect(x, y, DICE_SIZE, DICE_SIZE, BLACK); // Draw border

    // Draw dots based on the value
    drawDots(x, y, value);
}
void displayDice(int dice[]) {
  M5.Lcd.setCursor(1,1);
    // Draw top row dice
    for (int i = 0; i < 3; i++) {
        drawDie(i * (DICE_SIZE + DICE_SPACING), 20, dice[i]);
    }
    // Draw bottom row dice
    for (int i = 3; i < 6; i++) {
        drawDie((i - 3) * (DICE_SIZE + DICE_SPACING), 80, dice[i]);
    }
}

// The main bulk of our server code 

void handleRoot() {
  String html = "<html><body><h1>WiFi Information</h1>";
  html += "<p>SSID: " + String(ssid) + "</p>";
  html += "<p>IP Address: " + WiFi.softAPIP().toString() + "</p>";
  html += "<h2>Available Networks:</h2><ul>";

  // Scan for WiFi networks
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    html += "<li>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</li>";
  }
    html += "<hr><br/><h2>Set RTC Date and Time</h2>";
    html += "<form action=\"setTime\" method=\"post\">";
    html += "Year: <input type=\"text\" name=\"year\"><br>";
    html += "Month: <input type=\"text\" name=\"month\"><br>";
    html += "Day: <input type=\"text\" name=\"day\"><br>";
    html += "Hour: <input type=\"text\" name=\"hour\"><br>";
    html += "Minute: <input type=\"text\" name=\"minute\"><br>";
    html += "Second: <input type=\"text\" name=\"second\"><br>";
    html += "<label for=\"ampm\">AM/PM:</label>";
    html += "<select id=\"ampm\">";
    html += "<option value=\"AM\">AM</option>";
    html += "<option value=\"PM\">PM</option>";
    html += "</select>";
    html += "<input type=\"submit\" value=\"Set Time\">";
    html += "</form>";
    html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetTime() {
 if (server.hasArg("year") && server.hasArg("month") && server.hasArg("day") &&
        server.hasArg("hour") && server.hasArg("minute") && server.hasArg("second")) {
        
        int year = server.arg("year").toInt();
        int month = server.arg("month").toInt();
        int day = server.arg("day").toInt();
        int hour = server.arg("hour").toInt();
        int minute = server.arg("minute").toInt();
        int second = server.arg("second").toInt();
      
            String ampm = server.arg("ampm");

        // Convert hour to 24-hour format
        if (ampm == "PM" && hour < 12) {
            hour += 12; // Convert PM hour to 24-hour format
        } else if (ampm == "AM" && hour == 12) {
            hour = 0; // Midnight case
        }
        // Set the RTC date and time
        StickCP2.Rtc.setDateTime({{year, month, day}, {hour, minute, second}});
        
        server.send(200, "text/html", "<html><body><h1>Time Set Successfully!</h1><a href=\"/\">Back</a></body></html>");
    } else {
        server.send(400, "text/html", "<html><body><h1>Invalid Input!</h1><a href=\"/\">Back</a></body></html>");
    }
}
// Function to get the encryption type as a string
String getEncryptionType(int type) {
  switch (type) {
    case WIFI_AUTH_OPEN: return "Open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2 PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2 PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Enterprise";
    default: return "Unknown";
  }
}
int calculateDistance(int rssi) {
    // Simple estimation based on RSSI
    // This is a placeholder; implement your own logic
    return max(0, 100 - (rssi + 100)); // Example calculation
}

void displayNetwork(int index) {
    String ssid = WiFi.SSID(index); // Get SSID
    int rssi = WiFi.RSSI(index);     // Get RSSI
    int distance = calculateDistance(rssi); // Estimate distance

    // Display SSID, RSSI, and estimated distance
    M5.Lcd.setCursor(10, 1);
    M5.Lcd.printf("SSID: %s", ssid.c_str());
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("RSSI: %d dBm", rssi);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.printf("Est. Dist: %d m", distance);

    // Visual representation of signal strength
    int barLength = map(rssi, -100, -30, 0, 200); // Map RSSI to a bar length
    M5.Lcd.fillRect(10, 60, barLength, 20, GREEN); // Draw the signal strength bar
    M5.Lcd.drawRect(10, 60, 200, 20, WHITE); // Draw the border of the bar
}
// Scan WiFi networks 
void networks() {
    M5.Lcd.fillScreen(BLACK); // Clear the screen
    M5.Lcd.setTextSize(1); 
    int numNetworks = WiFi.scanNetworks(); // Scan for networks
    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            M5.Lcd.fillScreen(BLACK); 
            displayNetwork(i); // Display each network
            delay(3000); // Pause for 3 seconds before showing the next network
        }
    } else {
        M5.Lcd.setCursor(10, 30);
        M5.Lcd.println("No networks found");
    }
}
void scroll_wifi() {   
  M5.Lcd.fillScreen(BLACK); // Clear the screen
  M5.Lcd.setTextSize(1); 
  // Scan for WiFi networks
  int n = WiFi.scanNetworks();
  String networks[n];

  for (int i = 0; i < n; ++i) {
    networks[i] = WiFi.SSID(i) + "(" + String(WiFi.RSSI(i)) + "dBm)";
  }


  // Display networks with scrolling effect
  int scrollPosition = 0;
  while (scrollPosition < n * 20) { // Adjust 20 for spacing
    M5.Lcd.fillScreen(BLACK); // Clear the screen
    for (int i = 0; i < n; ++i) {
      int yPosition = (i * 20) - scrollPosition; // Adjust 20 for spacing
      if (yPosition > -20 && yPosition < 160) { // Only display if within screen bounds
        M5.Lcd.setCursor(0, yPosition);
        M5.Lcd.setTextColor(random(0xFFFF));
        M5.Lcd.drawLine(199, 65, M5.Lcd.width(), yPosition+2, random(0xFFFF)); 
        M5.Lcd.println(" ");  
        M5.Lcd.println(networks[i]);
        M5.Lcd.println(" "); 
         M5.Lcd.drawLine(199, 64, M5.Lcd.width(), yPosition, WHITE); 
      }
    }
    scrollPosition++;
    delay(100); // Adjust delay for scroll speed
  }
  M5.Lcd.clearDisplay();
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setCursor(1,44); 
  M5.Lcd.printf("Networks found %d\n", n);
  M5.Lcd.println("Searching for encryption type. "); 
  M5.Lcd.println("Searching for channel number."); 
  delay(1000);
  int j = WiFi.scanNetworks();
  // Store network information in an array for scrolling
  String networkInfo[j];
  for (int i = 0; i < j; ++i) {
    networkInfo[i] =  WiFi.SSID(i) + 
                     "|Ch:" + String(WiFi.channel(i)) + 
                     "|Auth:" + getEncryptionType(WiFi.encryptionType(i));
  }

  // Scroll through the network information
  int displayHeight = 160; // Height of the M5StickC Plus display
  int lineHeight = 20;     // Height of each line of text
  int totalLines = j + 1;  // Total lines to display (including the count line)
  scrollPosition = 0; 
  while (scrollPosition < totalLines * lineHeight) {
    M5.Lcd.fillScreen(BLACK);

    // Color fade effect Define start and end colors
    uint32_t startColor = M5.Lcd.color565(255, 0, 0); 
    uint32_t endColor = M5.Lcd.color565(0, 255, 0); 

    // Fade effect duration and steps
    int fadeDuration = 2000; // Duration in milliseconds
    int steps = 255;         // Number of steps in the fade

    // Calculate the delay between each step
    int delayTime = fadeDuration / steps;

    // Perform the fade effect
    for (int i = 0; i <= steps; i++) {
        // Calculate the current color
        uint8_t r = (uint8_t)((1 - (float)i / steps) * ((startColor >> 16) & 0xFF) + (float)i / steps * ((endColor >> 16) & 0xFF));
        uint8_t g = (uint8_t)((1 - (float)i / steps) * ((startColor >> 8) & 0xFF) + (float)i / steps * ((endColor >> 8) & 0xFF));
        uint8_t b = (uint8_t)((1 - (float)i / steps) * (startColor & 0xFF) + (float)i / steps * (endColor & 0xFF));

        // Set the text color
        M5.Lcd.setTextColor(M5.Lcd.color565(r, g, b));
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);    
    // Display each network's information
    for (int i = 0; i < j; ++i) {
      int yPosition2 = (i * lineHeight) - scrollPosition; // Calculate y position for scrolling
      if (yPosition2 > displayHeight) break; // Stop if the line is off the screen
      if (yPosition2 > 0) { // Only display if within screen bounds
        M5.Lcd.setCursor(0, yPosition2);
        M5.Lcd.print(networkInfo[i]);
      }
    }

    scrollPosition++; // Move the scroll position up
    delay(50); // Adjust delay for scroll speed
    }
  }
  delay(1000); // Pause before the next scan
} 
// Backgound effect 
void background_one() {
    M5.Lcd.fillScreen(BLACK); 
    M5.update(); 
    while(true) {
      if (M5.BtnA.wasPressed()) { break; }
      if (M5.BtnB.wasPressed()) { break; }
      server.handleClient();
       // Define the number of zigzag lines and their height
    int lineHeight = 10; // Height of each zigzag line
    int numLines = M5.Lcd.height() / lineHeight; // Calculate number of lines to fill the screen
    server.handleClient();
    // Fade effect duration and steps
    int fadeDuration = 2000; // Duration in milliseconds
    int steps = 100;         // Number of steps in the fade
    server.handleClient();
    // Perform the fade effect
    for (int i = 0; i <= steps; i++) {
        // Generate random colors for the fade effect
        uint8_t r = random(0, 256);
        uint8_t g = random(0, 256);
        uint8_t b = random(0, 256);
    // Draw floating red orbs
        for (int k = 0; k < 5; k++) { // Number of orbs
            int orbX = random(0, M5.Lcd.width());
            int orbY = random(0, M5.Lcd.height());
            M5.Lcd.drawCircle(orbX, orbY, random(2, 10), M5.Lcd.color565(r, g, b)); // Draw red orb
            M5.Lcd.fillCircle(orbX, orbY, random(4, 16), M5.Lcd.color565(r, g, b)); // Draw red orb
        }
        // Draw the zigzag pattern
        for (int j = 0; j < numLines; j++) {
            int y = j * lineHeight;
            int x1 = (j % 2 == 0) ? 0 : M5.Lcd.width(); // Change starting x position for zigzag
            int x2 = (j % 2 == 0) ? M5.Lcd.width() : 0; // Change ending x position for zigzag
            int y1 = random(0, M5.Lcd.height());
            int y2 = random(0, M5.Lcd.height());
            int x3 = random(0, M5.Lcd.width());
            int y3 = random(0, M5.Lcd.height());
         // Draw floating red orbs
        for (int k = 0; k < 5; k++) { // Number of orbs
            int orbX = random(0, M5.Lcd.width());
            int orbY = random(0, M5.Lcd.height());
            M5.Lcd.drawCircle(orbX, orbY, random(2, 10), random(0x0FFF)); // Draw red orb
        }
            // Set the current color
            M5.Lcd.drawLine(x1, y, x2, y + lineHeight, M5.Lcd.color565(r, g, b));
            M5.Lcd.drawLine(y1, y2, x3, y3 + lineHeight, M5.Lcd.color565(r, g, b));
        }

        // Delay for the next step
        delay(fadeDuration / steps);
    }
    server.handleClient();
    // Perform the fade effect
    for (int i = 0; i <= steps; i++) {
        // Generate random colors for the fade effect
        uint8_t r = random(0, 256);
        uint8_t g = random(0, 256);
        uint8_t b = random(0, 256);
      
        // Draw the zigzag maze pattern
        for (int j = 0; j < numLines; j++) {
            int y = j * lineHeight;
            int x1 = (j % 2 == 0) ? 0 : M5.Lcd.width(); // Change starting x position for zigzag
            int x2 = (j % 2 == 0) ? M5.Lcd.width() : 0; // Change ending x position for zigzag
            int y1 = random(0, M5.Lcd.height());
            int y2 = random(0, M5.Lcd.height());
            int x3 = random(0, M5.Lcd.width());
            int y3 = random(0, M5.Lcd.height());
         // Draw floating red orbs
        for (int k = 0; k < 5; k++) { // Number of orbs
            int orbX = random(0, M5.Lcd.width());
            int orbY = random(0, M5.Lcd.height());
            M5.Lcd.drawCircle(orbX, orbY, random(2, 10), random(0x0FFF)); // Draw red orb
            M5.Lcd.fillCircle(orbX, orbY, random(4, 6), random(0xFFF4)); // Draw red orb
        }
            // Set the current color with a glowing effect
            uint32_t color = M5.Lcd.color565(r, g, b);
            M5.Lcd.drawLine(x1, y, x2, y + lineHeight, color);
            M5.Lcd.drawLine(x1, y + lineHeight, x2, y + lineHeight * 2, color);
            M5.Lcd.drawLine(x1, x3 + lineHeight, x2, y3 + lineHeight * 2, color);
        }

        // Delay for the next step
        delay(fadeDuration / steps);
    }
        // Perform the fade effect
    for (int i = 0; i <= steps; i++) {
        // Generate random colors for the background
        uint8_t bgR = random(0, 256);
        uint8_t bgG = random(0, 256);
        uint8_t bgB = random(0, 256);

      // Draw random maze-like lines
        for (int j = 0; j < 20; j++) { // Number of lines
            int x1 = random(10, M5.Lcd.width());
            int y1 = random(20, M5.Lcd.height());
            int x2 = random(30, M5.Lcd.width());
            int y2 = random(40, M5.Lcd.height());
            M5.Lcd.drawLine(x1, y1, x2, y2, M5.Lcd.color565(random(0, 256), random(0, 256), random(0, 256)));
        }
        // Draw random maze-like lines
        for (int j = 0; j < 20; j++) { // Number of lines
            int x1 = random(0, M5.Lcd.width());
            int y1 = random(0, M5.Lcd.height());
            int x2 = random(0, M5.Lcd.width());
            int y2 = random(0, M5.Lcd.height());
            M5.Lcd.drawLine(x1, y1, x2, y2, M5.Lcd.color565(random(0, 256), random(0, 256), random(0, 256)));
        }

        // Draw floating red orbs
        for (int k = 0; k < 5; k++) { // Number of orbs
            int orbX = random(0, M5.Lcd.width());
            int orbY = random(0, M5.Lcd.height());
            M5.Lcd.fillCircle(orbX, orbY, 10, random(0xFFFF)); // Draw red orb
        }
         server.handleClient();
        // Delay for the next step
        delay(fadeDuration / steps);
    }
    // Number of triangles to draw
    int numTriangles = 750; // Adjust this for more or fewer triangles
 // Draw floating red orbs
        for (int k = 0; k < 5; k++) { // Number of orbs
            int orbX = random(0, M5.Lcd.width());
            int orbY = random(0, M5.Lcd.height());
            M5.Lcd.drawCircle(orbX, orbY, random(2, 20), M5.Lcd.color565(r, g, b)); // Draw red orb
            M5.Lcd.fillCircle(orbX, orbY, random(4, 39), M5.Lcd.color565(r, g, b)); // Draw red orb
        }
    for (int i = 0; i < numTriangles; i++) {
        // Generate random coordinates for the triangle vertices
        int x1 = random(0, M5.Lcd.width());
        int y1 = random(0, M5.Lcd.height());
        int x2 = random(0, M5.Lcd.width());
        int y2 = random(0, M5.Lcd.height());
        int x3 = random(0, M5.Lcd.width());
        int y3 = random(0, M5.Lcd.height());

        // Generate a random color for the triangle
        uint32_t color = M5.Lcd.color565(random(0, 256), random(0, 256), random(0, 256));
        int orbX = random(0, M5.Lcd.width());
        int orbY = random(0, M5.Lcd.height());
        // Draw the triangle
        M5.Lcd.drawTriangle(x1, y1, x2, y2, x3, y3, color);
        M5.Lcd.fillCircle(orbX, orbY, random(39), M5.Lcd.color565(r, g, b)); // Draw red orb
        M5.Lcd.drawTriangle(x3, y2, x2, y2, x2, y1, M5.Lcd.color565(r, g, b)); 
    }
      if (M5.BtnA.pressedFor(1000)) { break; }
      if (M5.BtnB.pressedFor(1000)) { break; } 
}
}

void drawFractal() {
  int maxIterations = 10; // Set the maximum number of iterations
  int counter = 0; // Initialize the counter
  server.handleClient();
  while(counter < maxIterations) {
    server.handleClient();
  // Draw the fractal
  drawFractalx(); 
  // Zoom in and out
  zoom *= 1.01; // Adjust zoom speed
  moveX += 0.01; // Adjust horizontal movement
  moveY += 0.01; // Adjust vertical movement
  counter++;     // count up maximum iterations 
  // Check if the counter is a multiple of 10
    if (counter % 10 == 0) {
       break; 
    }
  } 
  for (int x=0; x<152; x++) {
    drawBarnsleyFern(); 
  }
  drawMandelbrot();
    for (int x=0; x<154; x++) {
    drawBarnsleyFern(); 
  }
  M5.Lcd.fillScreen(BLACK); 
  counter = 0; 
      for (int x=0; x<1554; x++) {
    if (counter % 10 == 0) { M5.Lcd.fillScreen(BLACK); }
    drawBarnsleyFern(); 
    delay(30); 
    server.handleClient();
    counter++; 
  }
}

void drawBarnsleyFern() {
  float x = 0, y = 0;
  int width = M5.Lcd.width();
  int height = M5.Lcd.height();
  server.handleClient();
   // Generate random offsets for position
  float offsetX = random(-width / 2, width / 2);
  float offsetY = random(-height / 2, height / 2);
  // Generate a random scale factor between 5 and 15
  float scale = random(5, 15);
  // Generate a random angle in radians
  float angle = random(0, 360) * (PI / 180); // Convert degrees to radians
  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  // Generate random color 
  int colorQ =  random(0xFFFF); 

  for (int i = 0; i < 10000; i++) {
    float r = random(0, 100);
    float newX, newY;
    
    if (r < 1) {
      newX = 0;
      newY = 0.16 * y;
    } else if (r < 86) {
      newX = 0.85 * x + 0.04 * y;
      newY = -0.04 * x + 0.85 * y + 1.6;
    } else if (r < 93) {
      newX = 0.2 * x - 0.26 * y;
      newY = 0.23 * x + 0.22 * y + 1.6;
    } else {
      newX = -0.15 * x + 0.28 * y;
      newY = 0.26 * x + 0.24 * y + 0.44;
    }
    
    x = newX;
    y = newY;

    // Scale and translate to fit the screen
    int screenX = (int)(width / 2 + (x * scale * cosAngle - y * scale * sinAngle) + offsetX);
    int screenY = (int)(height - ((x * scale * sinAngle + y * scale * cosAngle) + offsetY) - 1); // Invert Y for display

    // Draw the point
    M5.Lcd.drawPixel(screenX, screenY, colorQ);
  }
}
void drawMandelbrot() {
  int width = M5.Lcd.width();
  int height = M5.Lcd.height();
  
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      float zx = 0.0;
      float zy = 0.0;
      float cX = 1.5 * (x - width / 2) / (0.5 * width);
      float cY = (y - height / 2) / (0.5 * height);
      int i = 0;
      const int maxIterations = 256;
      while (zx * zx + zy * zy < 4 && i < maxIterations) {
        float tmp = zx * zx - zy * zy + cX;
        zy = 2.0 * zx * zy + cY;
        zx = tmp;
        i++;
      }
      int color = (i == maxIterations) ? 0 : (i * 255 / maxIterations);
      M5.Lcd.drawPixel(x, y, M5.Lcd.color565(100 - color, 200 - color, 255 - color));
    }
  }
}
void drawFractalx() {
  int width = M5.Lcd.width();
  int height = M5.Lcd.height();
  
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      float zx = 1.5 * (x - width / 2) / (0.5 * width * zoom) + moveX;
      float zy = (y - height / 2) / (0.5 * height * zoom) + moveY;
      int i = 0;
      const int maxIterations = 120;
      while (zx * zx + zy * zy < 4 && i < maxIterations) {
        float tmp = zx * zx - zy * zy + -0.7; // Change this for different fractals
        zy = 2.0 * zx * zy + 0.27015; // Change this for different fractals
        zx = tmp;
        i++;
        server.handleClient();
      }
      
      // Smooth color mapping
      int color = (i == maxIterations) ? 0 : (i * 255 / maxIterations);
      int r = (color * 9) % 256; // Red component
      int g = (color * 15) % 256; // Green component
      int b = (color * 7) % 256;  // Blue component
      M5.Lcd.drawPixel(x, y, M5.Lcd.color565(r, g, b)); // Smooth color gradient
    }
  }
}

void essid_spam() {
    for (int essidIndex = 0; essidIndex < totalEssids; essidIndex++) {
        for (int i = 0; i < totalSpams; i++) {
            String randomEssid = generateRandomEssid();
            String randomMac = generateRandomMac();
            WiFi.softAP(randomEssid.c_str(), NULL, 0, false, 1); // Start AP with random ESSID and MAC
            displayProgress(essidIndex + 1, totalEssids, i + 1, totalSpams, randomEssid);
            delay(spamInterval);
        }
        WiFi.softAPdisconnect(true); // Disconnect after spamming each ESSID
    }
    M5.update(); 
    while(true) {
    M5.update(); 
    if(M5.BtnA.wasPressed()) { break; }
    if(M5.BtnB.wasPressed()) { break; }
    server.handleClient();
    }
}
String generateRandomEssid() {
    const char charset[] = "$!#%&*()-=+[]{}|\:;<>?/~^.,ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    String essid = "";
    for (int i = 0; i < essidLength; i++) {
        int index = random(0, sizeof(charset) - 1);
        essid += charset[index];
    }
    return essid;
}

String generateRandomMac() {
    String mac = "";
    for (int i = 0; i < 6; i++) {
        int byteValue = random(0, 256);
        if (i > 0) mac += ":";
        mac += String(byteValue, HEX);
    }
    return mac;
}

void displayProgress(int currentEssid, int totalEssids, int currentSpam, int totalSpams, String essid) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1.85);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Spamming:"); 
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setTextColor(BLUE); 
    M5.Lcd.printf("\t%s", essid.c_str());
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print("Overall essids:");
    M5.Lcd.setTextColor(YELLOW); 
    M5.Lcd.printf("%d/%d", currentEssid, totalEssids);
    M5.Lcd.setTextColor(WHITE,BLUE); 
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("Progress: ");
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.printf("%d %%", currentSpam);
    
    int barWidth = (M5.Lcd.width() - 20);
    int filledWidth = (barWidth * currentSpam) / totalSpams;
    M5.Lcd.fillRect(10, 100, filledWidth, 20,RED);
    M5.Lcd.drawRect(10, 100, barWidth, 20, WHITE);
}

void thetime() {
      // Set the maximum number of loops
  const int max_loops = 2500;
  static int loop_counter = 0; // Use static to retain value between loop iterations

   while(loop_counter < max_loops) {
   server.handleClient(); 
   M5.update(); 
   if(M5.BtnA.wasPressed()) {
   M5.Lcd.fillScreen(BLACK);
   break;  
   } else if (M5.BtnB.wasPressed()) {
    M5.Lcd.fillScreen(BLACK); 
    break; 
   }
   static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    loop_counter++; 
    // Clear the display before updating
    StickCP2.Display.fillScreen(BLACK);
    M5.Lcd.fillScreen(BLACK); 
    if (loop_counter % 60 == 0) {
    drawBarnsleyFern(); 
    drawBarnsleyFern(); 
   }
    drawBarnsleyFern();
    // Do loop color timer 
      if (loop_counter == 1) {
      M5.Lcd.setTextColor(ORANGE, BLACK); 
    } else if (loop_counter % 100 == 0) {
      M5.Lcd.setTextColor(ORANGE, BLACK); 
    }
     // Check if the loop counter is a multiple of 20
    if (loop_counter % 20 == 0) {
    M5.Lcd.setTextColor(WHITE, BLACK); 
    }
    // Clear the display before updating
    M5.Lcd.fillScreen(BLACK);
    drawBarnsleyFern();
    if (loop_counter % 40 == 0) {
    M5.Lcd.setTextColor(RED, BLACK); 
    }
    // Get the current ESP32 internal time
    auto t = time(nullptr);
    {

        auto tm = gmtime(&t);  // for UTC.
        
        // Display ESP32 UTC date and time
        StickCP2.Display.setCursor(0, 0);
        StickCP2.Display.printf("%04d/%02d/%02d \n(%s)\n  %02d:%02d:%02d\n",
                                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                                wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    drawBarnsleyFern();
    delay(1000); 
    M5.Lcd.fillScreen(BLACK); 
    drawBarnsleyFern();
    // Get the current local time
    {
        auto tm = localtime(&t);  // for local timezone.
        // Display local time
        StickCP2.Display.setCursor(0, 0);
        StickCP2.Display.printf("%04d/%02d/%02d \n(%s)\n  %02d:%02d:%02d\n",
                                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                                wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
 
    }
    drawBarnsleyFern();
    delay(1000); 
    M5.Lcd.fillScreen(BLACK);
         if (loop_counter % 70 == 0) {
  drawBarnsleyFern(); 
  drawBarnsleyFern(); 
  } else {
    drawBarnsleyFern();
  }
        auto tm = localtime(&t);  // for local timezone.
        // Display local time
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.printf("%04d/%02d/%02d \n(%s)\n  %02d:%02d:%02d\n",
                                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                                wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
   delay(1000); 
}
}

void the12time() {
    // Set the maximum number of loops
  const int max_loops = 2500;
  static int loop_counter = 0; // Use static to retain value between loop iterations

  while(loop_counter < max_loops) {
      server.handleClient();
      M5.update(); 
   if(M5.BtnA.wasPressed()) {
   M5.Lcd.fillScreen(BLACK);
   break;  
   } else if (M5.BtnB.wasPressed()) {
    M5.Lcd.fillScreen(BLACK); 
    break; 
   }
     static constexpr const char* const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    
    loop_counter++; // Increment the loop counter
    if (loop_counter == 1 ) {
      M5.Lcd.setTextColor(ORANGE, BLACK); 
    } else if (loop_counter % 100 == 0) {
      M5.Lcd.setTextColor(ORANGE, BLACK); 
    }
     // Check if the loop counter is a multiple of 20
    if (loop_counter % 20 == 0) {
    M5.Lcd.setTextColor(WHITE, BLACK); 
    }
    // Clear the display before updating
    M5.Lcd.fillScreen(BLACK);
    drawBarnsleyFern(); 
    if (loop_counter % 40 == 0) {
    M5.Lcd.setTextColor(RED, BLACK); 
    }
    M5.Lcd.setCursor(0,0);
    // Get the current RTC date and time
    auto dt = StickCP2.Rtc.getDateTime();
  
    // Convert to 12-hour format for RTC time
    int hour12 = dt.time.hours % 12; // Convert to 12-hour format
    hour12 = (hour12 == 0) ? 12 : hour12; // Adjust 0 to 12
    const char* ampmRTC = (dt.time.hours >= 12) ? "PM" : "AM";
    M5.Lcd.fillScreen(BLACK);
      if (loop_counter % 60 == 0) {
  drawBarnsleyFern(); 
  drawBarnsleyFern(); 
  } else { 
    drawBarnsleyFern(); 
  }
    // Display RTC date and time in 12-hour format
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%04d/%02d/%02d \n(%s)\n  %02d:%02d:%02d %s\n",
                            dt.date.year, dt.date.month, dt.date.date,
                            wd[dt.date.weekDay], hour12, dt.time.minutes,
                            dt.time.seconds, ampmRTC);
                            drawBarnsleyFern();
    delay(1000); 
    // Get the current ESP32 internal time
    auto t = time(nullptr);
    {
        auto tm = gmtime(&t);  // for UTC.
        int hour12ESP = tm->tm_hour % 12; // Convert to 12-hour format
        hour12ESP = (hour12ESP == 0) ? 12 : hour12ESP; // Adjust 0 to 12
        const char* ampmESP = (tm->tm_hour >= 12) ? "PM" : "AM";
        M5.Lcd.fillScreen(BLACK);
        drawBarnsleyFern();
        // Display ESP32 UTC date and time in 12-hour format
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.printf("%04d/%02d/%02d \n(%s)\n  %02d:%02d:%02d %s\n",
                                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                                wd[tm->tm_wday], hour12ESP, tm->tm_min, tm->tm_sec, ampmESP);
                                drawBarnsleyFern();
                                delay(1000); 
    }
  }
}

uint16_t getRandomColor() {
    // Generate a random color
    return M5.Lcd.color565(random(0, 256), random(0, 256), random(0, 256));
}
void rotatePoint(float &x, float &y, float angle) {
    float tempX = x * cos(angle) - y * sin(angle);
    float tempY = x * sin(angle) + y * cos(angle);
    x = tempX;
    y = tempY;
}

void qubex(void) {
  while(true) {
    server.handleClient();
    M5.update(); 
    if(M5.BtnA.wasPressed()) { 
      M5.Lcd.fillScreen(BLACK); 
      break; 
    }
       // Check for IMU updates
    auto imu_update = StickCP2.Imu.update();
    if (imu_update) {
        // Get accelerometer data
        float ax, ay, az;
        StickCP2.Imu.getAccelData(&ax, &ay, &az);

        // Get gyroscope data
        float gx, gy, gz;
        StickCP2.Imu.getGyroData(&gx, &gy, &gz);

        // Clear the screen
        M5.Lcd.fillScreen(BLACK);

        // Draw random line patterns based on IMU data
        for (int i = 0; i < 45; i++) { // Draw 10 lines
            // Calculate line start and end points based on IMU data
            int x1 = map(ax * 15, -20, 20, 0, SCREEN_WIDTH);
            int y1 = map(ay * 15, -20, 20, 0, SCREEN_HEIGHT);
            int x2 = random(0, SCREEN_WIDTH);
            int y2 = random(0, SCREEN_HEIGHT);
            int x3 = random(1, SCREEN_WIDTH-5); 
            int y3 = random(1, SCREEN_HEIGHT-5); 
            // Draw the line with a random color
            M5.Lcd.drawLine(x1, y1, x2, y2, getRandomColor());
            int ax1 = map(ax * 10, -20, 20, 0, SCREEN_WIDTH);
            int ay1 = map(ay * 10, -20, 20, 0, SCREEN_HEIGHT);
            int gx1 = map(gx * 30, -10, 30, 1, SCREEN_HEIGHT);
            int gy1 = map(gy * 30, -10, 30, 1, SCREEN_WIDTH);
            M5.Lcd.drawPixel(x2, y2, getRandomColor()); 
            M5.Lcd.drawLine(gx1, gy1, x3, y3, getRandomColor());
            M5.Lcd.drawCircle(y3, x3, gx1,getRandomColor());
            M5.Lcd.drawCircle(y3, x3, gy1,getRandomColor());
            M5.Lcd.drawCircleHelper(ax1, ay1, gx1, 8, getRandomColor()); 
            M5.Lcd.drawPixel(y1, x1, ORANGE); 
            M5.Lcd.drawPixel(gx1, gy1, GREEN); 
        }
        // Draw random line patterns based on gyroscope data
        for (int i = 0; i < 50; i++) { // Draw 20 lines
            // Random length and angle for the lines
            float length = random(20, 50);
            float angle = gz * 0.1; // Scale gyroscope Z data for rotation

            // Calculate the end point of the line
            float x1 = CENTER_X;
            float y1 = CENTER_Y;
            float x2 = length; // Initial x2 position
            float y2 = 0;      // Initial y2 position

            // Rotate the end point based on the gyroscope Z data
            rotatePoint(x2, y2, angle);

            // Translate the rotated point to the center of the screen
            x2 += CENTER_X;
            y2 += CENTER_Y;

            // Draw the line with a random color
            M5.Lcd.drawLine(x1, y1, x2, y2, getRandomColor());
     }
     } 
    delay(100); 
  }
}


void drawTriangle(float x, float y, float size) {
    // Calculate the vertices of the triangle
    float halfSize = size / 2;
    float height = (sqrt(3) / 2) * size; // Height of the equilateral triangle

    // Draw the triangle using three lines
    StickCP2.Display.drawLine(x, y - (2.0 / 3.0) * height, x - halfSize, y + (1.0 / 3.0) * height, ORANGE); // Left side
    StickCP2.Display.drawLine(x - halfSize, y + (1.0 / 3.0) * height, x + halfSize, y + (1.0 / 3.0) * height, ORANGE); // Base
    StickCP2.Display.drawLine(x + halfSize, y + (1.0 / 3.0) * height, x, y - (2.0 / 3.0) * height, ORANGE); // Right side
}

void obj_hex(void) {
   // Initialize cube positions
    for (int i = 0; i < numCubes; i++) {
        cubePositions[i][0] = random(0, StickCP2.Display.width()); // Random X position
        cubePositions[i][1] = random(0, StickCP2.Display.height()); // Random Y position
    }

    while(true) {
    server.handleClient();
    M5.update(); 
    if(M5.BtnA.wasPressed()) { 
      M5.Lcd.fillScreen(BLACK); 
      break; 
    }
     auto imu_update = StickCP2.Imu.update();
    if (imu_update) {
        auto data = StickCP2.Imu.getImuData();

        // Update speed based on IMU data
        speedX = data.accel.x * 2; // Scale the acceleration for speed
        speedY = data.accel.y * 2; // Scale the acceleration for speed

        // Update cube positions based on speed
        for (int i = 0; i < numCubes; i++) {
            cubePositions[i][0] += speedX; // Update X position
            cubePositions[i][1] += speedY; // Update Y position

            // Bounce off the edges
            if (cubePositions[i][0] < 0 || cubePositions[i][0] > StickCP2.Display.width()) {
                speedX = -speedX; // Reverse direction
                cubePositions[i][0] = constrain(cubePositions[i][0], 0, StickCP2.Display.width());
            }
            if (cubePositions[i][1] < 0 || cubePositions[i][1] > StickCP2.Display.height()) {
                speedY = -speedY; // Reverse direction
                cubePositions[i][1] = constrain(cubePositions[i][1], 0, StickCP2.Display.height());
            }
        }

        // Clear the display
        StickCP2.Display.clear();

        // Draw cubes at their updated positions
        for (int i = 0; i < numCubes; i++) {
            StickCP2.Display.fillRect(cubePositions[i][0], cubePositions[i][1], cubeSize, cubeSize, random(0xFFFF));

        }
  
    delay(100); // Delay for 100 milliseconds
}
}
}

void drawTriangle(float x, float y, float size, float angle) {
    // Calculate the vertices of the triangle
    float halfSize = size / 2;
    float height = (sqrt(3) / 2) * size; // Height of the equilateral triangle

    // Calculate the rotation in radians
    float rad = angle * (PI / 180.0);

    // Calculate the rotated vertices
    float x1 = x + cos(rad) * 0 - sin(rad) * (2.0 / 3.0) * height;
    float y1 = y + sin(rad) * 0 + cos(rad) * (2.0 / 3.0) * height;

    float x2 = x + cos(rad) * (-halfSize) - sin(rad) * (1.0 / 3.0) * height;
    float y2 = y + sin(rad) * (-halfSize) + cos(rad) * (1.0 / 3.0) * height;

    float x3 = x + cos(rad) * halfSize - sin(rad) * (1.0 / 3.0) * height;
    float y3 = y + sin(rad) * halfSize + cos(rad) * (1.0 / 3.0) * height;

    // Draw the triangle using three lines
    StickCP2.Display.drawLine(x1, y1, x2, y2, ORANGE); // Left side
    StickCP2.Display.drawLine(x2, y2, x3, y3, ORANGE); // Base
    StickCP2.Display.drawLine(x3, y3, x1, y1, ORANGE); // Right side
    
    StickCP2.Display.drawCircle(x1, y1, 9, getRandomColor()); 
    StickCP2.Display.drawCircle(x2, y2, 7, getRandomColor());
    StickCP2.Display.drawCircle(x3, y3, 8, getRandomColor()); 
}

void objtri(void){
    // Initialize triangle positions and angles
    for (int i = 0; i < numTriangles; i++) {
        trianglePositions[i][0] = random(0, StickCP2.Display.width()); // Random X position
        trianglePositions[i][1] = random(0, StickCP2.Display.height()); // Random Y position
        triangleAngles[i] = random(0, 360); // Random initial angle
    }

while(true) {
  server.handleClient();
   M5.update(); 
    if(M5.BtnA.wasPressed()) { 
      M5.Lcd.fillScreen(BLACK); 
      break; 
    }
   auto imu_update = StickCP2.Imu.update();
    if (imu_update) {
        auto data = StickCP2.Imu.getImuData();

        // Update speed based on IMU data
        speedX = data.accel.x * 2; // Scale the acceleration for speed
        speedY = data.accel.y * 2; // Scale the acceleration for speed

        // Update triangle positions and angles based on speed and gyroscope data
        for (int i = 0; i < numTriangles; i++) {
            trianglePositions[i][0] += speedX; // Update X position
            trianglePositions[i][1] += speedY; // Update Y position

            // Update angle based on gyroscope Z-axis data
            triangleAngles[i] += data.gyro.z; // Increment angle based on gyroscope data

            // Bounce off the edges
            if (trianglePositions[i][0] < 0 || trianglePositions[i][0] > StickCP2.Display.width()) {
                speedX = -speedX; // Reverse direction
                trianglePositions[i][0] = constrain(trianglePositions[i][0], 0, StickCP2.Display.width());
            }
            if (trianglePositions[i][1] < 0 || trianglePositions[i][1] > StickCP2.Display.height()) {
                speedY = -speedY; // Reverse direction
                trianglePositions[i][1] = constrain(trianglePositions[i][1], 0, StickCP2.Display.height());
            }
        }

        // Clear the display
        StickCP2.Display.clear();

        // Draw triangles at their updated positions with the current angle
        for (int i = 0; i < numTriangles; i++) {
            drawTriangle(trianglePositions[i][0], trianglePositions[i][1], triangleSize, triangleAngles[i]);
        }
      } 
    delay(100); // Delay for 100 milliseconds
}
}
