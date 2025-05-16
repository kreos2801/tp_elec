/**
 * @file sys3046Template.h
 * @author  Derache adrien
 * @author  Gaouditz Yann
 * @version 1.0.0
 */   
 
#ifndef SYS3046TEMPLATE_H_INCLUDE
#define SYS3046TEMPLATE_H_INCLUDE

/**
 * Description of MYCONFIG_VALUE
 */
#define MYCONFIG_VALUE  0x1234

// This enum defines the three possible states of the alarm system
enum ALARM_STATE {
    OFF,      // Alarm is completely disabled
    ON,       // Alarm is active and monitoring for unauthorized access
    TRIGGERED // Alarm has detected an unauthorized access and is sounding
};

// This structure represents an RFID card's unique identifier
typedef struct NUID_s {
    byte size;     // Number of bytes in the UID (can be 4, 7, or 10 bytes)
    byte *uidByte; // Pointer to the array containing the actual UID bytes
} NUID;

// Function to read an RFID card and return its UID
// Returns a pointer to a NUID structure, or nullptr if no card is detected
NUID *read();

// This structure represents a user authorized to access the system
typedef struct User_s {
    String name = "";                     // User's name
    byte tag_id[7]{};                     // UID of the user's RFID tag (up to 7 bytes)
    bool days_access[7]{};                // Access permissions for each day of week (Mon-Sun)
    bool isEnabled = false;               // Whether this user account is active
    std::array<int, 2> startHour = {-1, -1}; // Access start time (hour, minute)
    std::array<int, 2> endHour = {-1, -1};   // Access end time (hour, minute)
    
    // Operator overloading to compare users by name
    bool operator==(const User_s &other) const {
        return name == other.name;
    }
} User;

// Function to find a user by their RFID card's UID
// Returns pointer to the User if found, or nullptr if not found
User *getUserByUID(const NUID *nuid);

// Function to read and process commands from the serial monitor
void readSerial();

// Function to add a new user through the serial interface
// Prompts for name and scanning of RFID card
void addUserBySerial();

// Function to add a user with the given name and RFID card UID
void addUser(const String &username, const NUID *nuid);

// Function to delete a user from the system
void deleteUser(User *user);

// Function to print all registered users to the serial monitor
void printUsers();

// Function to activate the alarm system
void setAlarmOn();

// Function to update the LED based on current alarm state
// OFF: LED off, ON: pulsing LED, TRIGGERED: fast blinking LED
void updateLed();

// Function to immediately trigger the alarm
void triggerAlarm();

#endif  //SYS3046TEMPLATE_H_INCLUDE
