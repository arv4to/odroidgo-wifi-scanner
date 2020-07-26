// 2020 Arv4to ==================== WiFi scanner for ODROID GO ========================== //


#include "odroid_go.h"
#include "WiFi.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

bool autoMode = false;
static esp_adc_cal_characteristics_t adc_chars;

#define PIN_BLUE_LED 2                //Define LED PIN for scanning indication

#define SPEAKER 25                    //Define SPEAKER PIN, needed to prevent noise

#define RESISTANCE_NUM    2           //Settings for reading battery voltage
#define DEFAULT_VREF      1100
#define NO_OF_SAMPLES     64


// ============ENCRYPT - TYPE==============//

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "Open";
    case (1):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (3):
      return "WPA2_PSK";
    case (4):
      return "WPA_WPA2_PSK";
    case (5):
      return "WPA2_ENTERPRISE";
    default:
      return "UNKOWN";
    }
  }


//============================BATTERY VOLTAGE SAMPLING=================================//

double readBatteryVoltage() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
  }
  adc_reading /= NO_OF_SAMPLES;
 
  return (double) esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 1000;
}

//=======================PRINT BATTERY VOLTAGE================================//

void showBatteryVoltage(double voltage) {
  GO.lcd.printf("Current Voltage: %1.3lf V \n", voltage);
}

//===================MENU==================//

void menu(){
  GO.lcd.clear();
  GO.lcd.setTextSize(2);                                                       
  GO.lcd.setCursor(0, 0);                                                     
  GO.lcd.println("Press A for AUTO");
  GO.lcd.println("");
  GO.lcd.println("Press B for MANUAL");  
  GO.lcd.println("");
  delay(100);
  GO.lcd.println("");
  GO.lcd.println("");
  showBatteryVoltage(readBatteryVoltage());
  delay(100);
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("");
  GO.lcd.println("Ready !");
  GO.update();
}



//===================BLINK=================//

void blink(){
  digitalWrite(PIN_BLUE_LED, LOW);
  delay(50);
  digitalWrite(PIN_BLUE_LED,HIGH);
  delay(50);
  digitalWrite(PIN_BLUE_LED, LOW);
  delay(50);
  digitalWrite(PIN_BLUE_LED, HIGH);
  delay(50);
  digitalWrite(PIN_BLUE_LED, LOW);
  return;
}

//==========================SCANNER=========================//

void scan(){
  digitalWrite(PIN_BLUE_LED, HIGH);         
  GO.lcd.clearDisplay();                    
  GO.lcd.setCursor(0, 0);
  int n = WiFi.scanNetworks();
  blink(); 
  if (n == 0) {                              
    Serial.println("No networks found");
    GO.lcd.println("No networks found");
    delay(300);
  } else {
    Serial.print(n);                   
    GO.lcd.print(n);
    Serial.println(" networks found");    
    GO.lcd.println(" networks found");
    GO.lcd.println("");                     //Spacer
    Serial.println("");
    for (int i = 0; i < n; ++i) {      
      Serial.print(i + 1);                
      GO.lcd.print(i + 1);
      Serial.print(": ");
      GO.lcd.print(": ");
      
      Serial.print(WiFi.SSID(i));         //Print SSID
      GO.lcd.print(WiFi.SSID(i));         
      Serial.print(" (");
      GO.lcd.print(" (");
      
      Serial.print(WiFi.RSSI(i));         //Print RSSI (-#dBm)
      GO.lcd.print(WiFi.RSSI(i));         
      Serial.print("dBm) ");
      GO.lcd.print("dBm) ");
      Serial.print(" [");
      GO.lcd.print(" [");
      
      Serial.print(WiFi.channel(i));     //Print Channel
      GO.lcd.print(WiFi.channel(i));     
      Serial.print("] ");
      GO.lcd.print("] ");
      
      String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));     //Print WiFi encrypt type
      Serial.println(encryptionTypeDescription);      
      GO.lcd.println(encryptionTypeDescription);
      delay(100); 
    }
  }
  return;
}

//========================SETUP===============================//


void setup()
{      
  GO.begin();                                                                                               //Startup Odroid GO platform

    
  adc1_config_width(ADC_WIDTH_BIT_12);                                                                     //Setting up ADC to read battery voltage                       
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
  
  menu();                                                                                                   //Show menu
  
  WiFi.mode(WIFI_STA);                                                                                      //Set up WiFi
  WiFi.disconnect();
  
  pinMode(PIN_BLUE_LED, OUTPUT);                                                                            //Need for turn off speaker noise
  pinMode(SPEAKER, OUTPUT);
}


//===========================LOOP=================================//


void loop(){    
  digitalWrite(SPEAKER, LOW);                                                   //Turn off speaker PIN to prevent noise (has to be in loop, it does not work in setup() for me, dunno why
  
  GO.lcd.setTextSize(1);                                                       //Setup LCD
  GO.lcd.setCursor(0, 0);  

  GO.update();                                                                 //Wait for input

  if(GO.BtnSelect.isPressed()){
    GO.lcd.setBrightness(10);
  }
  if(GO.BtnStart.isPressed()){
    GO.lcd.setBrightness(250);
  }

//=======================DEFAULT======================//

  if(autoMode){
    
    scan();                                                                   //Run scanner
    
    GO.update();                                                              //Wait for input
    
    delay(4000);
  }


//====================AUTO=========================//
  
  if(GO.BtnA.isPressed() && autoMode){
    
    scan();
    
    GO.update();
    
    delay(4000);
  }
  if(GO.BtnA.isPressed() && !autoMode){
    
    autoMode = true;                                                        //Switch bool autoMode to "true" for enabling autoMode
    
    scan();
    
    GO.update();
    
    delay(4000);
  }


//====================MANUAL===================//

  if(GO.BtnB.isPressed() && autoMode){
    
    autoMode = false;                                                     //Switch bool autoMode to "false" for disabling autoMode
    
    menu();
    
  }
  
  if(GO.BtnB.isPressed() && !autoMode){
    
    scan();
    
    GO.update();
  }


//==============RETURN TO MENU===========//
  
  if(GO.BtnMenu.isPressed()){
    
    autoMode = false;
    
    menu();
  }
}
