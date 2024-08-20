#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <time.h>

// Screen Setting
#define Screen_Width 128
#define Screen_Height 64
#define OLED_reset -1
#define Screen_Address 0x3C

//Timezone setting
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET (0)//5.5*3600
#define UTC_OFFSET_DST 0

// I/O pins
#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 35
#define PB_DOWN 33
#define DHTPIN 12


// Declare objects
Adafruit_SSD1306 display(Screen_Width,Screen_Height,&Wire,OLED_reset);
DHTesp dhtSensor;

// Global Variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

//Variables for set UTC offset
int hours_u = 0;
int minutes_u = 0;
float offset = 0;

//Alram conditions
bool alarm_enabled = true;
int n_alarms = 3;

//Initial alarm values
int alarm_hours[]={0,0,0};
int alarm_minutes[]={0,0,0};
bool alarm_triggered[]={false,false,false};

//Music tone notes
int n_notes=8;
int C=262;
int D=294;
int E=330;
int F=349;
int G=392;
int A=440;
int B=492;
int C_H=523;

int notes[]={C,D,E,F,G,A,B,C_H};

//Initial menu option order
int current_mode=0;
int max_modes=5;
String modes[]={"1 -       Set Time",
"2 -  Set  Alarm 1", "3 -  Set  Alarm 2",
"4 -  Set  Alarm 3", "5 -       Disable   Alarms"};


void setup() {
  //Defing I/Os
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  //Transmitting data to display
  Serial.begin(115200);
  if(! display.begin(SSD1306_SWITCHCAPVCC,Screen_Address)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(500);

// Connecting to WiFi
  WiFi.begin("Wokwi-GUEST","",6);
  while (WiFi.status()!=WL_CONNECTED){
    delay(100);
    display.clearDisplay();
    print_line("Connecting to WiFi", 0, 0, 2);
  }
    display.clearDisplay();
    print_line("Connected to WiFi", 0, 0, 2);

//Setting time according to NTP
  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);

  display.clearDisplay();
  print_line("Welcome to MediBox",10,18,2);
  delay(200);
  display.clearDisplay();
}

void loop() {

  update_time_with_check_alarm();
  if(digitalRead(PB_OK)==LOW){
    //delay(100);
    go_to_menu();
  }
  check_temp();
}

// Line format setting in display
void print_line(String text, int column, int row, int t_size){
  display.setTextSize(t_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row); //Cursor position
  display.println(text);
  display.display();
}

//Displaying current time
void print_time_now(void){
  display.clearDisplay();
  print_line(String(days), 0, 0, 2); 
  print_line(":  ", 20, 0, 2);
  print_line(String(hours), 30, 0, 2);
  print_line(":", 50, 0, 2); 
  print_line(String(minutes), 60, 0, 2);
  print_line(":", 80, 0, 2);
  print_line(String(seconds), 90, 0, 2);
}

//Saving time values 
void update_time(void) { 
  struct tm timeinfo;
  getLocalTime(&timeinfo);
    char day_str[8];
    char hour_str[8];
    char min_str[8];
    char sec_str[8];
    strftime(day_str,8, "%d", &timeinfo); 
    strftime(sec_str,8, "%S", &timeinfo); 
    strftime(hour_str,8, "%H", &timeinfo); 
    strftime(min_str,8, "%M", &timeinfo);
    hours = atoi(hour_str);
    minutes = atoi(min_str);
    days = atoi(day_str);
    seconds = atoi(sec_str);
}

//Code for ring the alarm.
void ring_alarm(){
  display.clearDisplay();
  print_line("MEDICINE  TIME", 0, 0, 2);
  digitalWrite(LED_1, HIGH);

  bool break_happened = false;

  while(break_happened == false && digitalRead(PB_CANCEL)==HIGH){
    for (int i=0;i<n_notes;i++){
      if(digitalRead(PB_CANCEL)==LOW){
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER,notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}

void update_time_with_check_alarm() {
  //Updating time parameters to display
  update_time();
  print_time_now();
  // check for alarms
  if (alarm_enabled) {
    // iterating through all alarms
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
        ring_alarm(); // call the ringing function
        alarm_triggered[i] = true;
      }
    }
  }
}

//Getting input from each buttons
int wait_for_button_press(){
    while(true){
        if (digitalRead(PB_UP)==LOW){
            delay(200);
            return PB_UP;
        }
        else if (digitalRead(PB_DOWN)==LOW){
            delay(200);
            return PB_DOWN;
        }
        else if (digitalRead(PB_OK)==LOW){
            delay(200);
            return PB_OK;
        }
        else if (digitalRead(PB_CANCEL)==LOW){
            delay(200);
            return PB_CANCEL;
        }
        update_time();
    }
}

//Menu configuration
void go_to_menu(){
    while (digitalRead(PB_CANCEL) == HIGH){
        display.clearDisplay();
        print_line(modes[current_mode], 0, 0, 2);
        int pressed = wait_for_button_press();

        if (pressed == PB_UP){
            delay(200);
            current_mode += 1;
            current_mode = current_mode % max_modes;
        }
        else if (pressed == PB_DOWN){
            delay(200);
            current_mode -= 1;
            if (current_mode < 0){
                current_mode = max_modes -1;
            }

        }
        else if (pressed == PB_OK){
            delay(200);
            run_mode(current_mode);
        }
        else if(pressed == PB_CANCEL){
            delay(200);
            break;;
        }
    }
    current_mode=0;
}

//Setting time with input as UTC offset
void set_time(){
  //Offset hour setting
    int temp_hour = 0;
    while (true) {
        display.clearDisplay();
        print_line("UTC Offset",0,0,2);
        print_line("Enter hour : " + String(temp_hour), 0, 22, 2);
        int pressed = wait_for_button_press();

        if (pressed == PB_UP) {
            delay(200);
            temp_hour += 1;
            if(temp_hour > 14){
              temp_hour = -12; //Offset min
            }
            
        }
        else if (pressed == PB_DOWN) {
            delay(200);
            temp_hour -= 1;
            if (temp_hour < -12) {
                temp_hour = 14;//Offset max
            }
        }
        else if (pressed == PB_OK) {
            delay(200);
            hours_u = temp_hour;
            break;
        }
        else if (pressed == PB_CANCEL){
            delay(200);
            break;
        }       
    }
//Offset minutes setting
    int temp_minute=0;
    while (true) {
        display.clearDisplay();
        print_line("Enter     minute:" + String(temp_minute), 0, 0, 2);
        int pressed = wait_for_button_press();
        if (pressed == PB_UP) {
            delay(200);
            temp_minute += 15;//Increase minutes by 15
            temp_minute = temp_minute % 60;
        }
        else if (pressed == PB_DOWN) {
            delay(200);
            temp_minute -= 15;//Decrease minutes by 15
            if (temp_minute < 0) {
                temp_minute = 45;
            }
        }
        else if (pressed == PB_OK) {
            delay(200);
            minutes_u = temp_minute;
            break;
        }
        else if (pressed == PB_CANCEL) {
            delay(200);
            break;
        }
    }
    //Changing offset time into seconds
    if (hours_u<0){
      offset = (hours_u - (static_cast<float>(minutes_u) / 60)) * 3600;
    }
    else {
      offset = (hours_u + (static_cast<float>(minutes_u) / 60)) * 3600;
    }
    // Setting time according to the new offset
    configTime(offset,UTC_OFFSET_DST,NTP_SERVER);

    display.clearDisplay();
    print_line("Time is set", 0, 0, 2);
    delay(700);
}

// Setting alarm
void set_alarm(int alarm) {
    int temp_hour = alarm_hours[alarm]; // Declare and initialize temp_hour with the current alarm hour
    while (true) {
        display.clearDisplay();
        print_line("Enter hour: " + String(temp_hour), 0, 0, 2); // Display current temp_hour
        int pressed = wait_for_button_press(); // Wait for button press
        if (pressed == PB_UP) {
            delay(200);
            temp_hour += 1; // Increment temp_hour when UP button is pressed
            temp_hour = temp_hour % 24; // Ensure temp_hour wraps around to 0-23 range
        }
        else if (pressed == PB_DOWN) {
            delay(200);
            temp_hour -= 1; // Decrement temp_hour when DOWN button is pressed
            if (temp_hour < 0) {
                temp_hour = 23; // Ensure temp_hour wraps around to 23 when below 0
            }
        }
        else if (pressed == PB_OK) {
            delay(200);
            alarm_hours[alarm] = temp_hour; // Set the alarm hour
            break;
        }
        else if (pressed == PB_CANCEL) {
            delay(200);
            break; // Exit the loop if CANCEL button is pressed
        }
    }

    int temp_minute = alarm_minutes[alarm]; // Declare and initialize temp_minute 
                                            // with the current alarm minute
    while (true) {
        display.clearDisplay();
        print_line("Enter minute: " + String(temp_minute), 0, 0, 2); // Display current temp_minute
        int pressed = wait_for_button_press(); // Wait for button press
        if (pressed == PB_UP) {
            delay(200);
            temp_minute += 1; // Increment temp_minute when UP button is pressed
            temp_minute = temp_minute % 60; // Ensure temp_minute wraps around to 0-59 range
        }
        else if (pressed == PB_DOWN) {
            delay(200);
            temp_minute -= 1; // Decrement temp_minute when DOWN button is pressed
            if (temp_minute < 0) {
                temp_minute = 59; // Ensure temp_minute wraps around to 59 when below 0
            }
        }
        else if (pressed == PB_OK) {
            delay(200);
            alarm_minutes[alarm] = temp_minute; // Set the alarm minute
            break;
        }
        else if (pressed == PB_CANCEL) {
            delay(200);
            break; // Exit the loop if CANCEL button is pressed
        }
    }

    display.clearDisplay();
    print_line("Alarm is set", 0, 0, 2); // Print confirmation message
    delay(700);
}

//Calling internal functions of each option
void run_mode(int mode){
    if (mode == 0){
        set_time();
    }
    else if (mode == 1 || mode == 2 || mode == 3){
        set_alarm(mode-1);
        alarm_enabled = true; //Again activating alarms
    }
    else if (mode == 4){
        alarm_enabled = false;
    }
}

//Checking temperature
void check_temp(void) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    bool all_good = true;
    if (data.temperature > 32) {
        display.clearDisplay();
        print_line("TEMP HIGH", 0, 30, 2);
        delay(200);
    }
    else if (data.temperature < 26) {
        display.clearDisplay();
        print_line("TEMP LOW", 0, 30, 2);
        delay(200);
    }
    if (data.humidity > 80) {
        print_line("HUMD HIGH", 0, 50, 2);
        delay(200);
    }
    else if (data.humidity < 60) {
        print_line("HUMD LOW", 0, 50, 2);
        delay(200);
    }
}