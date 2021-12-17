/* NOTES:
 *
 * - On relayboard, switch HIGH and LOW
 * - pinmode output
 * - Test RTC
 * - set time rtc better
 * - order pins
 * -
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

#define TL_PER_DIGIT 7
#define  NUM_DIGITS 6
#define TL_COUNT 42
/* const int pins[TL_COUNT] = {
     7,  1,  2,  3,  4,  5,  6,
     8,  9, 10, 11, 12, 13, 14,
    22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49,
}; */
const int pins[TL_COUNT] = {
    22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 51, 52,  1,  2
};

int timeDisplayTime = 10;

int *numberToArray(int num) {
    /*
            0
            -
        5 |   | 1
            6
        4 |   | 2
            -
            3
    */
    // digit should be in range 0-9
    if (num < 0 || num > 9) {
        return {};
    }

    static int numberReps[10][TL_PER_DIGIT] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}, // 9
    };
    return numberReps[num];
}

void displayTime() {
    // hh - the hour with a leading zero (00 to 23)
    // mm - the minute with a leading zero (00 to 59)
    // ss - the whole second with a leading zero where applicable (00 to 59)
    // YYYY - the year as four digit number
    // YY - the year as two digit number (00-99)
    // MM - the month as number with a leading zero (01-12)
    // MMM - the abbreviated English month name ('Jan' to 'Dec')
    // DD - the day as number with a leading zero (01 to 31)
    // DDD - the abbreviated English day name ('Mon' to 'Sun')
    char nums[] = "hhmmss";
    rtc.now().toString(nums);

    int number;
    int *onOffs;
    for (char s = 0; s < strlen(nums); s++) {
        // magic to convert '9' (char) --> 9 (int)
        number = nums[s] - '0';
        Serial.print(number);
        onOffs = numberToArray(number);
        displayOneDigit(onOffs, s);
    }
    Serial.println("");
}

void displayOneDigit(int *onOffs, int location) {
    // location is index of the display digit group
    if (location < 0 || location > NUM_DIGITS - 1) {
        return;
    }
    int startPinIndex = location * TL_PER_DIGIT;


    int mode;
    for (int i = 0; i < TL_PER_DIGIT; i++) {
        mode = (onOffs[i] == 1) ? HIGH : LOW;
        digitalWrite(pins[startPinIndex + i], mode);
    }
}

void displayAllDigits(int *onOffs) {
    int mode;
    // for displaying all 42 tl's
    for (int i = 0; i < TL_COUNT; i++) {
        mode = (onOffs[i] == 1) ? HIGH : LOW;
        digitalWrite(pins[i], mode);
    }
}

int newMinute() {
    char nums[2] = "ss";
    rtc.now().toString(nums);
    return atoi(nums) < 10;
}

void setup() {
    Serial.begin(9600);

    if (!rtc.begin()) {
        Serial.println("ERROR: could not find RTC");
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        rtc.adjust(DateTime(2021, 1, 1, 20, 18, 30));
    }

    for (int i = 0; i < sizeof pins / sizeof (int); i++) {
        // TODO: probably needed for TL's
        /* pinMode(pins[i], OUTPUT); */
        digitalWrite(pins[i], LOW);
    }
}

void loop() {
    if (newMinute()) {
        displayTime();
        delay(timeDisplayTime);
    }

    String inputString = Serial.readString();

    if (inputString.length() < 2) {
        return;
    }

    char input[inputString.length() + 1];
    inputString.toCharArray(input, inputString.length());

    if (strcmp(input, "time") == 0) {
        displayTime();
    } else if (inputString.startsWith("timeset")) {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;

        int commandIndex = 0;
        char* command = strtok(input, "|");
        while (command != NULL) {
            if (strcmp(command, "timeset") != 0) {
                switch (commandIndex) {
                    case 0: year = atoi(command); break;
                    case 1: month = atoi(command); break;
                    case 2: day = atoi(command); break;
                    case 3: hour = atoi(command); break;
                    case 4: minute = atoi(command); break;
                    case 5: second = atoi(command); break;
                }
                // Find the next command in input string
                commandIndex++;
            }
            command = strtok(NULL, "|");
        }
        rtc.adjust(DateTime(year, month, day, hour, minute, second));
    } else {
        int onOffs[TL_COUNT];
        for (int i = 0; input[i] != '\0'; i++) {
            onOffs[i] = input[i] - '0';
        }
        displayOneDigit(onOffs, 0);
        /* displayAllDigits(onOffs); */
    }
}
