/******************************************************************
 This is an example for the Adafruit RA8875 Driver board for TFT displays
 ---------------> http://www.adafruit.com/products/1590
 The RA8875 is a TFT driver for up to 800x480 dotclock'd displays
 It is tested to work with displays in the Adafruit shop. Other displays
 may need timing adjustments and are not guanteed to work.

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 ******************************************************************/

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"


// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 27
#define RA8875_CS 33
#define RA8875_RESET 12
#define EEPROMLOCATION 100

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;

tsPoint_t       _tsLCDPoints[3];
tsPoint_t       _tsTSPoints[3];
tsMatrix_t      _tsMatrix;

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
int setCalibrationMatrix( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr)
{
  int  retValue = 0;

  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;

  if( matrixPtr->Divider == 0 )
  {
    retValue = -1 ;
  }
  else
  {
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

  return( retValue ) ;
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
int calibrateTSPoint( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr )
{
  int  retValue = 0 ;

  if( matrixPtr->Divider != 0 )
  {
    displayPtr->x = ( (matrixPtr->An * screenPtr->x) +
                      (matrixPtr->Bn * screenPtr->y) +
                       matrixPtr->Cn
                    ) / matrixPtr->Divider ;

    displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) +
                      (matrixPtr->En * screenPtr->y) +
                       matrixPtr->Fn
                    ) / matrixPtr->Divider ;
  }
  else
  {
    return -1;
  }

  return( retValue );
}

void setup()
{
  Serial.begin(9600);
  Serial.println("RA8875 start");

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
  tft.fillScreen(RA8875_WHITE);

  setCalibrationMatrix(&_tsLCDPoints[0], &_tsTSPoints[0], &_tsMatrix);

  // Play with PWM
  for (uint8_t i=255; i!=0; i-=5 )
  {
    tft.PWM1out(i);
    delay(10);
  }
  for (uint8_t i=0; i!=255; i+=5 )
  {
    tft.PWM1out(i);
    delay(10);
  }
  tft.PWM1out(255);

  tft.fillScreen(RA8875_RED);
  delay(500);
  tft.fillScreen(RA8875_YELLOW);
  delay(500);
  tft.fillScreen(RA8875_GREEN);
  delay(500);
  tft.fillScreen(RA8875_CYAN);
  delay(500);
  tft.fillScreen(RA8875_MAGENTA);
  delay(500);
  tft.fillScreen(RA8875_BLACK);

  // Try some GFX acceleration!
  tft.drawCircle(100, 100, 50, RA8875_BLACK);
  tft.fillCircle(100, 100, 49, RA8875_GREEN);

  tft.fillRect(11, 11, 398, 198, RA8875_BLUE);
  tft.drawRect(10, 10, 400, 200, RA8875_GREEN);
  tft.fillRoundRect(200, 10, 200, 100, 10, RA8875_RED);
  tft.drawPixel(10,10,RA8875_BLACK);
  tft.drawPixel(11,11,RA8875_BLACK);
  tft.drawLine(10, 10, 200, 100, RA8875_RED);
  tft.drawTriangle(200, 15, 250, 100, 150, 125, RA8875_BLACK);
  tft.fillTriangle(200, 16, 249, 99, 151, 124, RA8875_YELLOW);
  tft.drawEllipse(300, 100, 100, 40, RA8875_BLACK);
  tft.fillEllipse(300, 100, 98, 38, RA8875_GREEN);
  // Argument 5 (curvePart) is a 2-bit value to control each corner (select 0, 1, 2, or 3)
  tft.drawCurve(50, 100, 80, 40, 2, RA8875_BLACK);
  tft.fillCurve(50, 100, 78, 38, 2, RA8875_WHITE);

  pinMode(RA8875_INT, INPUT);
  digitalWrite(RA8875_INT, HIGH);

  tft.touchEnable(true);

  Serial.print("Status: "); Serial.println(tft.readStatus(), HEX);
  Serial.println("Waiting for touch events ...");
}

void loop()
{
  float xScale = 1024.0F/tft.width();
  float yScale = 1024.0F/tft.height();

  tsPoint_t raw;
  tsPoint_t calibrated;

  /* Wait around for touch events */
  if (! digitalRead(RA8875_INT))
  {
    if (tft.touched())
    {
      Serial.print("Touch: ");
      tft.touchRead(&tx, &ty);
      
      raw.x = tx;
      raw.y = ty;

      /* Calcuate the real X/Y position based on the calibration matrix */
      calibrateTSPoint(&calibrated, &raw, &_tsMatrix );
    
      /* Draw a single pixel at the calibrated point */
      tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_WHITE);
  
      Serial.print(tx); Serial.print(", "); Serial.println(ty);
      /* Draw a circle */
//      tft.fillCircle((uint16_t)(tx/xScale), (uint16_t)(ty/yScale), 4, RA8875_WHITE);
    }
  }
}
