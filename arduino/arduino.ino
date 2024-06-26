#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

#define TL_ON LOW
#define TL_OFF HIGH
#define TL_PER_DIGIT 7
#define NUM_DIGITS 6
#define TL_COUNT 42
const int pins[TL_COUNT] = {
     7, 51,  2,  3,  4,  5,  6,
    10, 11, 14, 50,  8, 12,  9,
    28, 27, 25, 24, 26, 22, 23,
    33, 32, 35, 29, 34, 31, 30,
    43, 49, 45, 47, 44, 46, 48,
    39, 42, 41, 36, 37, 40, 38
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
        onOffs = numberToArray(number);
        displayOneDigit(onOffs, s);
    }
}

void displayOff() {
    for (int i = 0; i < TL_COUNT; i++) {
        digitalWrite(pins[i], TL_OFF);
    }
}

void displayOneDigit(int *onOffs, int location) {
    // location is index of the display digit group
    if (location < 0 || location > NUM_DIGITS - 1) {
        return;
    }
    int startPinIndex = location * TL_PER_DIGIT;


    int mode;
    for (int i = 0; i < TL_PER_DIGIT; i++) {
        mode = (onOffs[i] == 1) ? TL_ON : TL_OFF;
        digitalWrite(pins[startPinIndex + i], mode);
    }
}

void displayAllDigits(int *onOffs) {
    int mode;
    // for displaying all 42 tl's
    for (int i = 0; i < TL_COUNT; i++) {
        mode = (onOffs[i] == 1) ? TL_ON : TL_OFF;
        digitalWrite(pins[i], mode);
    }
}

int isNewMinute() {
    char nums[2] = "ss";
    rtc.now().toString(nums);
    return atoi(nums) < 10 || atoi(nums) > 50;
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
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], TL_OFF);
    }
}

int isNight() {
    int hour = rtc.now().hour();
    return 2 < hour && hour < 18;
}

void loop() {

    if (isNight()) {
        displayOff();
        delay(5 * 60 * 1000);
        return;
    }

    if (isNewMinute()) {
        displayTime();
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
        displayAllDigits(onOffs);
    }
}
