#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>
#include <ESP8266WiFi.h>

#define NUM_LEDS 300
#define LED_PIN 3
#define SOUND_PIN 0
#define brightness 255

const char *ssid = "silentlad";
const char *password = "Silentlad1!";

WiFiServer server(1234);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
String header;

// Decode HTTP GET value
String redString = "0";
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

const int sampleWindow = 10; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

float sensorvalue = 0, lastmaxsensorvalue = 0, lastminsensorvalue = 1024.0;
int curshow = NUM_LEDS;
float val;

int findLEDNum(int len, float soundMin, float soundMax, float sound);

int loopCount = 0;

float fade_scale = 1.3;

float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);
void FillLEDsFromPaletteColors(uint8_t colorIndex, int curshow, int flag);
void RunBeats(int x);
int leftOver = 0;
int seperator = 0;
void RunBeats2(int x);
int curshowArray[20] = {0};

void insertCurshow(int x)
{
    int i;
    for (i = 0; i < 19; i++)
    {
        curshowArray[i + 1] = curshowArray[i];
    }
    curshowArray[0] = x;
}
int averageCurshow()
{
    int i, sum = 0;
    for (i = 0; i < 20; i++)
    {
        sum += curshowArray[i];
    }
    int res = sum / 20;
    return res;
}

CRGBPalette16 currentPalette;
TBlendType currentBlending;

CRGB currentColor;
CRGB leds[NUM_LEDS];

void setup()
{
    Serial.begin(9600);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    // put your setup code here, to run once:
    FastLED.addLeds<WS2812B, LED_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = CRGB(0, 0, 255);
    currentPalette = PartyColors_p;
    currentBlending = LINEARBLEND;
    FastLED.show();
    delay(1000);
}

void loop()
{
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    { // If a new client connects,
        currentTime = millis();
        previousTime = currentTime;
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected() && currentTime - previousTime <= timeoutTime)
        { // loop while the client's connected
            currentTime = millis();
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
                        client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
                        client.println("</head><body><div class=\"container\"><div class=\"row\"><h1>ESP Color Picker</h1></div>");
                        client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> ");
                        client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
                        client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
                        client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script></body></html>");
                        // The HTTP response ends with another blank line
                        client.println();

                        // Request sample: /?r201g32b255&
                        // Red = 201 | Green = 32 | Blue = 255
                        if (header.indexOf("GET /?r") >= 0)
                        {
                            pos1 = header.indexOf('r');
                            pos2 = header.indexOf('g');
                            pos3 = header.indexOf('b');
                            pos4 = header.indexOf('&');
                            redString = header.substring(pos1 + 1, pos2);
                            greenString = header.substring(pos2 + 1, pos3);
                            blueString = header.substring(pos3 + 1, pos4);
                            Serial.println(redString.toInt());
                            Serial.println(greenString.toInt());
                            Serial.println(blueString.toInt());
                        }
                        // Break out of the while loop
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }

    unsigned long startMillis = millis(); // Start of sample window
    unsigned int peakToPeak = 0;          // peak-to-peak level

    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow)
    {
        sample = analogRead(SOUND_PIN);
        if (sample < 1024) // toss out spurious readings
        {
            if (sample > signalMax)
            {
                signalMax = sample; // save just the max levels
            }
            else if (sample < signalMin)
            {
                signalMin = sample; // save just the min levels
            }
        }
    }
    peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude

    static uint8_t startIndex = 0;
    sensorvalue = peakToPeak;
    curshow = fscale(15, 645, 0, NUM_LEDS, sensorvalue, 2);
    startIndex = startIndex + 1; /* motion speed */
                                 //  FillLEDsFromPaletteCoitlors( startIndex,curshow);
                                 //  curshow = findLEDNum(NUM_LEDS,lastminsensorvalue,lastmaxsensorvalue,sensorvalue);
                                 //  curshow = abs(curshow);
                                 //  Serial.print(sensorvalue);
                                 //  Serial.print(" ");
    Serial.println(curshow);
    //  FillLEDsFromPaletteColors( startIndex,curshow,0);
    // FillLEDsFromPaletteColors(startIndex, curshow, 1);
    //  FillLEDsFromPaletteColors( startIndex,curshow,2);
    //  FillLEDsFromPaletteColors( startIndex,curshow,4);
    //  RunBeats(peakToPeak);
    //  RunBeats2(peakToPeak);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB(redString.toInt(), greenString.toInt(), blueString.toInt());
    }
    FastLED.show();
    loopCount++;
    //  delay(50);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, int curshow, int flag)
{
    if (flag == 0)
    {
        //    uint8_t brightness = 50;

        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (i <= curshow)
            {
                leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                colorIndex += 1;
            }
            else
            {
                leds[i] = CRGB(leds[i].r / fade_scale, leds[i].g / fade_scale, leds[i].b / fade_scale);
            }
        }
    }
    else if (flag == 1)
    {
        //    uint8_t brightness = 255;

        for (int i = NUM_LEDS / 2 - 1, j = NUM_LEDS / 2; i >= 0 && j < NUM_LEDS; i--, j++)
        {

            if (NUM_LEDS / 2 - i <= curshow / 2)
            {
                leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                leds[j] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                colorIndex += 1;
            }
            else
            {
                leds[i] = CRGB(leds[i].r / fade_scale, leds[i].g / fade_scale, leds[i].b / fade_scale);
                leds[j] = CRGB(leds[j].r / fade_scale, leds[j].g / fade_scale, leds[j].b / fade_scale);
            }
        }
    }
    else if (flag == 2)
    {
        for (int i = NUM_LEDS / 2 - 1, j = NUM_LEDS / 2; i >= 0 && j < NUM_LEDS; i--, j++)
        {

            if (NUM_LEDS / 2 - i <= curshow / 2)
            {
                leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                leds[j] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                colorIndex += 1;
            }
            else if (NUM_LEDS / 2 - i <= curshow / 2 + NUM_LEDS / 4)
            {
                leds[i] = CRGB(0, 200, 100);
                leds[j] = CRGB(0, 200, 100);
            }
            else
            {
                leds[i] = CRGB(200, 200, 0);
                leds[j] = CRGB(200, 200, 0);
            }
        }
    }
    else if (flag == 3)
    {
        insertCurshow(curshow);
        int average = averageCurshow();
        Serial.print(" ");
        Serial.println(average);
        for (int i = NUM_LEDS / 2 - 1, j = NUM_LEDS / 2; i >= 0 && j < NUM_LEDS; i--, j++)
        {

            if (NUM_LEDS / 2 - i <= average / 2)
            {
                leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                leds[j] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                colorIndex += 1;
            }
            else
            {
                leds[i] = CRGB(leds[i].r / fade_scale, leds[i].g / fade_scale, leds[i].b / fade_scale);
                leds[j] = CRGB(leds[j].r / fade_scale, leds[j].g / fade_scale, leds[j].b / fade_scale);
            }
        }
    }
    else if (flag == 4)
    {
        insertCurshow(curshow);
        int average = averageCurshow();
        for (int i = NUM_LEDS / 2 - 1, j = NUM_LEDS / 2; i >= 0 && j < NUM_LEDS; i--, j++)
        {

            if (NUM_LEDS / 2 - i <= average / 2)
            {
                leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                leds[j] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
                colorIndex += 1;
            }
            else if (NUM_LEDS / 2 - i <= average / 2 + NUM_LEDS / 4)
            {
                leds[i] = CRGB(0, 200, 100);
                leds[j] = CRGB(0, 200, 100);
            }
            else
            {
                leds[i] = CRGB(200, 200, 0);
                leds[j] = CRGB(200, 200, 0);
            }
        }
    }
}

void RunBeats(int x)
{

    if (x > 15)
    {
        float mappedSound = map(15, 645, 0, 15, x);
        leds[0] = ColorFromPalette(currentPalette, (int)mappedSound, brightness, currentBlending);
    }
    else
    {
        leds[0] = CRGB(0, 0, 0);
    }

    for (int z = NUM_LEDS; z > 0; z--)
    {
        leds[z] = leds[z - 1];
    }
}

void RunBeats2(int x)
{
    //  uint8_t brightness = 255;
    //  int x = analogRead(SOUND_PIN);
    if (x < 50)
    {
        leds[0] = CRGB(0, 0, 0);
    }
    else if (x > 100 && x <= 200)
    {
        leds[0] = CRGB(255, 154, 0);
    }
    else if (x > 300 && x <= 350)
    {
        leds[0] = CRGB(255, 255, 0);
    }
    else if (x > 350 && x <= 400)
    {
        leds[0] = CRGB(0, 255, 0);
    }
    else if (x > 450 && x <= 500)
    {
        leds[0] = CRGB(0, 0, 255);
    }
    else if (x > 550 && x <= 600)
    {
        leds[0] = CRGB(150, 102, 255);
    }
    else
    {
        leds[0] = CRGB(255, 0, 255);
    }
    for (int z = NUM_LEDS; z > 0; z--)
    {
        leds[z] = leds[z - 1];
    }
}

float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve)
{

    float OriginalRange = 0;
    float NewRange = 0;
    float zeroRefCurVal = 0;
    float normalizedCurVal = 0;
    float rangedValue = 0;
    boolean invFlag = 0;

    // condition curve parameter
    // limit range

    if (curve > 10)
        curve = 10;
    if (curve < -10)
        curve = -10;

    curve = (curve * -.1);  // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
    curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

    /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
   Serial.println(); 
   */

    // Check for out of range inputValues
    if (inputValue < originalMin)
    {
        inputValue = originalMin;
    }
    if (inputValue > originalMax)
    {
        inputValue = originalMax;
    }

    // Zero Refference the values
    OriginalRange = originalMax - originalMin;

    if (newEnd > newBegin)
    {
        NewRange = newEnd - newBegin;
    }
    else
    {
        NewRange = newBegin - newEnd;
        invFlag = 1;
    }

    zeroRefCurVal = inputValue - originalMin;
    normalizedCurVal = zeroRefCurVal / OriginalRange; // normalize to 0 - 1 float

    // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
    if (originalMin > originalMax)
    {
        return 0;
    }

    if (invFlag == 0)
    {
        rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;
    }
    else // invert the ranges
    {
        rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
    }

    return rangedValue;
}