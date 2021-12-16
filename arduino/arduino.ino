#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#define TL_PER_DIGIT 7
#define  NUM_DIGITS 6
#define TL_COUNT 42
/* const int pins[TL_COUNT] = {
    20, 21, 22, 23, 24, 25, 26, // digit 1
    27, 28, 29, 30, 31, 32, 33, // digit 2
    34, 35, 36, 37, 38, 39, 40, // digit 3
    41, 42, 43, 44, 45, 46, 47, // digit 4
    48, 49, 50, 51, 52, 53, 54, // digit 5
    55, 56, 57, 58, 59, 51, 52  // digit 6
} */
const int pins[14] = {
    0, 0, 0, 0, 0, 0, 0,
    22, 23, 24, 25, 26, 27, 28};

char *mode = "random";

// space for 100 artworks, hard cap
int artworks[100];
int NUM_ARTWORKS = 0;

int timeDisplayTime = 10;
int randomDisplayTime = 10;

void setBit(int k, int val) {
    int bitIndex = sizeof (int) * 8;
    if (val == 1) {
        artworks[k/bitIndex] |= 1 << (k%bitIndex);
    } else {
        artworks[k/bitIndex] &= ~(1 << (k%bitIndex));
    }
}

int getBit(int k) {
    return ((artworks[k/32] & (1 << (k%32) )) != 0);
}

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
    char nums[6] = "hhmmss";
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

void modeRandomWithTime() {
    /* char secs[2] = "ss";
    rtc.now().toString(secs); */
    char secs[2];
    sprintf(secs, "%d", random(0, 60));

    // look in a window of the first 10 seconds of a minute
    if (atoi(secs) < 10) {
        displayTime();
        delay(timeDisplayTime * 100);
    }

    // every artwork is tl_count long in artworks
    int *onOffs = artworks[random(0, NUM_ARTWORKS) * TL_COUNT];
    displayAllDigits(onOffs);
    delay(randomDisplayTime * 100);
}

void setup() {
    Serial.begin(9600);

    if (!rtc.begin()) {
        Serial.println("ERROR: could not find RTC");
    }

    if (!rtc.isrunning()) {
        Serial.println("WARNING: RTC is NOT running, let's set the time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    for (int i = 0; i < sizeof pins / sizeof (int); i++) {
        // TODO: probably needed for TL's
        /* pinMode(pins[i], OUTPUT); */
        digitalWrite(pins[i], LOW);
    }

    for (int i = 0; i < sizeof artworks / sizeof (int); i++) {
        artworks[i] = 0;
    }
}

void loop() {
    Serial.println(getBit(0));
    Serial.println(getBit(1));
    Serial.println(getBit(2));

    setBit(1, 1);
    setBit(1, 0);
    setBit(0, 1);
    setBit(2, 1);

    Serial.println(getBit(0));
    Serial.println(getBit(1));
    Serial.println(getBit(2));

    delay(1000000);

    return;
    if (strcmp(mode, "random") == 0) {
        modeRandomWithTime();
    }

    int commandIndex = 0;
    int artworkIndex = 0;

    String inputString = Serial.readString();

    if (inputString.length() < 2) {
        return;
    }

    char input[inputString.length() + 1];
    inputString.toCharArray(input, inputString.length());


    // Read each command pair
    char* command = strtok(input, "|");
    while (command != NULL) {
        if (commandIndex == 0) {
            mode = command;
        } else if (commandIndex == 1) {
            timeDisplayTime = atoi(command);
        } else if (commandIndex == 2) {
            randomDisplayTime = atoi(command);
        } else if (commandIndex == 3) {
            /* free(artworks);
            artworks = (int *)malloc(sizeof (int) * atoi(command) * TL_COUNT); */
            Serial.println(sizeof (int));
        } else {
            int i;
            for (i = 0; command[i] != '\0'; i++) {
                artworks[artworkIndex * 42 + i] = command[i] - '0';
            }
            Serial.print(artworkIndex);
            Serial.println(i);
            artworkIndex++;
        }

        // Find the next command in input string
        command = strtok(NULL, "|");
        commandIndex++;
    }

    int sum = 0;
    for (int i = 0; i < 7 * 42; i++) {
        /* sum += artworks[i]; */
        Serial.print(artworks[i]);
        if (i % 41 == 0 && i != 0) {
            Serial.println("");
            /* Serial.println(sum); */
            sum = 0;
        }
    }

    NUM_ARTWORKS = artworkIndex;
}
