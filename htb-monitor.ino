#include <TM1637Display.h>
#define CLK D2//pins definitions for TM1637 and can be changed to other ports
#define DIO D3

TM1637Display display = TM1637Display(CLK, DIO);

int global_boxid = 281;

void setup() {
    
    Particle.subscribe("hook-response/release-id", idHandler, MY_DEVICES);
    Particle.subscribe("hook-response/user-owns", userHandler, MY_DEVICES);
    Particle.subscribe("hook-response/root-owns", rootHandler, MY_DEVICES);
    display.setBrightness(0xff);
}

void loop() {
        
        if (Time.weekday() == 1) {
            if (Time.hour() == 7) {
                if ((Time.minute() == 0) || (Time.minute() == 1)) {
                    String getRecentBoxID = "getRecentBoxID";
                    Particle.publish("release-id", getRecentBoxID, PRIVATE);
                }
            }
        }
        
        String boxid = String(global_boxid);
        Particle.publish("user-owns", boxid, PRIVATE);
        delay(10000);
        Particle.publish("root-owns", boxid, PRIVATE);
        delay(10000);
        
        // tm1637.display(0, 1);
        // tm1637.display(1, 1);
        // tm1637.display(2, 1);
        // tm1637.display(3, 1);
}

void idHandler(const char *event, const char *data) {
    // Handle the integration response
    
    global_boxid = String(data).toInt();
    
}

void userHandler(const char *event, const char *data) {
    // Handle the integration response

    const uint8_t done[] = {
      SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
      SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
      SEG_C | SEG_E | SEG_G,                           // n
      SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
    };
    
    const uint8_t user[] = {
        SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // U
        SEG_A | SEG_G | SEG_D | SEG_C | SEG_F,   // S
        SEG_A | SEG_G | SEG_D | SEG_E | SEG_F,   // E
        SEG_A | SEG_B | SEG_F | SEG_E            // R

    };
    
    display.setSegments(user);
    delay(10000);
    
    int user_count = String(data).toInt();
    display.showNumberDec(user_count, false, 4, 0);
    
}

void rootHandler(const char *event, const char *data) {
    // Handle the integration response
    
    const uint8_t root[] = {
        SEG_A | SEG_B | SEG_F | SEG_E,                   // R
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
        SEG_A | SEG_B | SEG_C                            // T
    };
    
    display.setSegments(root);
    delay(10000);
    
    int root_count = String(data).toInt();
    display.showNumberDec(root_count, false, 4, 0);
    
}
