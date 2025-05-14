#pragma once
#include <Arduino.h>

enum ALARM_STATE {
    OFF,
    ON,
    TRIGGERED
};

typedef struct NUID_s {
    byte size; // Number of bytes in the UID. 4, 7 or 10.
    byte *uidByte;
} NUID;

NUID *read();

typedef struct User_s {
    String name = "";
    byte tag_id[7]{};
    bool days_access[7]{};
    bool isEnabled = false;
    std::array<int, 2> startHour = {-1, -1};
    std::array<int, 2> endHour = {-1, -1};

    bool operator==(const User_s &other) const {
        return name == other.name;
    }
} User;

User *getUserByUID(const NUID *nuid);

void readSerial();

void addUserBySerial();

void addUser(const String &username, const NUID *nuid);

void deleteUser(User *user);

void printUsers();

void setAlarmOn();

void updateLed();

void triggerAlarm();