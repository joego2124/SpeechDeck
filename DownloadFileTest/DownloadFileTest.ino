#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "XT_DAC_Audio.h"

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#include <Wire.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Embedded Systems Class"
#define WIFI_PASSWORD "embedded1234"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCHuNh5r-qJi9y5Aa21nPYaoJ0XPHLAuvA"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "joegocod3@gmail.com"
#define USER_PASSWORD "Password"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "speechdeck-45f5b.appspot.com"

#define DATABASE_URL "speechdeck-45f5b-default-rtdb.firebaseio.com"

// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 27
#define RA8875_CS 33
#define RA8875_RESET 12
#define EEPROMLOCATION 100

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

XT_DAC_Audio_Class DacAudio(25, 0);
XT_Wav_Class *Stinky;

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;

tsPoint_t       _tsLCDPoints[3];
tsPoint_t       _tsTSPoints[3];
tsMatrix_t      _tsMatrix;

bool taskCompleted = false;
unsigned char* pBuffer;
bool loadedSound = false;
uint32_t DemoCounter = 0;

String userId = "WS1m2aFK8lO0IhBlGQGbOUwYfW62";

/**************************************************************************/
/*!
    @brief Calculates the difference between the touch screen and the
           actual screen co-ordinates, taking into account misalignment
           and any physical offset of the touch screen.
    @note  This is based on the public domain touch screen calibration code
           written by Carlos E. Vidales (copyright (c) 2001).
           For more information, see the following app notes:
           - AN2173 - Touch Screen Control and Calibration
             Svyatoslav Paliy, Cypress Microsystems
           - Calibration in touch-screen systems
             Wendy Fang and Tony Chang,
             Analog Applications Journal, 3Q 2007 (Texas Instruments)
*/
/**************************************************************************/
int setCalibrationMatrix( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr) {
  int  retValue = 0;

  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;

  if ( matrixPtr->Divider == 0 ) {
    retValue = -1 ;
  } else {
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;

    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;

    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;

    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;

    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;

    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }

  return ( retValue ) ;
}

/**************************************************************************/
/*!
    @brief  Converts raw touch screen locations (screenPtr) into actual
            pixel locations on the display (displayPtr) using the
            supplied matrix.
    @param[out] displayPtr  Pointer to the tsPoint_t object that will hold
                            the compensated pixel location on the display
    @param[in]  screenPtr   Pointer to the tsPoint_t object that contains the
                            raw touch screen co-ordinates (before the
                            calibration calculations are made)
    @param[in]  matrixPtr   Pointer to the calibration matrix coefficients
                            used during the calibration process (calculated
                            via the tsCalibrate() helper function)
    @note  This is based on the public domain touch screen calibration code
           written by Carlos E. Vidales (copyright (c) 2001).
*/
/**************************************************************************/
int calibrateTSPoint( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr ) {
  int  retValue = 0 ;

  if ( matrixPtr->Divider != 0 ) {
    displayPtr->x = ( (matrixPtr->An * screenPtr->x) +
                      (matrixPtr->Bn * screenPtr->y) +
                      matrixPtr->Cn
                    ) / matrixPtr->Divider ;

    displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) +
                      (matrixPtr->En * screenPtr->y) +
                      matrixPtr->Fn
                    ) / matrixPtr->Divider ;
  } else {
    return -1;
  }

  return ( retValue );
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

#define BUFFPIXEL 40

void bmpDraw(const char *filename, int x, int y) {
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;

  if ((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == false) {
    Serial.println(F("File not found"));
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.println(F("File size: "));
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: "));
    Serial.println(bmpImageoffset, DEC);

    // Read DIB header
    Serial.print(F("Header size: "));
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);

    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: "));
      Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds

        for (row = 0; row < h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if (lcdidx > 0) {
                tft.drawPixel(col + x, row + y, lcdbuffer[lcdidx]);
                lcdidx = 0;
                first  = false;
              }

              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx] = color565(r, g, b);
            tft.drawPixel(col + x, row + y, lcdbuffer[lcdidx]);
          } // end pixel

        } // end scanline

        // Write any remaining data to LCD
        if (lcdidx > 0) {
          tft.drawPixel(col + x, row + y, lcdbuffer[lcdidx]);
        }

        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");

      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));

}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

byte decToBcd(byte val) {
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

bool buttonsRendered = false;

void renderButtons() {
  for (int i = 0; i < 3; i++)  {
    // for (int r = 0; r < 1; r++)  {
    //  for (int c = 0; c < 1; c++)  {
    char path[100];
    char buf[50];
    strcpy(path, "/");
    String(i).toCharArray(buf, 50);
    strcat(path, buf);
    strcat(path, "/image.bmp");
    Serial.println(path);
    //    bmpDraw("/icon.bmp", r * 160, c * 160);
    bmpDraw(path, i * 160, 0);
    //  }
  }
  buttonsRendered = true;
}

XT_Wav_Class *sounds[15];

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  /* Initialize the display using 'RA8875_480x80', 'RA8875_480x128', 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }
  Serial.println("Found RA8875");

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  //Calibration
  _tsLCDPoints[0].x = tft.width() / 10;
  _tsLCDPoints[0].y = tft.height() / 10;
  _tsLCDPoints[1].x = tft.width() / 2;
  _tsLCDPoints[1].y = tft.height() - tft.height() / 10;
  _tsLCDPoints[2].x = tft.width() - tft.width() / 10;
  _tsLCDPoints[2].y = tft.height() / 2;

  _tsTSPoints[0].x = 148;
  _tsTSPoints[0].y = 190;
  _tsTSPoints[1].x = 516;
  _tsTSPoints[1].y = 867;
  _tsTSPoints[2].x = 893;
  _tsTSPoints[2].y = 535;

  // With hardware accelleration this is instant
  tft.graphicsMode();
  tft.fillScreen(RA8875_WHITE);
  tft.graphicsMode();

  setCalibrationMatrix(&_tsLCDPoints[0], &_tsTSPoints[0], &_tsMatrix);

  pinMode(RA8875_INT, INPUT);
  digitalWrite(RA8875_INT, HIGH);

  tft.touchEnable(true);

  Serial.print("Status: "); Serial.println(tft.readStatus(), HEX);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
}

bool playing = false;
bool layoutLoaded = false;
uint8_t currentLayoutId = 0;
String layout[] = {
  "stinky", "hurt", "monkey", "stinky", "stinky",
  "stinky", "stinky", "stinky", "stinky", "stinky",
  "stinky", "stinky", "stinky", "stinky", "stinky"
};

void loop()
{
  float xScale = 1024.0F / tft.width();
  float yScale = 1024.0F / tft.height();

  tsPoint_t raw;
  tsPoint_t calibrated;

  /* Wait around for touch events */
  if (! digitalRead(RA8875_INT)) {
    if (tft.touched() && playing != true) {
      Serial.print("Touch: ");
      tft.touchRead(&tx, &ty);

      raw.x = tx;
      raw.y = ty;

      /* Calcuate the real X/Y position based on the calibration matrix */
      calibrateTSPoint(&calibrated, &raw, &_tsMatrix );

      if (calibrated.x < 160) {
        currentLayoutId = 0;
      } else if (calibrated.x > 160 && calibrated.x < 320) {
        currentLayoutId = 1;
      } else if (calibrated.x > 320) {
        currentLayoutId = 2;
      }

      /* Draw a single pixel at the calibrated point */
      //        tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_BLACK);
      if (!playing) playing = true;

      Serial.print(tx); Serial.print(", "); Serial.println(ty);
    }
  }

  if (Firebase.ready() && !layoutLoaded) {
    FirebaseJsonArray databaseLayoutArr;

    char remotePath[100];
    char buf[50];

    userId.toCharArray(buf, 50);
    strcpy(remotePath, buf);
    strcat(remotePath, "/Layout");

    if (Firebase.RTDB.get(&fbdo, remotePath, databaseLayoutArr)) {
      for (int i = 0; i < 3; i ++) {
        FirebaseJsonData jsonData;
        databaseLayoutArr.get(jsonData, i);
        Serial.println(jsonData.intValue);
      }
      layoutLoaded = true;
    } else {
      Serial.println("RTDB error");
    }
  }

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;
    Firebase.sdBegin(15, 5, 19, 18); //SS, SCK,MISO, MOSI

    for (int i = 0; i < 3; i++) {
      char remotePath[100];
      char localPath[100];
      char buf[50];

      userId.toCharArray(buf, 50);
      strcpy(remotePath, buf);
      strcat(remotePath, "/");
      layout[i].toCharArray(buf, 50);
      strcat(remotePath, buf);
      strcat(remotePath, "/audio.wav");

      strcpy(localPath, "/");
      String(i).toCharArray(buf, 50);
      strcat(localPath, buf);
      createDir(SD, localPath);
      strcat(localPath, "/audio.wav");

      Serial.println(remotePath);
      Serial.println(localPath);

      Serial.printf(
        "Downloading file... %s\n",
        Firebase.Storage.download(
          &fbdo,
          STORAGE_BUCKET_ID /* Firebase Storage bucket id */,
          remotePath /* path of remote file stored in the bucket */,
          localPath /* path to local file */,
          mem_storage_type_sd /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */
        ) ? "ok" : fbdo.errorReason().c_str()
      );
    }

    for (int i = 0; i < 3; i++) {
      char remotePath[100];
      char localPath[100];
      char buf[50];

      userId.toCharArray(buf, 50);
      strcpy(remotePath, buf);
      strcat(remotePath, "/");
      layout[i].toCharArray(buf, 50);
      strcat(remotePath, buf);
      strcat(remotePath, "/image.bmp");

      strcpy(localPath, "/");
      String(i).toCharArray(buf, 50);
      strcat(localPath, buf);
      strcat(localPath, "/image.bmp");

      Serial.println(remotePath);
      Serial.println(localPath);

      Serial.printf(
        "Downloading file... %s\n",
        Firebase.Storage.download(
          &fbdo,
          STORAGE_BUCKET_ID /* Firebase Storage bucket id */,
          remotePath /* path of remote file stored in the bucket */,
          localPath /* path to local file */,
          mem_storage_type_sd /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */
        ) ? "ok" : fbdo.errorReason().c_str()
      );
    }
    listDir(SD, "/", 0);

  } else if (taskCompleted) {
    if (!buttonsRendered) {
      renderButtons();
    }
    if (!loadedSound) {
      for (int i = 0; i < 3; i++) {
        char path[100];
        char buf[50];
        strcpy(path, "/");
        String(i).toCharArray(buf, 50);
        strcat(path, buf);
        strcat(path, "/audio.wav");
        Serial.println(path);
        File myFile = SD.open(path);        // Open file for reading.
        if (myFile) {
          uint32_t fileSize = myFile.size();  // Get the file size.
          pBuffer = (unsigned char*)malloc(fileSize + 1);  // Allocate memory for the file and a terminating null char.
          myFile.read(pBuffer, fileSize);         // Read the file into the buffer.
          pBuffer[fileSize] = '\0';               // Add the terminating null char.
          sounds[i] = new XT_Wav_Class(pBuffer);
          myFile.close();                         // Close the file.
        } else {
          Serial.println("Could not open stinky.wav");
        }
      }
      loadedSound = true;
    }
  }

  if (loadedSound) {
    DacAudio.FillBuffer();                // Fill the sound buffer with data
    if (sounds[currentLayoutId]->Playing == false && playing) {
      playing = false;
      DacAudio.Play(sounds[currentLayoutId]);
      Serial.println(DemoCounter++);
    }
  }
}
