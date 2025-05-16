/**

@file main.h
@brief Main header file for the TP_ELEC project
@details Contains declarations for RFID access control system functionality
including user management, alarm control, and RFID reading capabilities.
*/

#pragma once
#include <Arduino.h>
/**

@enum ALARM_STATE
@brief Represents the possible states of the alarm system
*/
enum ALARM_STATE {
OFF,        /** Alarm system is turned off */
ON,         /** Alarm system is active but not triggered */
TRIGGERED   /** Alarm system has been triggered */
};

/**

@struct NUID_s
@brief Structure to hold RFID tag unique identifier data
*/
typedef struct NUID_s {
byte size; /** Number of bytes in the UID. 4, 7 or 10. */
byte *uidByte; /** Pointer to the UID byte array */
} NUID;

/**

@brief Reads an RFID tag and returns its NUID
@return Pointer to NUID structure containing the read data, or NULL if no tag present
*/
NUID *read();

/**

@struct User_s
@brief User data structure for access control system
@details Contains user identification information, access credentials, schedule permissions
*/
typedef struct User_s {
String name = ""; /** User's name or identifier */
byte tag_id[7]{}; /** RFID tag identifier bytes */
bool days_access[7]{}; /** Access permissions for each day of the week, indexed 0-6 */
bool isEnabled = false; /** Whether this user account is currently active */
std::array<int, 2> startHour = {-1, -1}; /** Access start time [hour, minute], -1 means no restriction */
std::array<int, 2> endHour = {-1, -1}; /** Access end time [hour, minute], -1 means no restriction */
/**

@brief Equality comparison operator
@param other The User_s to compare with
@return true if users have the same name, false otherwise
*/
bool operator==(const User_s &other) const {
return name == other.name;
}
} User;



/**

@brief Look up a user by their RFID tag identifier
@param nuid Pointer to NUID structure containing the tag identifier
@return Pointer to the matching User, or NULL if no match found
*/
User *getUserByUID(const NUID *nuid);

/**

@brief Process serial input commands
@details Reads commands from the serial port and processes them
*/
void readSerial();

/**

@brief Add a new user with credentials provided via serial input
*/
void addUserBySerial();

/**

@brief Add a new user to the system
@param username String containing the user's name
@param nuid Pointer to NUID structure containing the user's RFID tag
*/
void addUser(const String &username, const NUID *nuid);

/**

@brief Remove a user from the system
@param user Pointer to the User to be deleted
*/
void deleteUser(User *user);

/**

@brief Display all users in the system via serial output
*/
void printUsers();

/**

@brief Activate the alarm system
*/
void setAlarmOn();

/**

@brief Update LED indicators based on system state
*/
void updateLed();

/**

@brief Activate the alarm in triggered state
*/
void triggerAlarm();