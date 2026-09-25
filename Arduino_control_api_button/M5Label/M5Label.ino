/////////////////////////////////////////////////////////////
//Jan Gomez
//Proffesor kyle Jhonsen 
//Lab 5  arduino 
//IM going to make a quick conclusion here basically all my
//system is controlled by bool commands
//it works like swiches and easy to make it fluent

//////////////////////////////////////////////////////////////////////////
///here are the modules
//the library and the server
//for the bluetooth connection basically 
// the code we were given in the class
#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "esp_gap_ble_api.h"
#include <M5GFX.h>

//this COMMUNICATION is our "MAIN CHANNEL"
#define COMMUNICATION_UUID "8fc5ff9a-33c5-42c8-b557-eabbb0f956d2"
#define CHARACTERISTIC_UUID "7d753f58-ba8f-4833-9f9a-e36a59af7516"
#define USER_DELIVER_TO_UUID "995ad5d6-47f2-4414-8f2a-30a11b849a60"

M5Canvas canvas(&M5.Display);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool advertising = false;
//bool off = false;
String message_from_computer="";
//tryin to get only the battery values
#pragma pack(1)
typedef struct{
  uint16_t batt;
  char instructions[6];
} Packet;
Packet p;

//this is from my computer to the M5 using onWrite function that allow the 
//M5 to receive the messages from the computer
class ComputerTOMC : public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *pCharacteristic){
    //this basically transform the inputs from python to conver the string to arduino type string
    message_from_computer = String(pCharacteristic->getValue().c_str());
    //I will skip all design (CANVAS) since we already learn it or did it before
    canvas.setRotation(1);
    canvas.fillScreen(BLACK);
    canvas.setCursor(10,10);
    canvas.setTextSize(1);
    canvas.setTextColor(WHITE);
    canvas.println("Incoming text");
    canvas.println(message_from_computer);
    canvas.pushSprite(0,0);
  }
};
// this is only the server from M5 to Python or my computer im still
//working on it 
class MyCallbacks : public BLEServerCallbacks{
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param){
    canvas.setRotation(1);
    canvas.fillScreen(BLACK);
    canvas.setCursor(10,10);
    canvas.setTextSize(1);
    canvas.setTextColor(WHITE);
    canvas.println("Device has been found");
    canvas.pushSprite(0,0);
    deviceConnected = true;
    advertising = false;
    delay(1000);
  }
  void onDisconnect(BLEServer* pServer){
    canvas.setRotation(1);
    canvas.fillScreen(BLACK);
    canvas.setCursor(10,10);
    canvas.setTextSize(1);
    canvas.setTextColor(WHITE);
    canvas.println("Device not found");
    canvas.pushSprite(0,0);
    deviceConnected = false;
    advertising = true;
    //delay(500);
    //this helps the m5stick if it looses connection to immediately 
    //try and pick up the signal
    pServer->startAdvertising();
  }
};

void setup(){
  //ignore this im just rying to make it look fancy :D
  M5.begin();

  
  canvas.createSprite(M5.Display.width(),M5.Display.height()); 
  canvas.setRotation(1);
  canvas.fillScreen(BLACK);
  canvas.setCursor(10,10);
  canvas.setTextSize(2);
  canvas.setTextColor(WHITE);
  canvas.println("Welcome please wait while we detect your device");
  canvas.pushSprite(0,0);
  BLEDevice::init("M5StickCPlusJanG");
  //server is created
  pServer = BLEDevice::createServer();
  //Mycallbacks is "invoke" to let the user known if the device has been connect
  //to the m5 is scytablish and data is stored
  ///IMPORTANT THIS SEND THE CHRACTERS EITHER Q OR C TO PYTHON
  pServer->setCallbacks(new MyCallbacks());
  BLEService* pService = pServer->createService(COMMUNICATION_UUID);
  //here is where the data gets push or deliver spacially 
  //for the data of the Battery of the M5Stick
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  //this allow the notifications to communicate to each other
  pCharacteristic->addDescriptor(new BLE2902());

  //THIS PART RECEIVES ANY INPUT FROM THE COMPUTER
  //it would be the same steps except there is mall catch
  //instead of sending we let arduino that is also going to send data to the client
  BLECharacteristic* pReceiveChar = pService->createCharacteristic(
  USER_DELIVER_TO_UUID,
  BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pReceiveChar->setCallbacks(new ComputerTOMC());
  pReceiveChar->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(COMMUNICATION_UUID);

  BLEDevice::startAdvertising();
  //canvas.setRotation(1);
  canvas.fillScreen(BLACK);
  canvas.setCursor(10,10);
  canvas.setTextSize(1);
  canvas.setTextColor(WHITE);
  canvas.println("Waiting for data");
  canvas.pushSprite(0,0);
}
void loop(){
  M5.update();
  //M5.Display.setRotation(1);
  //Here are the main controls to take pic and quit the program 
  bool off = false;
  if (deviceConnected && !off){
    bool next_action = false;
    if(M5.BtnA.wasPressed()){
    
      strcpy(p.instructions, "c");
      next_action = true;
    }
    if(M5.BtnB.wasPressed()){
      strcpy(p.instructions, "q");
      next_action = true;
      off = true;
    }
    p.batt = M5.Power.getBatteryVoltage() * 1000;
    if (next_action){
      pCharacteristic->setValue((uint8_t*)&p, sizeof(Packet));
      pCharacteristic->notify();
    }
    
    delay(10);
  }
  //if disconected would not do anything 
  if(!deviceConnected && !advertising){
    BLEDevice::startAdvertising();
    canvas.setRotation(1);
    canvas.fillScreen(BLACK);
    canvas.setCursor(10,10);
    canvas.setTextSize(1);
    canvas.setTextColor(WHITE);
    canvas.println("Please run the program or connect to a device");
    canvas.pushSprite(0,0);
    advertising = false;
  
  }
}
