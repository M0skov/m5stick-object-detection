########################################
#LAB 5 final lab
#Jan Israel Charrez Gomez
#Professor : Kyle Jhonson

########################################
#NOw final steps toward the api and m5 connection
import asyncio
import cv2 
import os
from google import genai

# Set your own key before running:
#   export GEMINI_API_KEY="your-key-here"   (macOS / Linux)
#   setx GEMINI_API_KEY "your-key-here"      (Windows)
api_key = os.environ.get("GEMINI_API_KEY")
if not api_key:
    raise SystemExit("Missing GEMINI_API_KEY environment variable.")

client_ai = genai.Client(api_key=api_key)

##import time
from bleak import BleakClient, BleakScanner
###Im still working on it or in this uuid 
##the idea i have is one delivers pc to m5 and
## the communication m5 to pc 
##im not sure if im doing this correctly 
## but so far i undertand that UUID are like paths? 
##in conclusion
COMMUNICATION_UUID = "8fc5ff9a-33c5-42c8-b557-eabbb0f956d2"
#CHARACTERISTICS IS FOR M5 TO PC AND WE GET THE BATTERY LEVELS
CHARACTERISTIC_UUID = "7d753f58-ba8f-4833-9f9a-e36a59af7516"
#WE GET THE CONNECTION FROM PC TO M5 THE OUPUT OF GEMINI
USER_DELIVER_TO_UUID = "995ad5d6-47f2-4414-8f2a-30a11b849a60"

##another boolean swict to make sure we control the program:



async def run():
    #we scan for the device
    devices = await BleakScanner.discover(20)

    #i put this just to grap the len and not the whole data but i may change it
    print(f"Number of devices available:, {len(devices)} devices")
    
    #we declare a variable for the m5
    m5_device = None

    ##we create a list that would store all devices
    device_list= []
    ##a count of number of devices
    number_of_devices = 0
        
    for d in devices:
        ##ok i have finally able to create the user choice to connect to the m5
        
        if d.name is not None:
            ##since is running in a loop it would display the name of every device near by
            print(f"[{number_of_devices}].- - Name: {d.name}")
            ##and would append to the list above
            device_list.append(d)
            number_of_devices +=1
            ##once it reach 20 it would stop i did 20 beacause for some reason sometime it does not 
            ##apppear on the first 10 also sometime just displays 7,  or even 4 devices
        if number_of_devices == 20:
            break
    
    if not device_list:
            print("please try again")
            return
    try: 
        #if the programs list all devices the programs ask for which device to connect
        choice = int(input("please selcet the devive: "))
        m5_device = device_list[choice]
##if choice is out of bounds the program would close
    except (IndexError):
        print("Invalid Please come back later")
        return
        
    ##then if the device has been found we stablish the connection
    if m5_device:
        try:
            print(f"Please wait while we connect your device, You have selected: {m5_device.name} - Address: {d.address} ")
            ##a time of 20 seconds since sometime it takes a lot of time
            async with BleakClient(m5_device, timeout=20) as client:
                print("connected") #notification for the user :D
                ##this is very important since it did not let me use a variable of def notification_from_m5
                ##i have to create a global so it can let me use a "switch"
                command_from_m5 = None
                ##in this part we transfer all data from the M5Stick
                def notification_from_m5(sender, data):
                    ##we recall the variable
                    nonlocal command_from_m5

                    from struct import unpack
                    try:
                        ##and we get the battery 
                        ##i put h before because we did the previous lab as <h
                        #until it told me that the whole package is 6 byte
                        battery_mv, command = unpack('<H6s', data)
                        ##and the most important thing the controls
                        ##we need to clean the the "null bytes" other wise we cannot contnue 
                        ##in other words raw bytes into readable data 
                        command_from_m5 = command.decode('utf-8').rstrip('\x00')
                        ##and we print it every time i puhsh A Or B Btn
                        print(f'm5 to pc instructions: Battery:{battery_mv} mV - command: {command_from_m5}')
                    #if any error we do display it    
                    except Exception as e:
                        print(f"Error {e}")
                ##now in his part is where we send the ouput of gemini to M5 stic display
                await client.start_notify(CHARACTERISTIC_UUID, notification_from_m5)
                ##
                print("waiting for user command (M5 button take pic)(upper-side right btn quit)")
                #we set the camera of the computer
                cap = cv2.VideoCapture(0)
                while cap.isOpened():
                    res, frame = cap.read()
                    if not res:
                        continue
                    cv2.imshow("my window", frame)
                    key = cv2.waitKey(1)
                    ##and this is important the previous data we export now we check what those values are i will skip all this
                    #since is simple
                    if command_from_m5 == "q":
                        break
                    elif command_from_m5 == "c":
                        cv2.imwrite("test.png", frame)
                        myfile = client_ai.files.upload(file = "test.png")
                        response = client_ai.models.generate_content(model="gemini-2.0-flash",
                        contents=["if you see a person: identify the object that is holding try to determine the following"
                        "country of origin, brand, price, where you can get it (guess) "
                        "condition of the object you should have 7 labels- object - brand -country- condition -price- -location. "
                        "(keep short just labels) if you cannot identify an"
                        " object do not write and just output the following - GET OUT >:( - thats it please and Thank you", myfile])
                        result_text = response.text
                        
                        
                        await client.write_gatt_char(USER_DELIVER_TO_UUID, result_text.encode())
            
                        print("message has sent")
                        command_from_m5 = None

                    await asyncio.sleep(0.01)
                cap.release()
                #when finish we close all
                cv2.destroyAllWindows()
                print("\nyeiiiii disconecting byyyeeee :D")
        except Exception as e:
            print(f"connection error: {e}")
    else:
        print("device not found")
asyncio.run(run())
