/* Part of an open source template for Blynk.Inject for ESP32, provided by Blynk via GitHub */


#define BOARD_FIRMWARE_VERSION        "1.0.1"
#define BOARD_HARDWARE_VERSION        "1.0.0"

#define BOARD_FIRMWARE_TYPE           "FW01"                // Unique identifier of a firmware
#define BOARD_NAME                    "Automated Smart Knob"        // Name of your product. Should match App Export request info.
#define BOARD_VENDOR                  "MAGLT"        // Name of your company. Should match App Export request info.
#define BOARD_TEMPLATE_ID             "TMPL0000"            // ID of the Tile Template. Can be found in Tile Template Settings

#define PRODUCT_WIFI_SSID             "ASK Hub"         // Name of the device, to be displayed during configuration. Should match export request info.
#define BOARD_CONFIG_AP_URL           "ask.cc"      // Config page will be available in a browser at 'http://ask.cc/'

#define BOARD_DEFAULT_SERVER          "blynk-cloud.com"     // Default server to connect to (Blynk App may override it)
#define BOARD_DEFAULT_PORT            443                   // Default port

/*
 * Board configuration
 */

#if defined(USE_WROVER_BOARD)

  // Custom board configuration
  #define BOARD_BUTTON_PIN            15                     // Pin where user button is attached
  #define BOARD_BUTTON_ACTIVE_LOW     true                  // true if button is "active-low"

  #define BOARD_LED_PIN_R             0                     // Set R,G,B pins - if your LED is PWM RGB 
  #define BOARD_LED_PIN_G             2
  #define BOARD_LED_PIN_B             4
  #define BOARD_LED_INVERSE           false                 // true if LED is common anode, false if common cathode
  #define BOARD_LED_BRIGHTNESS        64                    // 0..255 brightness control

#else
  #error "No board selected"
#endif


/*
 * Advanced options
 */

#define BUTTON_HOLD_TIME_INDICATION   2000
#define BUTTON_HOLD_TIME_ACTION       5000

#define BOARD_PWM_MAX                 1023

#define LEDC_CHANNEL_1     1
#define LEDC_CHANNEL_2     2
#define LEDC_CHANNEL_3     3
#define LEDC_TIMER_BITS    10
#define LEDC_BASE_FREQ     12000

#define WIFI_NET_CONNECT_TIMEOUT      30000
#define WIFI_CLOUD_CONNECT_TIMEOUT    30000
#define WIFI_AP_CONFIG_PORT           80
#define WIFI_AP_IP                    IPAddress(192, 168, 4, 1)
#define WIFI_AP_Subnet                IPAddress(255, 255, 255, 0)
//#define WIFI_CAPTIVE_PORTAL_ENABLE

#define USE_TICKER
//#define USE_TIMER_ONE
//#define USE_TIMER_THREE

#define BLYNK_NO_DEFAULT_BANNER

#if defined(APP_DEBUG)
  #define DEBUG_PRINT(...) BLYNK_LOG1(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif
