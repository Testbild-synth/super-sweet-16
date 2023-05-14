#include "Mux.h"
#include <CD74HC4067.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>


Adafruit_MCP4728 mcp;

#define CLK_IN 2
#define RESET_IN 3

#define MUX1 4
#define MUX2 5
#define MUX3 6
#define MUX4 7
#define PAUSE_SW 8
#define HOLD_NOTE_SW 9
#define STEP_BUTTON 10
#define CARRY_OUT 11
#define CARRY_IN 12

#define CLOCK_OUT 13
#define ACTIVE_STEP_POT A0

#define SCALE_POT A2
#define RESET_SET_STEP_BUTTON A3
#define SLIDE_POT A7
#define ORDER_POT A6

//this is how long the mux needs to settle to prevent crosstalk with my circuit.
//if you still get crosstalk, set it higher
#define MUX_DELAY_CROSSTALK_US 300
CD74HC4067 my_mux(MUX1, MUX2, MUX3, MUX4);

int step_count = 0;
int max_step = 15;
unsigned long last_step_time = 0;
int debounce_timer = 30;

int clock_ms = 600;
bool internal_clock_enable = false;

bool gate = false;
int gate1_out, gate2_out = 0;

int value = 0;
uint8_t scale_val = 0;
uint8_t seq_val = 0;
uint8_t note = 0;
uint16_t out_values[61] = {0, 68, 136, 205, 273, 341, 410, 478, 546, 614, 682, 751, 819, 887, 956, 1024, 1092, 1160, 1228, 1297, 1365, 1433, 1502, 1570, 1638, 1706, 1774, 1843, 1911, 1979, 2048, 2116, 2184, 2252, 2320, 2389, 2457, 2525, 2594, 2662, 2730, 2798, 2866, 2935, 3003, 3071, 3140, 3208, 3276, 3344, 3412, 3481, 3549, 3617, 3686, 3754, 3822, 3890, 3958, 4027, 4095};
uint8_t notes[1024] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60};
uint8_t num_octaves = 5;
float aread_offset = 1.06;
float cv1_out, cv2_out, cv1_out_temp, cv2_out_temp = 0;

float slide_alpha = 0.0;
bool ch1_hold, ch2_hold, ch1_pause, ch2_pause = false;

//updated via check_seq_order()
uint8_t step_order[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

uint8_t max_step_pot_value = 0;
uint8_t scales[8][12] = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, //chromatic
  {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9 , 11}, //major
  {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10}, //minor
  {0, 0, 2, 3, 3, 5, 5, 7, 7, 9, 9, 11}, //melodic minor
  {0, 0, 0, 3, 3, 5, 6, 7, 7, 9, 9, 11}, //blues
  {0, 0, 0, 3, 3, 5, 5, 7, 7, 7, 10, 10}, // pentatonic
  {0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 12, 12}, //oct and quint
  {0, 0, 0, 0, 0, 0, 0, 12, 12, 12, 12, 12} // oct
}; //oct

void update_dac(int valueA, int valueB, int valueC, int valueD) {
  mcp.setChannelValue(MCP4728_CHANNEL_A, valueA);
  mcp.setChannelValue(MCP4728_CHANNEL_B, valueB);
  mcp.setChannelValue(MCP4728_CHANNEL_C, valueC);
  mcp.setChannelValue(MCP4728_CHANNEL_D, valueD);
}

void setup() {
  pinMode(ACTIVE_STEP_POT, INPUT);
  pinMode(MUX1, OUTPUT);
  pinMode(MUX2, OUTPUT);
  pinMode(MUX3, OUTPUT);
  pinMode(MUX4, OUTPUT);

  pinMode(RESET_SET_STEP_BUTTON, INPUT_PULLUP);
  digitalWrite(RESET_SET_STEP_BUTTON, HIGH);
  pinMode(SCALE_POT, INPUT);
  pinMode(SLIDE_POT, INPUT);
  pinMode(ORDER_POT, INPUT);
  pinMode(STEP_BUTTON, INPUT_PULLUP);
  digitalWrite(STEP_BUTTON, HIGH);
  pinMode(CARRY_OUT, OUTPUT);
  pinMode(CARRY_IN, INPUT);
  
  pinMode(RESET_IN, INPUT);
  pinMode(HOLD_NOTE_SW, INPUT);
  pinMode(PAUSE_SW, INPUT);
  pinMode(CLOCK_OUT, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK_IN), clk_step_update, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET_IN), reset_counter, RISING);
  analogReference(EXTERNAL);

  while (true) {
    if (mcp.begin()) {
      break;
    }
    delay(100);
  }
  for (int i = 0; i < 16; i++) {
    my_mux.channel(i);
    update_dac(i * 255, i * 255, i * 255, i * 255);
    delay(100);
  }
  for (int i = 15; i >= 0 ; i--) {
    my_mux.channel(i);
    update_dac(i * 255, i * 255, i * 255, i * 255);
    delay(100);
  }
}

void reset_counter() {
  step_count = 0;
}

void update_channel_values(int& gate_out, float& temp_cv_out, float& cv_out) {
  delayMicroseconds(10);
  if ((gate || digitalRead(HOLD_NOTE_SW)) && !digitalRead(PAUSE_SW)) {
    gate_out = 4095;
  }
  else {
    gate_out = 0;
  }
  (void)analogRead(ACTIVE_STEP_POT);
  delayMicroseconds(100);
  if (scale_val == 0) {
    temp_cv_out = aread_offset * float(analogRead(ACTIVE_STEP_POT)) * 4 * float(num_octaves) / 5.0;

  }
  else {
    note = int(notes[constrain(int(aread_offset * float(analogRead(ACTIVE_STEP_POT))), 0, 1023)] * float(num_octaves) / 5.0);
    note = note - note % 12  + scales[scale_val][note % 12];
    temp_cv_out = out_values[note];
  }
  if (digitalRead(HOLD_NOTE_SW)) {
    cv_out = constrain(temp_cv_out * (1 - slide_alpha) + cv_out * (slide_alpha), 0, 4095.0);
  }
  else {
    cv_out = constrain(temp_cv_out, 0, 4095.0);
  }
}


void update_split() {
  // update channel 1
  slide_alpha = float(analogRead(SLIDE_POT) / 1200.0);
  my_mux.channel(step_order[step_count]);
  delayMicroseconds(MUX_DELAY_CROSSTALK_US);
  update_channel_values(gate1_out, cv1_out_temp, cv1_out);

  my_mux.channel(step_order[step_count] + 8);
  delayMicroseconds(MUX_DELAY_CROSSTALK_US);
  update_channel_values(gate2_out, cv2_out_temp, cv2_out);
  update_dac(gate1_out, gate2_out, int(cv1_out), int(cv2_out));
}


void update_dual() {
  slide_alpha = float(analogRead(SLIDE_POT) / 1024.0);
  my_mux.channel(step_order[step_count]);
  delayMicroseconds(MUX_DELAY_CROSSTALK_US);
  update_channel_values(gate1_out, cv1_out_temp, cv1_out);
  update_dac(gate1_out, gate1_out, int(cv1_out), int(cv1_out));
}



void update_gate_and_cv() {
  scale_val = round(analogRead(SCALE_POT) / 128);

  if (max_step < 8) {
    update_split();
  }
  else {
    update_dual();
  }
}


void button_step_update(bool button_value){
  if (((millis() - last_step_time) < debounce_timer)){
    return;
  }
  last_step_time = millis();
  if(!button_value){
    if(step_count < max_step){
      digitalWrite(CARRY_OUT, LOW);
      step_count += 1;
      if(step_count == max_step){
        digitalWrite(CARRY_OUT, HIGH);
        delayMicroseconds(1);
    }
    }
    else if (digitalRead(CARRY_IN)){
      step_count = 0;
      digitalWrite(CARRY_OUT, LOW);
    }
   }
   gate = !button_value;
}

void clk_step_update(){
  if (((millis() - last_step_time) < debounce_timer)){
    return;
  }
  last_step_time = millis();
  if(digitalRead(CLK_IN)){
      if(step_count < max_step){
      digitalWrite(CARRY_OUT, LOW);
      step_count += 1;
      if(step_count == max_step){
        digitalWrite(CARRY_OUT, HIGH);
        delayMicroseconds(1);
      }
      }
      else if (digitalRead(CARRY_IN)){
      step_count = 0;
      digitalWrite(CARRY_OUT, LOW);
    }
    gate = true;
  }
  else{
    gate = false;
  }
}

void check_seq_order(bool recalculate) {
  if ((seq_val != analogRead(ORDER_POT) / 256) || recalculate) {
    seq_val =  analogRead(ORDER_POT) / 256;

    //normal
    if (seq_val == 0) {
      if (max_step < 8) {
        for (int i = 0; i < 8; i++) {
          step_order[i] = i;
          step_order[i + 8] = i;
        }
      }
      else {
        for (int i = 0; i < 16; i++) {
          step_order[i] = i;
        }
      }
    }
    // backwards
    else if (seq_val == 1) {
      if (max_step < 8) {
        for (int i = max_step; i >= 0; i--) {
          step_order[max_step - i] = i;
          step_order[max_step - i + 8] = i;
        }
      }
      else {
        for (int i = max_step; i >= 0; i--) {
          step_order[max_step - i] = i;
        }
      }
    }
    // random
    else if (seq_val == 2) {
      for (int i = max_step; i > 0; i--)
      {
        long j = random(0, max_step);
        swap(&step_order[i], &step_order[j]);
      }
      if (max_step < 8) {
        for (int i = max_step; i >= 0; i--) {
          step_order[i + 8] = step_order[i];
        }
      }
    }
  }
}

void swap (uint8_t *a, uint8_t *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

//set the step count, disable interrupts
void reset_set_step() {
  int start_max_steps_setting = floor(analogRead(SLIDE_POT) / 64);
  int start_num_octaves_setting = constrain(round(analogRead(SCALE_POT) / 190), 1, 5);
  digitalWrite(CLOCK_OUT, LOW);
  int settings_timer = millis();
  while (!digitalRead(RESET_SET_STEP_BUTTON)) {
    if ((millis() - settings_timer) > 500) {
      
      // turn on internal clock and allow settings while both buttons are pressed
      if(!digitalRead(STEP_BUTTON)){
        internal_clock_enable = !internal_clock_enable;
        mcp.setChannelValue(MCP4728_CHANNEL_C, 4000);
        mcp.setChannelValue(MCP4728_CHANNEL_D, 4000);
        delay(200);
        mcp.setChannelValue(MCP4728_CHANNEL_C, 0);
        mcp.setChannelValue(MCP4728_CHANNEL_D, 0);
        delay(200);
        mcp.setChannelValue(MCP4728_CHANNEL_C, 4000);
        mcp.setChannelValue(MCP4728_CHANNEL_D, 4000);
        delay(200);
        mcp.setChannelValue(MCP4728_CHANNEL_C, 0);
        mcp.setChannelValue(MCP4728_CHANNEL_D, 0);
        delay(200);
        int start_clock_setting = analogRead(ORDER_POT)/128;
        while(!digitalRead(STEP_BUTTON) && internal_clock_enable){
          clock_out(false, true);
          if (start_clock_setting != analogRead(ORDER_POT)/128){
            clock_ms = constrain(3100-analogRead(ORDER_POT)*3, 50, 3000); 
          }
        }
      }
      // press both buttons - switch between detailed and full mode
      if (constrain(round(analogRead(SCALE_POT) / 190), 1, 5) != start_num_octaves_setting) {
        num_octaves = constrain(round(analogRead(SCALE_POT) / 190), 1, 5);
        update_dac(800 * num_octaves, 800 * num_octaves, 800 * num_octaves, 800 * num_octaves);
        delay(1);
      }
      if (floor(analogRead(SLIDE_POT) / 64) != start_max_steps_setting) {
        max_step = floor(analogRead(SLIDE_POT) / 64);
        if (max_step < 8) {
          for (int i = 0; i <= max_step; i++) {
            my_mux.channel(i);
            delay(1);
            my_mux.channel(i + 8);
            delay(1);
          }
        }
        else {
          update_dac(4095, 0, max_step * 255, max_step * 255);
          for (int i = 0; i <= max_step; i++) {
            my_mux.channel(i);
            delay(1);
          }
        }
      }
    }
  }
  mcp.setChannelValue(MCP4728_CHANNEL_A, 0);
  mcp.setChannelValue(MCP4728_CHANNEL_B, 0);
  mcp.setChannelValue(MCP4728_CHANNEL_C, 0);
  mcp.setChannelValue(MCP4728_CHANNEL_D, 0);
  step_count = 0;
}

void clock_out(bool pin, bool dac){
  if(internal_clock_enable){
    if(pin){
      digitalWrite(CLOCK_OUT, ((millis() % clock_ms) < int(clock_ms/2)));
    }
    if(dac){
       mcp.setChannelValue(MCP4728_CHANNEL_A, ((millis() % clock_ms) < int(clock_ms/2))*4000);
    }
  }
}

bool button_val = false;

void loop() {
  clock_out(true, false);
  if (!digitalRead(RESET_SET_STEP_BUTTON)) {
    reset_set_step();
  }
  if (digitalRead(STEP_BUTTON) != button_val){
    button_val = digitalRead(STEP_BUTTON);
    button_step_update(button_val);
  }
  check_seq_order(false);
  update_gate_and_cv();
}
