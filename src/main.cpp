#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "main.h"
#include <list>
#include <string>
#include "../lib/STM32RTC-1.7.0/src/STM32RTC.h"

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

std::list<User> users;

unsigned long RFIDTimeMemory = 0;

const int alarm_pin = D4;

/* Get the rtc object */
STM32RTC &rtc = STM32RTC::getInstance();

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
/* Monday 15th June 2015 */
const byte weekDay = 1;
const byte day = 15;
const byte month = 6;
const byte year = 15;

void setup() {
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.begin(9600);
    delay(1000);
    pinMode(alarm_pin, OUTPUT);
    digitalWrite(alarm_pin, LOW);

    User user;
    user.name = "Adrien";
    uint8_t tag_data[7] = {227, 219, 58, 251, 0, 0, 0};
    memcpy(user.tag_id, tag_data, sizeof(user.tag_id));
    user.isEnabled = true;

    users.emplace_back(user);

    rtc.begin(); // initialize RTC 24H format

    // Set the time
    rtc.setHours(hours);
    rtc.setMinutes(minutes);
    rtc.setSeconds(seconds);

    // Set the date
    rtc.setWeekDay(weekDay);
    rtc.setDay(day);
    rtc.setMonth(month);
    rtc.setYear(year);
}

void loop() {
    // Serial.println("Hello World!");
    readSerial();

    if (millis() - RFIDTimeMemory >= 200) {
        const NUID *nuid = read();
        if (nuid != nullptr) {
            Serial.println("A new card has been detected...");

            Serial.print("Card UID: ");
            for (int i = 0; i < nuid->size; i++) {
                Serial.print(nuid->uidByte[i]);
                Serial.print(" ");
            }
            Serial.println("");

            const User *user = getUserByUID(nuid);
            if (user != nullptr) {
                Serial.printf("User name: %s\n", user->name.c_str());
            } else {
                Serial.println("User not found");
                digitalWrite(alarm_pin, HIGH);
                delay(5000);
                digitalWrite(alarm_pin, LOW);
            }
        }
        delete nuid;
    }
}

NUID *read() {
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent())
        return nullptr;

    // Verify if the NUID has been readed
    if (!mfrc522.PICC_ReadCardSerial())
        return nullptr;

    // Check is the PICC of Classic MIFARE type
    MFRC522::PICC_Type piccType = MFRC522::PICC_GetType(mfrc522.uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return nullptr;
    }

    // Halt PICC
    mfrc522.PICC_HaltA();

    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();

    return new NUID{mfrc522.uid.size, mfrc522.uid.uidByte};
}


User *getUserByUID(const NUID *nuid) {
    if (nuid == nullptr) return nullptr;
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

void addUserBySerial() {
    Serial.println("Give username: ");
    String name;
    while (name.length() < 1) {
        if (Serial.available() > 0) {
            name = Serial.readString();
            name.trim();
        }
    }

    Serial.println("Scan the new card...");
    NUID *nuid = nullptr;

    while (nuid == nullptr) {
        nuid = read();
    }

    addUser(name, nuid);
    delete nuid;

    Serial.printf("User name: %s, has been added\n", name.c_str());

    printUsers();
}

void addUser(const String &username, const NUID *nuid) {
    User user;
    user.name = username;
    user.isEnabled = true;
    for (int i = 0; i < nuid->size; i++) {
        user.tag_id[i] = nuid->uidByte[i];
    }

    users.emplace_back(user);
}

void deleteUserBySerial() {
    Serial.println("Scan cart for deleting the user....");

    NUID *nuid = nullptr;
    while (nuid == nullptr) {
        nuid = read();
    }

    User *user = getUserByUID(nuid);
    delete nuid;
    if (user != nullptr) {
        Serial.printf("User name: %s, has been deleted\n", user->name.c_str());
        deleteUser(user);
    } else {
        Serial.println("User not found");
    }
}

void deleteUser(User *user) {
    if (user == nullptr) return;

    users.remove(*user);
    delete user;
}

void setDate() {
    Serial.println("Hour: ");
    String hour;
    while (hour.length() == 0) {
        if (Serial.available() > 0) {
            hour = Serial.readString();
            hour.trim();
        }
    }
    rtc.setHours(hour.toInt());
    Serial.println("Minutes: ");
    String minute;
    while (minute.length() == 0) {
        if (Serial.available() > 0) {
            minute = Serial.readString();
            minute.trim();
        }
    }
    rtc.setMinutes(minute.toInt());
    Serial.println("Current time set succefully...");
    Serial.printf("%02d:%02d:%02d.%03d\n",
                  rtc.getHours(),
                  rtc.getMinutes(),
                  rtc.getSeconds(),
                  rtc.getSubSeconds());
}

void readSerial() {
    if (Serial && Serial.available() > 0) {
        String str = Serial.readString(); // read until timeout
        str.trim();
        Serial.println(str);

        if (str == "useradd") {
            addUserBySerial();
        } else if (str == "print-user") {
            printUsers();
        } else if (str == "userdel") {
            deleteUserBySerial();
        } else if (str == "setDate") {
            setDate();
        } else if (str == "printTime") {
            Serial.printf("%02d:%02d:%02d.%03d\n",
                          rtc.getHours(),
                          rtc.getMinutes(),
                          rtc.getSeconds(),
                          rtc.getSubSeconds());
        } else {
            Serial.println(F("\nCommands available :"));
            Serial.println(F("\t - useradd | Add user"));
            Serial.println(F("\t - print-user | List users"));
            Serial.println(F("\t - userdel | Delete user"));
            Serial.println(F("\t - setDate | set a new date"));
            Serial.println(F("\t - printTime | Print the time"));
            Serial.print("\n\n");
        }
    }
}

void printUsers() {
    Serial.println("\n\n\nLIST USERS");
    for (auto &user: users) {
        Serial.printf("USERNAME: %s\nIS ENABLED: %d\n",
                      user.name.c_str(),
                      user.isEnabled);
        Serial.print("TAG ID: ");
        for (int j: user.tag_id) {
            Serial.print(j);
            Serial.print(" ");
        }
        Serial.print("\nDAYS ACCESS: ");
        for (int j: user.days_access) {
            Serial.print(j);
            Serial.print(" ");
        }

        Serial.printf("\nStart Hour: %d:%d - End Hour: %d:%d\n",
                      user.startHour[0],
                      user.startHour[1],
                      user.endHour[0],
                      user.endHour[1]);
        Serial.println();
    }
}
