#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wi-Fi credentials
const char* ssid = "__";
const char* password = "__";

// Global vars
bool spacePressed = false;

// Define GPIO pins for buttons
#define DOT_BUTTON D1
#define DASH_BUTTON D2
#define BACKSPACE_BUTTON D3
#define SPACE_BUTTON D4

// Create web server on port 80
ESP8266WebServer server(80);

// Morse tree structure
struct MorseNode {
  char letter;
  MorseNode* dot;
  MorseNode* dash;
  MorseNode()
    : letter(0), dot(nullptr), dash(nullptr) {}
};

// Root of Morse tree
MorseNode* root = new MorseNode();

// Morse input buffer
String morseInput = "";
String decodedText = "";

// Function to insert Morse code into tree
void insert(const String& morse, char letter) {
  MorseNode* node = root;
  for (char c : morse) {
    if (c == '.') {
      if (!node->dot) node->dot = new MorseNode();
      node = node->dot;
    } else if (c == '-') {
      if (!node->dash) node->dash = new MorseNode();
      node = node->dash;
    }
  }
  node->letter = letter;
}

// Function to build Morse tree
void buildMorseTree() {
  insert(".-", 'A');
  insert("-...", 'B');
  insert("-.-.", 'C');
  insert("-..", 'D');
  insert(".", 'E');
  insert("..-.", 'F');
  insert("--.", 'G');
  insert("....", 'H');
  insert("..", 'I');
  insert(".---", 'J');
  insert("-.-", 'K');
  insert(".-..", 'L');
  insert("--", 'M');
  insert("-.", 'N');
  insert("---", 'O');
  insert(".--.", 'P');
  insert("--.-", 'Q');
  insert(".-.", 'R');
  insert("...", 'S');
  insert("-", 'T');
  insert("..-", 'U');
  insert("...-", 'V');
  insert(".--", 'W');
  insert("-..-", 'X');
  insert("-.--", 'Y');
  insert("--..", 'Z');
  insert("-----", '0');
  insert(".----", '1');
  insert("..---", '2');
  insert("...--", '3');
  insert("....-", '4');
  insert(".....", '5');
  insert("-....", '6');
  insert("--...", '7');
  insert("---..", '8');
  insert("----.", '9');
}

// Function to decode Morse code
char decodeMorse(const String& morse) {
  MorseNode* node = root;
  for (char c : morse) {
    if (c == '.' && node->dot) node = node->dot;
    else if (c == '-' && node->dash) node = node->dash;
    else return '?';  // Unknown character
  }
  return node->letter ? node->letter : '?';
}

// Webpage Code (Updated)
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Dancing+Script&family=Italiana&display=swap" rel="stylesheet">
    <title>Ranjan ka morse translator</title>
    <style>
        body {
            font-family: "Italiana", sans-serif;
            text-align: center;
            background: linear-gradient(180deg, #13001C, #2A0028);
            color: white;
            margin: 0;
            padding: 20px;
            overflow:hidden;
        }
        .content{
            display: grid;
            place-items: center;
            align-items: stretch;
            height: 82vh;
            align-content: center;
            justify-content: center;
            justify-items: center;
        }
        #decodedText {
            font-size: 24px;
        }
        .morse-input {
            font-size: 18px;
            color: #00ff6d;
            padding: 10px;
        }
    </style>
</head>
<body>
    <h1>Morse Code Decoder</h1>
    <div class = "content">
    <p class="morse-input">Morse input will appear here: <span id="morseInput">...</span></p>
    <p id="decodedText"><b>Decoded text will appear here: </b></p>
    </div>
    <script>
        function updateDecodedText(text) {
            document.getElementById("decodedText").innerText = text;
        }

        function updateMorseInput(morse) {
            document.getElementById("morseInput").innerText = morse;
        }

        setInterval(() => {
            fetch("/get_text")
                .then(response => response.text())
                .then(data => updateDecodedText(data));

            fetch("/get_morse")
                .then(response => response.text())
                .then(data => updateMorseInput(data));
        }, 500);
    </script>
</body>
</html>
)rawliteral";

// Web server function to serve the web page
void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

// Function to send decoded text
void handleDecodedText() {
  server.send(200, "text/plain", decodedText);
}

// Function to send Morse input
void handleMorseInput() {
  server.send(200, "text/plain", morseInput);
}

void setup() {
  Serial.begin(115200);

  // Configure button pins
  pinMode(DOT_BUTTON, INPUT_PULLUP);
  pinMode(DASH_BUTTON, INPUT_PULLUP);
  pinMode(BACKSPACE_BUTTON, INPUT_PULLUP);
  pinMode(SPACE_BUTTON, INPUT_PULLUP);

  // Build Morse tree
  buildMorseTree();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/get_text", handleDecodedText);
  server.on("/get_morse", handleMorseInput);
  server.begin();

  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();

  if (digitalRead(DOT_BUTTON) == LOW) {
    spacePressed = false;
    morseInput += '.';
    Serial.println("Dot Pressed: " + morseInput);
    delay(200);
  }

  if (digitalRead(DASH_BUTTON) == LOW) {
    spacePressed = false;
    morseInput += '-';
    Serial.println("Dash Pressed: " + morseInput);
    delay(200);
  }

  if (digitalRead(BACKSPACE_BUTTON) == LOW) {
    spacePressed = false;
    if (!morseInput.isEmpty()) {
      morseInput.remove(morseInput.length() - 1);
      Serial.println("Backspace Pressed: " + morseInput);
    }
    delay(200);
  }

  if (digitalRead(SPACE_BUTTON) == LOW) {
    delay(200);  // Debounce delay
    if (spacePressed) {
      decodedText += " ";
      Serial.println("Space Added!");
    } else {
      char decodedChar = decodeMorse(morseInput);
      if (decodedChar != '?') {
        decodedText += decodedChar;
      }
      Serial.print("Decoded Letter: ");
      Serial.println(decodedChar);
    }
    spacePressed = true;
    morseInput = "";
    delay(500);
  }
}
