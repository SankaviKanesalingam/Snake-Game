#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Pin definitions for the TFT display and joystick
#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8
#define JOY_X      A0
#define JOY_Y      A1
#define BUTTON_PIN 2 // Joystick button
#define BUZZER_PIN 7


// Create TFT display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Snake properties
int snakeX[100], snakeY[100]; // Snake coordinates
int snakeLength = 2;         // Initial snake length
int foodX, foodY;             // Food coordinates
int redFoodX[5], redFoodY[5]; // Array for red food coordinates (up to 5)
int redFoodCount = 0;         // Number of red food items active
int score = 0;                // Player score
int level = 1;                // Game level
int direction = 1;            // Initial direction (0: up, 1: right, 2: down, 3: left)
bool foodActive = false;      // Tracks if food is currently active
bool redFoodActive[5] = {false};   // Tracks if red food is currently active
int delayTime = 400; // Initial delay for snake speed

// Barrier properties for Level 2 and Level 3
int barrierX, barrierY;       // Barrier coordinates
const int barrierDigit = 7;   // Example digit for barrier (replace with actual last digit)
bool barrierActive = false;

// Timer variables for food disappearance in Level 3
unsigned long foodTimer = 0;  // Timer for food
int foodCountdown = 5;        // Food countdown in seconds

// Function declarations
void spawnFood();
void spawnRedFood(int index); // Modified spawnRedFood to accept an index
bool isFoodTooCloseToBarrier(int foodX, int foodY);
void increaseSpeed();

// Function to initialize the game
void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);

  // Initialize joystick button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output

  // Initial snake position 
  snakeX[0] = tft.width() / 2;
  snakeY[0] = tft.height() / 2;

  spawnFood(); // Spawn initial food

  // Initial Display Setup
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  // Print the labels
  tft.setCursor(0, 0);
  tft.print("Score: ");
  tft.setCursor(100, 0);
  tft.print("Level: ");
  tft.setCursor(180, 0);
  tft.print("Timer: ");

  // Display start message
  tft.setCursor(60, 100);
  tft.print("Press button to start");
}

// Function to spawn food at a random location
void spawnFood() {
  // Ensure food is not too close to the barrier for Level 2 and Level 3
  do {
    foodX = random(0, tft.width() / 10) * 10;
    foodY = random(0, tft.height() / 10) * 10;
  } while (level >= 2 && isFoodTooCloseToBarrier(foodX, foodY)); // Check for Level 2 and above

  foodActive = true;         // Food is active
  foodCountdown = 5;         // Reset countdown to 5 seconds
  foodTimer = millis();      // Record the time when food is spawned
}

// Function to spawn red food for Level 4 and above
void spawnRedFood(int index) { // Modified spawnRedFood to accept an index
  // Ensure red food is not too close to the barrier or normal food
  do {
    redFoodX[index] = random(0, tft.width() / 10) * 10;
    redFoodY[index] = random(0, tft.height() / 10) * 10;
  } while ((level >= 2 && isFoodTooCloseToBarrier(redFoodX[index], redFoodY[index])) ||
           (redFoodX[index] == foodX && redFoodY[index] == foodY) ||
           (index > 0 && (redFoodX[index] == redFoodX[index - 1] &&
                                  redFoodY[index] == redFoodY[index - 1])));

  redFoodActive[index] = true; // Red food is now active
  tft.fillRect(redFoodX[index], redFoodY[index], 10, 10, ILI9341_RED); // Draw red food immediately
}

// Function to check if food is too close to the barrier for Level 2 and Level 3
bool isFoodTooCloseToBarrier(int foodX, int foodY) {
  if (barrierActive) {
    return (abs(foodX - barrierX) < 10 && abs(foodY - barrierY) < 10);
  }
  return false;
}

// Function to spawn a barrier at a random location for Level 2 and Level 3
void spawnBarrier() {
  barrierX = random(0, tft.width() / 10) * 10;
  barrierY = random(0, tft.height() / 10) * 10;

  // If the snake is near the center, move it to a corner
  if (snakeX[0] > tft.width() / 4 && snakeX[0] < 3 * tft.width() / 4 &&
      snakeY[0] > tft.height() / 4 && snakeY[0] < 3 * tft.height() / 4) {
    snakeX[0] = 10; // Move to top-left corner
    snakeY[0] = 10;
  }

  barrierActive = true;
}

// In the 'drawSnake' function, change the color to white:
void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    if (i == 0) { // Only for the head (i = 0)
      tft.fillRect(snakeX[i], snakeY[i], 10, 10, ILI9341_WHITE); // Head color is white
    } else {
      tft.fillRect(snakeX[i], snakeY[i], 10, 10, ILI9341_BLUE); // Body color is blue
    }
  }
}

// Function to clear the snake's previous position on the screen
void clearSnake() {
  for (int i = 0; i < snakeLength; i++) {
    tft.fillRect(snakeX[i], snakeY[i], 10, 10, ILI9341_BLACK);
  }
}

// Function to draw food on the screen if active
void drawFood() {
  if (foodActive) {
    tft.fillRect(foodX, foodY, 10, 10, ILI9341_GREEN);
  }

  // Draw red food if active for Level 4 and above
  for (int i = 0; i < redFoodCount; i++) {
    if (redFoodActive[i]) {
      tft.fillRect(redFoodX[i], redFoodY[i], 10, 10, ILI9341_RED);
    }
  }
}

// Function to draw the barrier on the screen for Level 2 and Level 3
void drawBarrier() {
  if (barrierActive && level >= 2) { // Draw barrier for Level 2 and above
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(10);
    // Calculate the center coordinates based on the screen width and height
    barrierX = (tft.width() - 20) / 2; // Adjust 20 for barrier width
    barrierY = (tft.height() - 24) / 2; // Adjust 24 for barrier height
    tft.setCursor(barrierX, barrierY);
    tft.print(barrierDigit); // Print the barrier digit
  }
}

// Variables to store the previous score and level
int previousScore = -1;
int previousLevel = -1;

void displayScoreAndCountdown() {
  // Check if the score has changed
  if (score != previousScore) {
    // Clear the area for the Score
    tft.fillRect(0, 0, 170, 20, ILI9341_BLACK); // Clear previous score area
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(score);
    previousScore = score; // Update the previous score
  }
  
  // Check if the level has changed
  if (level != previousLevel) {
    // Clear the area for the Level
    tft.fillRect(0, 20, 170, 20, ILI9341_BLACK); // Clear previous level area
    tft.setCursor(0, 20);
    tft.print("Level: ");
    tft.print(level);
    previousLevel = level; // Update the previous level
  }



  // Display countdown timer for Level 3 and above, and clear the area first
  if (level >= 3 && foodActive) {
    tft.fillRect(180, 0, 120, 20, ILI9341_BLACK); // Adjust dimensions based on your screen and text size
    tft.setCursor(180, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("Timer: ");
    tft.print(foodCountdown);
  }
}


// Function to update the snake's position based on direction
void updateSnake() {
  // Move snake body
  for (int i = snakeLength; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Update head position based on direction
  switch (direction) {
    case 0: snakeY[0] -= 10; break; // Up
    case 1: snakeX[0] += 10; break; // Right
    case 2: snakeY[0] += 10; break; // Down
    case 3: snakeX[0] -= 10; break; // Left
  }

  // Wrap around screen edges
  if (snakeX[0] >= tft.width()) snakeX[0] = 0;
  if (snakeX[0] < 0) snakeX[0] = tft.width() - 10;
  if (snakeY[0] >= tft.height()) snakeY[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = tft.height() - 10;
}

// Function to check for collisions (self or barrier)
bool checkCollision() {
  // Check for self-collision
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      return true; // Snake collided with itself
    }
  }
  // Check for barrier collision in Level 2 and Level 3
  if (level >= 2 && barrierActive) {
    // Check if snake's head touches the barrier digit
    if (snakeX[0] >= barrierX && snakeX[0] < barrierX + 20 &&
        snakeY[0] >= barrierY && snakeY[0] < barrierY + 24) {
      return true; // Snake collided with barrier digit
    }
  }
  return false;
}

// Function to update the countdown timer for food in Level 3
void updateFoodTimer() {
  if (level >= 3 && foodActive) {
    // Only for Level 3
    // Check if 1 second has passed since the last update
    if (millis() - foodTimer >= 1000) {
      foodTimer = millis(); // Reset timer
      foodCountdown--;      // Decrease countdown

      // If countdown reaches zero, remove the food
      if (foodCountdown <= 0) {
        tft.fillRect(foodX, foodY, 10, 10, ILI9341_BLACK); // Clear food from screen
        foodActive = false;  // Food is no longer active
      }
    }
  }
}

// Function to increase snake speed by 20% for each new level after Level 4
void increaseSpeed() {
  if (level > 4) {
    delayTime = delayTime * 0.8; // Decrease delay by 20% for faster speed
  }
}

// Function to spawn multiple red foods based on count
void spawnRedFoods(int count) {
  for (int i = 0; i < count; i++) {
    if (!redFoodActive[i]) {
      spawnRedFood(i);
    }
  }
}

void loop() {
  if (!digitalRead(BUTTON_PIN)) {
    // Start the game if the button is pressed
    tft.fillRect(50, 95, 270, 30, ILI9341_BLACK);

    while (true) {
    clearSnake();  // Clear previous position
    updateSnake(); // Update snake position
    drawSnake();   // Draw snake

    // Check for food collision
    if (foodActive && snakeX[0] == foodX && snakeY[0] == foodY) {
      score++;
      snakeLength++;
      foodActive = false; // Food eaten, set to inactive
      tone(BUZZER_PIN, 1000, 500);  // Play 1000 Hz sound for 200 ms
    }

    // Check for red food collisions
    for (int i = 0; i < redFoodCount; i++) {
      if (redFoodActive[i] && snakeX[0] == redFoodX[i] && snakeY[0] == redFoodY[i]) {
        score--;           // Decrease score by 1 when red food is eaten
        redFoodActive[i] = false;  // Set red food as inactive
        tone(BUZZER_PIN, 500, 1000);  // Play 500 Hz sound for 200 ms
      }
    }

    // Respawn red food if it's eaten
    for (int i = 0; i < redFoodCount; i++) {
      if (!redFoodActive[i]) {
        spawnRedFood(i); // Spawn a new red food item
      }
    }

    // Spawn new food if the previous food is no longer active
    if (!foodActive) {
      spawnFood();
    }

    // Level 2: Spawn barrier only at this level
    if (level >= 2 && !barrierActive) {
      spawnBarrier();
    }

    // Update level based on score
    if (score > 0 && score % 2 == 0) {
      level = score / 2 + 1;
      increaseSpeed(); // Increase speed for levels above 4
    }

    // Spawn red foods based on level
    if (level >= 4) {
      spawnRedFoods(level - 3); // Spawn red foods based on level difference from level 4
    }

    // Draw food and barrier (if any)
    drawFood();
    drawBarrier();

    // Display score and countdown (if applicable)
    displayScoreAndCountdown();

    // Update food timer and remove food if necessary (Level 3)
    updateFoodTimer();

    // Check for collisions and end the game if collision detected
    if (checkCollision()) {
      tft.fillScreen(ILI9341_BLACK); // Clear screen
      tft.setCursor(60, 100);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.print("Game Over");
      tft.setCursor(40, 150);
      tft.print("Final Score: ");
      tft.print(score);

      // Game over sound effect
    tone(BUZZER_PIN, 100, 2000);  // Play low sound (100 Hz) for 1 second
      while (true); // Halt the game
    }

    // Read joystick input to change direction
    int joyX = analogRead(JOY_X);
    int joyY = analogRead(JOY_Y);

    if (joyX < 400) direction = 0; // Move left
    else if (joyX > 600) direction = 2; // Move right
    if (joyY < 400) direction = 1; // Move up
    else if (joyY > 600) direction = 3; // Move down

    delay(delayTime); // Adjust delay for snake speed
  }
  }
}