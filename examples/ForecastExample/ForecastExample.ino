
#include <AsyncOpenWeatherMapClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

const char* OPEN_WEATHER_MAP_APP_ID = "insert-app-id";
const char* OPEN_WEATHER_MAP_LOCATION_ID = "insert-location-id";
const char* OPEN_WEATHER_MAP_LANGUAGE = "en";
const char* OPEN_WEATHER_LAT = "insert-lat";
const char* OPEN_WEATHER_LON = "insert-lon";
AsyncOpenWeatherMapClient client;

void setup() {
    Serial.begin(115200);
    // Log.begin(LOG_LEVEL_NOTICE, &Serial);

    Log.notice("Connecting to ");
    Log.notice(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Log.notice("\n");
    Log.notice("WiFi connected\n");
    Log.notice("IP address: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
    static bool requested = false;
    if (!requested) {
        client.setMetric(true);
        client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);

        client.getForecastsById(
            OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID,
            [](OpenWeatherMapForecastData* d, uint8_t count) {
                Serial.println("---- entry ------\n");
                Serial.println(count);
                Serial.print("weather id: ");
                Serial.println(d->id);
                Serial.print("weather dt: ");
                Serial.println(d->dt);
                Serial.print("weather main: ");
                Serial.println(d->main);
                Serial.print("weather pressure: ");
                Serial.println(d->pressure);
                Serial.print("weather grnd_level: ");
                Serial.println(d->grnd_level);
                Serial.print("weather sea_level: ");
                Serial.println(d->sea_level);
                Serial.print("weather rain: ");
                Serial.println(d->rain);
                Serial.print("weather temp: ");
                Serial.println(d->temp);
                Serial.print("weather temp_max: ");
                Serial.println(d->temp_max);
                Serial.print("weather temp_min: ");
                Serial.println(d->temp_min);
                Serial.print("weather wind_deg: ");
                Serial.println(d->wind_deg);
                Serial.print("weather wind_speed: ");
                Serial.println(d->wind_speed);
            },
            [](int error) {
                Serial.print("weather update error: ");
                Serial.println(error);
            });
        requested = true;
    }
}