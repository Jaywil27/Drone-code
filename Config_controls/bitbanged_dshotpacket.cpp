#include <iostream>
#include <bitset>
#include <vector>
#include <pigpio.h>
#include <time.h>  // For nanosleep


#define bit_time (1.0 / 150000.0) // Bit time in seconds (≈ 6.66667 µs)
#define gpio_pin 18, 13, 15, 16

// Global vector holding the 16-bit DShot frame data.
vector<int> nums(16, 0);

/// Delays for the given time in microseconds with fractional precision.
void preciseDelay(double microseconds) {
    int whole = static_cast<int>(microseconds);
    double fractional = microseconds - whole;
    gpioDelay(whole);  // Delay the whole microsecond part

    // Delay the fractional part using nanosleep (convert µs fraction to ns)
    if (fractional > 0) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = static_cast<long>(fractional * 1000); 
        nanosleep(&ts, NULL);
    }
}

void dshotsend() {
    // Convert bit_time (in seconds) to microseconds.
    double bit_time_us = bit_time * 1e6;  // ≈6.66667 µs

    for (int i = 0; i < 16; i++){
        if (nums[i] == 1) {
            // For a logical 1, use a 75% duty cycle.
            // High for 75% of the bit time, low for 25%.
            gpioWrite(18, 1);
            gpioWrite(16, 1);
            gpioWrite(15, 1);
            gpioWrite(13, 1);
            preciseDelay(bit_time_us * 0.75);  // 5.0 µs high
            gpioWrite(18, 0);
            gpioWrite(16, 0);
            gpioWrite(15, 0);
            gpioWrite(13, 0);
            preciseDelay(bit_time_us * 0.25);  // 1.66667 µs low
        } else {
            // For a logical 0, use a 37.5% duty cycle.
            // High for 37.5% of the bit time, low for 62.5%.
            gpioWrite(18, 1);
            gpioWrite(16, 1);
            gpioWrite(15, 1);
            gpioWrite(13, 1);
            preciseDelay(bit_time_us * 0.375); // 2.5 µs high
            gpioWrite(18, 0);
            gpioWrite(16, 0);
            gpioWrite(15, 0);
            gpioWrite(13, 0);
            preciseDelay(bit_time_us * 0.625); // 4.16667 µs low
        }
    }
}

int main() {
    // Initialize pigpio and verify success.
    if (gpioInitialise() < 0) {
        cerr << "GPIO initialization failed!" << endl;
        return 1;
    }
    
    // Set up GPIO 18 as output.
    gpioSetMode(18, PI_OUTPUT);
    gpioSetMode(16, PI_OUTPUT);
    gpioSetMode(15, PI_OUTPUT);
    gpioSetMode(13, PI_OUTPUT);


    int throttle; 

    std::cin >> throttle;
    
    // Example: Create an 11-bit binary representation of the number 99.
    int num = throttle;
    std::bitset<11> binary(num);
    for (int i = 0; i < 11; i++){
        // The most-significant bit goes first.
        nums[i] = binary.test(10 - i);
    }
    // Populate the remaining bits with telemetry and checksum values.
    nums[11] = 0; // telemetry value 
    nums[12] = 0; // Checksum 
    nums[13] = 1; // Checksum 
    nums[14] = 1; // Checksum 
    nums[15] = 0; // Checksum (16th bit)
    
    // Compute the frame gap delay.
    // Frame gap is 1/9375 seconds, converted to microseconds.
    double frame_gap_us = 1e6 / 9375.0;  // ≈106.66667 µs

    // Continuously send the DShot frame.
    while (true) {
        dshotsend();
        preciseDelay(frame_gap_us); // Delay between frames
    }
    
    gpioTerminate();
    return 0;
}
