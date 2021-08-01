/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

//This example shows how to download file from Firebase Storage bucket.

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "XT_DAC_Audio.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Embedded Systems Class"
#define WIFI_PASSWORD "embedded1234"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDj1lxvVdFudyfiw4q_ckde5CyTdYUQVX0"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "joegocod3@gmail.com"
#define USER_PASSWORD "Password"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "speechdeck-17a61.appspot.com"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

XT_DAC_Audio_Class DacAudio(25,0);
XT_Wav_Class *Stinky;

bool taskCompleted = false;
unsigned char* pBuffer;
bool loadedSound = false;
uint32_t DemoCounter=0;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
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

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    
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

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

#if defined(ESP8266)
    //required
    fbdo.setBSSLBufferSize(1024, 1024);
#endif

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

#if defined(ESP32)
        Firebase.sdBegin(13, 5, 19, 18); //SS, SCK,MISO, MOSI
#elif defined(ESP8266)
        Firebase.sdBegin(15); //SS
#endif
        //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        Serial.printf(
          "Download file... %s\n", 
          Firebase.Storage.download(
            &fbdo, 
            STORAGE_BUCKET_ID /* Firebase Storage bucket id */, 
            "stinky.wav" /* path of remote file stored in the bucket */, 
            "/stinky.wav" /* path to local file */, 
            mem_storage_type_sd /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */
          ) ? "ok" : fbdo.errorReason().c_str()
        );
        listDir(SD, "/", 0);
    
    } else if (taskCompleted && !loadedSound) {
      File myFile = SD.open("/stinky.wav");        // Open file for reading.
      if (myFile) {
//          myFile.seek(24); //skip the header info
//          unsigned int SAMPLE_RATE = myFile.read();
//          SAMPLE_RATE |= myFile.read() << 8;
//          Serial.println(SAMPLE_RATE);
        
          uint32_t fileSize = myFile.size();  // Get the file size.
          pBuffer = (unsigned char*)malloc(fileSize + 1);  // Allocate memory for the file and a terminating null char.
          myFile.read(pBuffer, fileSize);         // Read the file into the buffer.
          pBuffer[fileSize] = '\0';               // Add the terminating null char.
          Serial.println(fileSize);
          Stinky = new XT_Wav_Class(pBuffer);
          myFile.close();                         // Close the file.
          loadedSound = true;
      } else {
          Serial.println("Could not open stinky.wav");                
      }
    }
    if (loadedSound) {
      DacAudio.FillBuffer();                // Fill the sound buffer with data
      if (Stinky->Playing == false) {
        DacAudio.Play(Stinky); 
//        Serial.println(DemoCounter++);
      }
    }
}
