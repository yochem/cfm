#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#define TL_COUNT 7
int pins[7] = {22, 23, 24, 25, 26, 27, 28};


void numberToArray(int *arr, int num) {
    /*
            0
            -
        5 |   | 1
            6
        4 |   | 2
            -
            3
    */
    int numberReps[10][7] = {
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
    for (int i = 0; i < 7; i++) {
        arr[i] = numberReps[num][i];
    }
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
    int onOffs[7] = {0, 0, 0, 0, 0, 0, 0};

    for (char s = 0; s < strlen(nums); s++) {
        // convert '9' (char) --> 9 (int)
        int number = nums[s] - '0';
        // get the right array representation for the number
        numberToArray(onOffs, number);
        // apply array
        for (int i = 0; i < 7; i++) {
            // s is the current digit, i is the index in the digit
            digitalWrite(pins[s * 7 + i], onOffs[i]);
        }
    }
}

bool startswith(char *pre, char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}


void setup() {
    Serial.begin(9600);

    if (!rtc.begin()) {
        Serial.println("ERROR: could not find RTC");
        Serial.flush();
        abort();
    }

    if (!rtc.isrunning()) {
        Serial.println("WARNING: RTC is NOT running, let's set the time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    for (int i = 0; i < sizeof pins / sizeof (int); i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], 0);
    }
}

void loop() {
    String setting = Serial.readString();
    int setting_length = setting.length() - 1; // skip the newline char on the end

    // time is a special case that gets handled on the arduino itself
    if (startswith("time", setting)) {
        rtc.adjust(DateTime(unix_time));
        // todo
        DateTime now = rtc.now();
        time(now);
        delay(1000);
        return;
    }

    if (setting_length != TL_COUNT) {
        if (setting_length > 0) {
            Serial.println("ERROR: setting length does not match number of pins in Arduino configuration");
        }
        return;
    }

    for (int i = 0; i < setting_length; i++) {
        if (setting[i] == '0') {
            digitalWrite(pins[i], 0);
        } else if (setting[i] == '1') {
            digitalWrite(pins[i], 1);
        } else {
            Serial.print("WARNING: bad formatted setting: value ");
            Serial.print(setting[i]);
            Serial.print(" for pin ");
            Serial.print(i);
            Serial.print(" is not 0 or 1\n");
        }
    }
    Serial.println("INFO: new setting completed");
}
