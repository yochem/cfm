#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#define TL_PER_DIGIT 7
#define NUM_DIGITS 6


// NUM_DIGITS * TL_PER_DIGIT
#define TL_COUNT 42
/* int pins[TL_COUNT] = {
    20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 51, 52
};
 */
int pins[7] = {22, 23, 24, 25, 26, 27, 28};
int NUM_ARTWORKS = 0;

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

void time(DateTime time) {
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
    time.toString(nums);
    int *onOffs;

    for (char s = 0; s < strlen(nums); s++) {
        // magic to convert '9' (char) --> 9 (int)
        int number = nums[s] - '0';
        // get the right array representation for the number
        onOffs = numberToArray(number);
        // apply array
        for (int i = 0; i < 7; i++) {
            // s is the current digit, i is the index in the digit
            // TODO: check if s * 7 + i is correct
            digitalWrite(pins[s * 7 + i], onOffs[i]);
        }
    }
}


void setup() {
    Serial.begin(9600);

    if (!rtc.begin()) {
        /* Serial.println("ERROR: could not find RTC"); */
        /* Serial.flush();
        abort(); */
    }

    if (!rtc.isrunning()) {
        /* Serial.println("WARNING: RTC is NOT running, let's set the time"); */
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    for (int i = 0; i < sizeof pins / sizeof (int); i++) {
        /* pinMode(pins[i], OUTPUT); */
        digitalWrite(pins[i], 0);
    }
}

void writeNum(int num, int sleep) {
    int *onOffs;
    onOffs = numberToArray(num);
    for (int j = 0; j < 7; j++) {
        if (onOffs[j] == 1) {
            digitalWrite(pins[j], HIGH);
        } else {
            digitalWrite(pins[j], LOW);
        }
    }
    delay(sleep);
}


void loop() {
    int timeDisplayTime = 20;
    char* mode = "random";
    int commandIndex = 0;
    int artworkIndex = 0;
    int artworks[4200];

    String inputString = Serial.readString();
    char input[inputString.length() + 1];
    inputString.toCharArray(input, inputString.length());

    if (inputString.length() < 2) {
        return;
    }

    // Read each command pair
    char* command = strtok(input, "|");
    while (command != NULL) {
        if (commandIndex == 0) {
            mode = command;
            Serial.print("mode is: ");
            Serial.println(mode);
        } else if (commandIndex == 1) {
            timeDisplayTime = atoi(command);
            Serial.print("time is: ");
            Serial.println(timeDisplayTime);
        } else {
            Serial.print("artworks are: ");
            for (int i = 0; command[i] != '\0'; i++) {
                artworks[artworkIndex * 42 + i] = command[i] - '0';
                Serial.print(command[i]);
            }
            Serial.print("\n");
            artworkIndex++;
        }

        // Find the next command in input string
        command = strtok(NULL, "|");
        commandIndex++;
    }

    NUM_ARTWORKS = artworkIndex;
    Serial.println(artworkIndex);
    delay(1000);
    return;
}
