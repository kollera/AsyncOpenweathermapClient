
#ifndef ASYNCOPENWEATHERMAPCLIENT_H
#define ASYNCOPENWEATHERMAPCLIENT_H

#include <ArduinoLog.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <stdio.h>

#include <functional>

typedef struct OpenWeatherMapForecastData {
    uint32_t dt;
    float temp;
    float temp_min;
    float temp_max;
    float pressure;
    float sea_level;
    float grnd_level;
    uint8_t humidity;
    uint16_t id;
    char main[32];
    char description[32];
    char icon[4];
    uint8_t clouds;
    float wind_speed;
    float wind_deg;
    float rain;
    char dt_txt[20];
} OpenWeatherMapForecastData;

typedef struct OpenWeatherMapUVIForecastData {
    uint32_t date;
    float lat;
    float lon;
    float value;
} OpenWeatherMapUVIForecastData;

typedef struct OpenWeatherMapOneCallDailyData {
    uint32_t dt;
    float temp_day;
    float temp_night;
    float temp_min;
    float temp_max;
    float pressure;
    float humidity;
    float clouds;
    float pop;
    float rain;
    float uvi;
} OpenWeatherMapOneCallDailyData;

typedef std::function<void(OpenWeatherMapForecastData*, uint8_t)>
    OpenWeatherMapForecastDataCallback;
typedef std::function<void(OpenWeatherMapUVIForecastData*, uint8_t)>
    OpenWeatherMapUVIForecastDataCallback;
typedef std::function<void(int)> OpenWeatherMapErrorCallback;

typedef std::function<void(OpenWeatherMapOneCallDailyData*, uint8_t)>
    OpenWeatherMapOneCallDailyDataCallback;

class OpenWeatherMapListener : public JsonListener {
   private:
    OpenWeatherMapErrorCallback errorcb = nullptr;

   public:
    void setErrorCb(OpenWeatherMapErrorCallback cb) { errorcb = cb; }
    void error(int error) {
        if (errorcb != nullptr) {
            errorcb(error);
        }
    }
};

class AOpenWeatherMapForecastListener : public OpenWeatherMapListener {
   private:
    String currentKey;
    String currentParent;
    uint8_t weatherItemCounter = 0;
    OpenWeatherMapForecastData dataObj;
    OpenWeatherMapForecastData* data;
    OpenWeatherMapForecastDataCallback weatherback = nullptr;

   public:
    AOpenWeatherMapForecastListener() { data = &dataObj; }

    void setCb(OpenWeatherMapForecastDataCallback cb) { weatherback = cb; }
    void whitespace(char c) {}

    void startDocument() {}

    void key(String key) { currentKey = String(key); }

    void value(String value) {
        if (currentKey == "dt") {
            data->dt = value.toInt();
        } else if (currentKey == "temp") {
            data->temp = value.toFloat();
            // initialize potentially empty values:
            data->rain = 0;
            ;
        } else if (currentKey == "temp_min") {
            data->temp_min = value.toFloat();
        } else if (currentKey == "temp_max") {
            data->temp_max = value.toFloat();
        } else if (currentKey == "pressure") {
            data->pressure = value.toFloat();
        } else if (currentKey == "sea_level") {
            data->sea_level = value.toFloat();
        } else if (currentKey == "grnd_level") {
            data->grnd_level = value.toFloat();
        } else if (currentKey == "humidity") {
            data->humidity = value.toInt();
        } else if (currentKey == "all") {
            data->clouds = value.toInt();
        } else if (currentKey == "speed") {
            data->wind_speed = value.toFloat();
        } else if (currentKey == "deg") {
            data->wind_deg = value.toFloat();
        } else if (currentKey == "3h") {
            data->rain = value.toFloat();
        } else if (currentKey == "dt_txt") {
            strcpy(data->dt_txt, value.c_str());
            if (weatherback != nullptr) {
                weatherback(data, weatherItemCounter);
            }
        }

        if (currentParent == "weather") {
            if (currentKey == "id") {
                data->id = value.toInt();
            } else if (currentKey == "main") {
                strcpy(data->main, value.c_str());
            } else if (currentKey == "description") {
                strcpy(data->description, value.c_str());
            } else if (currentKey == "icon") {
                strcpy(data->icon, value.c_str());
            }
        }
    }

    void endArray() {}

    void startObject() { currentParent = currentKey; }

    void endObject() {
        if (currentParent == "weather") {
            weatherItemCounter++;
        }
        currentParent = "";
    }

    void endDocument() {}

    void startArray() {}
};

class AOpenWeatherMapUVIForecastListener : public OpenWeatherMapListener {
   private:
    String currentKey;
    String currentParent;
    uint8_t weatherItemCounter = 0;
    OpenWeatherMapUVIForecastData dataObj;
    OpenWeatherMapUVIForecastData* data;
    OpenWeatherMapUVIForecastDataCallback weatherback = nullptr;

   public:
    AOpenWeatherMapUVIForecastListener() { data = &dataObj; }

    void setCb(OpenWeatherMapUVIForecastDataCallback cb) { weatherback = cb; }
    void whitespace(char c) {}

    void startDocument() {}

    void key(String key) { currentKey = String(key); }

    void value(String value) {
        if (currentKey == "lat") {
            weatherItemCounter++;
            data->lat = value.toFloat();
        } else if (currentKey == "lon") {
            data->lon = value.toFloat();
        } else if (currentKey == "date") {
            data->date = value.toInt();
        } else if (currentKey == "value") {
            data->value = value.toFloat();
        }

        if (currentKey == "value") {
            if (weatherback != nullptr) {
                weatherback(data, weatherItemCounter);
            }
        }
    }

    void endArray() {}

    void startObject() { currentParent = currentKey; }

    void endObject() {}

    void endDocument() {}

    void startArray() {}
};

class AOpenWeatherMapOneCallListener : public OpenWeatherMapListener {
   private:
    String currentKey;
    bool dailyArray;
    String currentParent;
    int objcounter = 0;
    uint8_t weatherItemCounter = 0;
    OpenWeatherMapOneCallDailyData dataObj;
    OpenWeatherMapOneCallDailyData* data;
    OpenWeatherMapOneCallDailyDataCallback weatherback = nullptr;
    char* _urltemplate = "";

   public:
    AOpenWeatherMapOneCallListener() { data = &dataObj; }

    void setUrlTemplate(char* urltemplate) {
        _urltemplate = urltemplate;
    }
    char* getUrlTemplate() {
        get _urltemplate;
    }

    void setCb(OpenWeatherMapOneCallDailyDataCallback cb) { weatherback = cb; }
    void whitespace(char c) {}

    void startDocument() {
        objcounter = 0;
        weatherItemCounter = 0;
        dailyArray = false;
    }

    void key(String key) { currentKey = String(key); }

    void value(String value) {
        if (dailyArray) {
            if (currentKey == "dt") {
                data->dt = value.toInt();
            } else if (currentKey == "day") {
                data->temp_day = value.toFloat();
            } else if (currentKey == "night") {
                data->temp_night = value.toFloat();
            } else if (currentKey == "rain") {
                data->rain = value.toFloat();
            } else if (currentKey == "pop") {
                data->pop = value.toFloat();
            } else if (currentKey == "clouds") {
                data->clouds = value.toFloat();
            } else if (currentKey == "pressure") {
                data->pressure = value.toFloat();
            } else if (currentKey == "uvi") {
                data->uvi = value.toFloat();
            } else if (currentKey == "humidity") {
                data->humidity = value.toFloat();
            } else if (currentKey == "min") {
                data->temp_min = value.toFloat();
            } else if (currentKey == "max") {
                data->temp_max = value.toFloat();
            }
        }
    }

    void endArray() {
        if (dailyArray && objcounter == 0) {
            dailyArray = false;
        }
    }

    void startObject() {
        currentParent = currentKey;
        if (dailyArray) {
            objcounter++;
        }
    }

    void endObject() {
        if (dailyArray) {
            objcounter--;
            if (objcounter == 0) {
                weatherItemCounter++;
                if (weatherback != nullptr) {
                    weatherback(data, weatherItemCounter);
                }
            }
        }
    }

    void endDocument() {}

    void startArray() {
        if (currentKey == "daily") {
            dailyArray = true;
        }
    }
};

class AsyncOpenWeatherMapClient {
   private:
    AOpenWeatherMapForecastListener listener;
    AOpenWeatherMapUVIForecastListener uvilistener;
    AOpenWeatherMapOneCallListener onecalllistener;
    boolean metric = true;
    const char* language = "en";
    JsonStreamingParser parser;
    boolean isBody = false;
    int statuscode = 0;
    boolean inProgress = false;
    char url[96];
    const char* _hostname;

    boolean get(OpenWeatherMapListener* jsonlistener);
    void setInProgress() { inProgress = true; }
    void setIdle() { inProgress = false; }
    boolean isInProgress() { return inProgress; }

   public:
    AsyncOpenWeatherMapClient() {
	_hostname = "api.openweathermap.org";
    }
    AsyncOpenWeatherMapClient(const char* hostname) {
        _hostname = hostname;
    }
    boolean getUVIForecasts(const char* appId, const char* lat, const char* lon,
                            OpenWeatherMapUVIForecastDataCallback cb,
                            OpenWeatherMapErrorCallback errorcb);
    boolean getForecastsById(const char* appId, const char* locationId,
                             OpenWeatherMapForecastDataCallback cb,
                             OpenWeatherMapErrorCallback errorcb);
    boolean getForecastsByZip(const char* appId, const char* zip_countrycode,
                              OpenWeatherMapForecastDataCallback cb,
                              OpenWeatherMapErrorCallback errorcb);
    boolean getOneCallForecasts(const char* appId, const char* lat,
                                const char* lon,
                                OpenWeatherMapOneCallDailyDataCallback cb,
                                OpenWeatherMapErrorCallback errorcb);

    void reset() {
        parser.reset();
        isBody = false;
        statuscode = 0;
    }
    void setMetric(boolean metric) { this->metric = metric; }
    boolean isMetric() { return this->metric; }
    void setLanguage(const char* language) { this->language = language; }
    const char* getLanguage() { return this->language; }
};

#endif
