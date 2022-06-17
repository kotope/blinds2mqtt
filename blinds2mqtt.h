// wifi settings
const char* ssid     = "****"; // Your WiFi SSID
const char* password = "****"; // Your WiFi password

// Servo pins to use (each servo to be placed on it's own IO pin)
// To set servos turn as reversed, set reversed pins array to wich servos should turn opposite direction. example: { D7, D6 }
// Reversed servos should be subset of servoPins e.g. reversedPin must be also in servoPins array!
const unsigned int servoPins[] = { D7, D6, D5 };
const unsigned int reversedPins[] = { }; // Array of reversed servo pins. Must be subset of servoPins.
const unsigned int turnTime = 50; // how many milliseconds after one point of turn (must be greater than amount of turn time of all the servos)

// mqtt server settings
const char* mqtt_server   = "192.168.1.*"; // Your MQTT server address
const int mqtt_port       = 1883; // Your MQTT server port
const char* mqtt_username = "****"; // Your MQTT user
const char* mqtt_password = "****"; // Your MQTT password

// mqtt client settings
const char* client_id = "blinds"; // Must be unique on the MQTT network
const boolean retain_status = true; // Retain status messages (keeps blinds status available after HA reset)
const boolean retain_position = true; // Retain position messages

// Home assistant configuration
// Friendly name of the device. If using multiple servos, a number will be appended at the end of the name e.g. "Blinds 1", "Blinds 2"
const String friendly_name = "Blinds";

// -- Below should not be changed if using same servos as mentioned in the  blog post
// Servo settings
const int servo_min_pulse = 500;
const int servo_max_pulse = 2500;
const int servo_max_angle = 270;

// OTA Settings
const char* ota_password = "BlindsOTA";

// Mqtt topics (for advanced use, no need to modify)
const char* blinds_state_topic        = "blinds/%s/%d/state";
const char* blinds_command_topic      = "blinds/%s/%d/set";
const char* blinds_position_topic     = "blinds/%s/%d/position";
const char* blinds_set_position_topic = "blinds/%s/%d/position/set";
const char* blinds_debug_topic        = "blinds/%s/debug"; // debug topic
const char* ha_config_topic           = "homeassistant/cover/%s/%d/config";
