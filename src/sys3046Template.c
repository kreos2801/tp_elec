/**
 * @file sys3046Template.c
 * @brief my short description for sys3046Template.c
 */   
/**
 * @file RFID_Security_System.c
 * @brief RFID-based access control system with alarm capabilities
 */

//==========================================================
// Main header includes and module definitions
//==========================================================
#include <SPI.h>
#include <MFRC522.h>
#include "main.h"
#include <list>
#include <string>
#include "../lib/STM32RTC-1.7.0/src/STM32RTC.h"

/**
 * @brief Defines the pins for the RFID reader module
 */
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

/**
 * @brief List to store all registered users
 */
std::list<User> users;

/**
 * @brief Current state of the alarm system
 */
ALARM_STATE alarmState = OFF;

/**
 * @brief Pin definitions and global variables
 */
constexpr int alarm_led_pin = D5;  // LED to indicate alarm status
unsigned long RFIDTimeMemory = 0;  // Timer for RFID reading intervals
constexpr int on_pin = A0;         // Pin to manually activate alarm
constexpr int alarm_pin = D4;      // Alarm output pin
bool pinState = false;             // Internal state for LED blinking

/**
 * @brief Real-time clock instance
 */
STM32RTC &rtc = STM32RTC::getInstance();

/**
 * @brief Default time and date settings
 */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;
const byte weekDay = 1;
const byte day = 15;
const byte month = 6;
const byte year = 15;

//==========================================================
// System initialization
//==========================================================
void setup() {
    // Initialize hardware components
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.begin(9600);
    delay(1000);
    
    // Configure I/O pins
    pinMode(alarm_pin, OUTPUT);
    pinMode(on_pin, INPUT);
    digitalWrite(alarm_pin, LOW);
    pinMode(alarm_led_pin, OUTPUT);

    // Create default user with pre-programmed RFID tag
    User user;
    user.name = "Adrien";
    uint8_t tag_data[7] = {227, 219, 58, 251, 0, 0, 0};
    memcpy(user.tag_id, tag_data, sizeof(user.tag_id));
    user.isEnabled = true;
    users.emplace_back(user);

    // Initialize and configure the real-time clock
    rtc.begin(); // initialize RTC 24H format
    
    // Set time and date
    rtc.setHours(hours);
    rtc.setMinutes(minutes);
    rtc.setSeconds(seconds);
    rtc.setWeekDay(weekDay);
    rtc.setDay(day);
    rtc.setMonth(month);
    rtc.setYear(year);
}

/**
 * @brief Timer for alarm duration
 */
unsigned long alarmMemory = millis();

//==========================================================
// Main program loop
//==========================================================
void loop() {
    // Update LED status based on alarm state
    updateLed();

    // Process any pending serial commands
    readSerial();

    // Check if alarm should be activated via hardware switch
    if (analogRead(on_pin) > 500) {
        setAlarmOn();
    }

    // Turn off alarm after 4 seconds when triggered
    if (alarmState == TRIGGERED && millis() - alarmMemory > 4000) {
        digitalWrite(alarm_pin, LOW);
    }

    // Check for RFID cards every 200ms
    if (millis() - RFIDTimeMemory >= 200) {
        const NUID *nuid = read();
        if (nuid != nullptr) {
            // Card detected, print its UID
            Serial.println("A new card has been detected...");
            Serial.print("Card UID: ");
            for (int i = 0; i < nuid->size; i++) {
                Serial.print(nuid->uidByte[i]);
                Serial.print(" ");
            }
            Serial.println("");

            // Check if card belongs to registered user
            const User *user = getUserByUID(nuid);
            if (user != nullptr) {
                // Authorized user - disable alarm
                alarmState = OFF;
                Serial.printf("User name: %s\n", user->name.c_str());
            } else {
                // Unauthorized card - trigger alarm if system armed
                Serial.println("User not found");
                if (alarmState == ON) {
                    alarmMemory = millis();
                    alarmState = TRIGGERED;
                    digitalWrite(alarm_pin, HIGH);
                }
            }
        }
        delete nuid;
    }
}

/**
 * @brief Variables for LED state management
 */
unsigned long ledTimeMemory = 0;
int val = 0;
bool ast = true;

//==========================================================
// LED indicator control based on alarm state
//==========================================================
void updateLed() {
    if (alarmState == OFF) {
        // Alarm off - LED off
        Serial.println("Alarm off");
        analogWrite(alarm_led_pin, LOW);
    } else if (alarmState == TRIGGERED) {
        // Alarm triggered - fast blinking LED
        if (millis() - ledTimeMemory >= 200) {
            pinState = !pinState;
            Serial.println(pinState);
            ledTimeMemory = millis();
            digitalWrite(alarm_led_pin, pinState);
        }
    } else if (alarmState == ON) {
        // Alarm on - pulsing LED (fading in and out)
        if (millis() - ledTimeMemory >= 100) {
            ledTimeMemory = millis();
            val += ast ? 50 : -50;
            if (val > 255 || val < 50) ast = !ast;
            analogWrite(alarm_led_pin, val);
        }
    }
}

//==========================================================
// RFID card reading functionality
//==========================================================
NUID *read() {
    // Check if a new card is present
    if (!mfrc522.PICC_IsNewCardPresent())
        return nullptr;

    // Try to read the card
    if (!mfrc522.PICC_ReadCardSerial())
        return nullptr;

    // Verify card type
    MFRC522::PICC_Type piccType = MFRC522::PICC_GetType(mfrc522.uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return nullptr;
    }

    // Release the card for future readings
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    // Return card UID
    return new NUID{mfrc522.uid.size, mfrc522.uid.uidByte};
}

//==========================================================
// User management functionality
//==========================================================
User *getUserByUID(const NUID *nuid) {
    if (nuid == nullptr) return nullptr;
    
    // Search for matching user
    for (auto &user: users) {
        bool isValid = false;
        for (int j = 0; j < nuid->size; j++) {
            if (user.tag_id[j] != nuid->uidByte[j]) break;
            isValid = true;
        }
        if (isValid)
            return &user;
    }
    return nullptr;
}

/**
 * @brief Add new user through serial interface
 */
void addUserBySerial() {
    // Get username
    Serial.println("Give username: ");
    String name;
    while (name.length() < 1) {
        if (Serial.available() > 0) {
            name = Serial.readString();
            name.trim();
        }
    }

    // Wait for new card to scan
    Serial.println("Scan the new card...");
    NUID *nuid = nullptr;
    while (nuid == nullptr) {
        nuid = read();
    }

    // Register the new user
    addUser(name, nuid);
    delete nuid;

    Serial.printf("User name: %s, has been added\n", name.c_str());
    printUsers();
}

/**
 * @brief Add user with given name and RFID tag
 */
void addUser(const String &username, const NUID *nuid) {
    User user;
    user.name = username;
    user.isEnabled = true;
    
    // Copy the tag ID
    for (int i = 0; i < nuid->size; i++) {
        user.tag_id[i] = nuid->uidByte[i];
    }

    users.emplace_back(user);
}

/**
 * @brief Delete user by scanning their RFID card
 */
void deleteUserBySerial() {
    Serial.println("Scan cart for deleting the user....");

    // Wait for card scan
    NUID *nuid = nullptr;
    while (nuid == nullptr) {
        nuid = read();
    }

    // Find and delete user
    User *user = getUserByUID(nuid);
    delete nuid;
    if (user != nullptr) {
        Serial.printf("User name: %s, has been deleted\n", user->name.c_str());
        deleteUser(user);
    } else {
        Serial.println("User not found");
    }
}

/**
 * @brief Remove user from system
 */
void deleteUser(User *user) {
    if (user == nullptr) return;
    users.remove(*user);
    delete user;
}

//==========================================================
// Time setting functionality
//==========================================================
void setDate() {
    // Get hour
    Serial.println("Hour: ");
    String hour;
    while (hour.length() == 0) {
        if (Serial.available() > 0) {
            hour = Serial.readString();
            hour.trim();
        }
    }
    rtc.setHours(hour.toInt());
    
    // Get minutes
    Serial.println("Minutes: ");
    String minute;
    while (minute.length() == 0) {
        if (Serial.available() > 0) {
            minute = Serial.readString();
            minute.trim();
        }
    }
    rtc.setMinutes(minute.toInt());
    
    // Confirm time was set
    Serial.println("Current time set succefully...");
    Serial.printf("%02d:%02d:%02d.%03d\n",
                  rtc.getHours(),
                  rtc.getMinutes(),
                  rtc.getSeconds(),
                  rtc.getSubSeconds());
}

//==========================================================
// Serial command interface
//==========================================================
void readSerial() {
    if (Serial && Serial.available() > 0) {
        String str = Serial.readString(); // read until timeout
        str.trim();
        Serial.println(str);

        // Process commands
        if (str == "useradd") {
            addUserBySerial();
        } else if (str == "print-user") {
            printUsers();
        } else if (str == "userdel") {
            deleteUserBySerial();
        } else if (str == "setDate") {
            setDate();
        } else if (str == "set-alarm on") {
            setAlarmOn();
        } else if (str == "set-alarm trig") {
            triggerAlarm();
        } else if (str == "printTime") {
            Serial.printf("%02d:%02d:%02d.%03d\n",
                          rtc.getHours(),
                          rtc.getMinutes(),
                          rtc.getSeconds(),
                          rtc.getSubSeconds());
        } else {
            // Display command help
            Serial.println(F("\nCommands available :"));
            Serial.println(F("\t - useradd | Add user"));
            Serial.println(F("\t - print-user | List users"));
            Serial.println(F("\t - userdel | Delete user"));
            Serial.println(F("\t - setDate | set a new date"));
            Serial.println(F("\t - printTime | Print the time"));
            Serial.println(F("\t - set-alarm on | Turn on alarm"));
            Serial.print("\n\n");
        }
    }
}

//==========================================================
// User list display function
//==========================================================
void printUsers() {
    Serial.println("\n\n\nLIST USERS");
    for (auto &user: users) {
        // Print user details
        Serial.printf("USERNAME: %s\nIS ENABLED: %d\n",
                      user.name.c_str(),
                      user.isEnabled);
        
        // Print tag ID
        Serial.print("TAG ID: ");
        for (int j: user.tag_id) {
            Serial.print(j);
            Serial.print(" ");
        }
        
        // Print access days
        Serial.print("\nDAYS ACCESS: ");
        for (int j: user.days_access) {
            Serial.print(j);
            Serial.print(" ");
        }

        // Print access hours
        Serial.printf("\nStart Hour: %d:%d - End Hour: %d:%d\n",
                      user.startHour[0],
                      user.startHour[1],
                      user.endHour[0],
                      user.endHour[1]);
        Serial.println();
    }
}

//==========================================================
// Alarm control functions
//==========================================================
void setAlarmOn() {
    alarmState = ON;
}

void triggerAlarm() {
    alarmState = TRIGGERED;
}