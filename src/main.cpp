#include <Arduino.h>
#include "rfid.h"

Rfid rfid(10, 9);

void setup() {
    Serial.begin(9600);
    delay(1000);
}

void loop() {
    Serial.println("Hello World!");

    // const NUID *nuid = rfid.read();
    // if (nuid != nullptr) {
    //     Serial.println("A new card has been detected...");
    //
    //     Serial.print("Card UID: ");
    //     for (int i = 0; i < nuid->size; i++) {
    //         Serial.print(nuid->uidByte[i]);
    //         Serial.print(" ");
    //     }
    //     Serial.println("");
    //
    //     // if (doorState.RFID_enabled) {
    //     //     const User *user = getUserByUID(nuid);
    //     //     if (user != nullptr) {
    //     //         Serial.printf("User name: %s\n", user->name.c_str());
    //     //         toggleDoor(user);
    //     //     } else {
    //     //         Serial.println("User not found");
    //     //         pushDoorAccessLog("Unknown", "Unknown", false, "Unknown");
    //     //     }
    //     // } else {
    //     //     Serial.println("RFID disabled - can't open the door");
    //     // }
    // }
}


