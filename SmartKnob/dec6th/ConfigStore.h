/* Part of an open source template for Blynk.Inject for ESP32, provided by Blynk via GitHub */


struct ConfigStore {
  uint32_t  magic;
  char      version[9];
  uint8_t   flagConfig:1;
  uint8_t   flagApFail:1;
  uint8_t   flagSelfTest:1;

  char      wifiSSID[34];
  char      wifiPass[64];

  char      cloudToken[34];
  char      cloudHost[34];
  uint16_t  cloudPort;

  uint16_t  checksum;
} __attribute__((packed));

ConfigStore configStore;

const ConfigStore configDefault = {
  0x626C6E6B,
  BOARD_FIRMWARE_VERSION,
  0, 0, 0,
  
  "",
  "",
  
  "invalid token",
  "blynk-cloud.com", 80,
  0
};

#include <Preferences.h>
Preferences preferences;

void config_load()
{
  memset(&configStore, 0, sizeof(configStore));
  preferences.getBytes("config", &configStore, sizeof(configStore));
  if (configStore.magic != configDefault.magic) {
    DEBUG_PRINT("Using default config.");
    configStore = configDefault;
    return;
  }
}

bool config_save()
{
  preferences.putBytes("config", &configStore, sizeof(configStore));
  return true;
}

bool config_init()
{
  preferences.begin("blynk", false);
  config_load();
  return true;
}

void enterResetConfig()
{
  DEBUG_PRINT("Resetting configuration!");
  configStore = configDefault;
  config_save();
  BlynkState::set(MODE_WAIT_CONFIG);
}

template<typename T, int size>
void CopyString(const String& s, T(&arr)[size]) {
  s.toCharArray(arr, size);
}
