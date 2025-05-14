#pragma once
#include <SPI.h>
#include <MFRC522.h>

typedef struct NUID_s {
    byte size; // Number of bytes in the UID. 4, 7 or 10.
    byte *uidByte;
} NUID;

class Rfid {
public:
    Rfid(const int ssPin, const int rstPin) : mfrc522(ssPin, rstPin) {
        SPI.begin();
        // mfrc522.PCD_Init();

    };

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

private:
    MFRC522 mfrc522;
};
