#include <iostream>
#include <bitset>
#include <vector>
#include <Arduino.h>
#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"  // Include GPIO header
#include <stdio.h>

using namespace std;

rmt_encoder_handle_t copy_encoder = NULL;
rmt_channel_handle_t tx_chan = NULL;

int target = 1;
vector<int> nums(16, 0);  // Array to store 16 bits

void setup() {
    Serial.begin(9600);
    
    int num = 99;                 
    std::bitset<11> binary(num);  

    // Store the first 11 bits of num in the nums array
    for (int i = 0; i < 11; i++) {
        nums[i] = binary.test(10 - i);  // Start from the most significant bit (bit 10)
    }
    nums[11] = 0; // telemetry value 
    nums[12] = 0; // Checksum 
    nums[13] = 1; // Checksum 
    nums[14] = 1; // Checksum 
    nums[15] = 0; // checksum The 16th bit (index 15)  

    // Configure the RMT TX channel
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = GPIO_NUM_16,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 6000000,  // Clock resolution
        .mem_block_symbols = 64,
        .trans_queue_depth = 10
    };

    // Configure RMT transmission
    rmt_transmit_config_t tx_config = {
        .loop_count = -1 // Continuous transmission
    };

    // Configure the byte encoder
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .duration0 = 30,
            .level0 = 1,
            .duration1 = 10,
            .level1 = 0,
        },
        .bit1 = {
            .duration0 = 15,
            .level0 = 1,
            .duration1 = 25,
            .level1 = 0,
        },
        .flags = {
            .msb_first = true,
        }
    };

    // Initialize RMT TX channel
    esp_err_t err = rmt_new_tx_channel(&tx_chan_config, &tx_chan);
    if (err != ESP_OK) {
        Serial.printf("Error initializing TX channel: 0x%x\n", err);  // Use printf for error
        return;
    }

    // Initialize RMT encoder
    rmt_encoder_handle_t tx_encoder = NULL;
    err = rmt_new_bytes_encoder(&bytes_encoder_config, &tx_encoder);
    if (err != ESP_OK) {
        Serial.printf("Error initializing RMT encoder: 0x%x\n", err);  // Use printf for error
        return;
    }

    // Enable RMT TX
    err = rmt_enable(tx_chan);
    if (err != ESP_OK) {
        Serial.printf("Error enabling RMT TX: 0x%x\n", err);  // Use printf for error
        return;
    }

    // Transmit the data
    err = rmt_transmit(tx_chan, tx_encoder, &nums[0], nums.size(), &tx_config);
    if (err != ESP_OK) {
        Serial.printf("Error creating RMT TX transmission: 0x%x\n", err);  // Use printf for error
    } else {
        Serial.printf("RMT TX transmission started successfully!");  // Success message
    }
}

void loop() {
    // Your main loop code here

}
