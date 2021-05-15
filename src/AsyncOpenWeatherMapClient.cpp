
#include "AsyncOpenWeatherMapClient.h"
#include <ESPAsyncTCP.h>
#include <WiFiClient.h>

AsyncOpenWeatherMapClient::AsyncOpenWeatherMapClient() {}

boolean AsyncOpenWeatherMapClient::getForecastsById(
    const char *appId, const char *locationId,
    OpenWeatherMapForecastDataCallback cb,
    OpenWeatherMapErrorCallback errorcb) {
    if (isInProgress()) {
        Log.notice("request pending");
        return false;
    }
    listener.setCb(cb);
    listener.setErrorCb(errorcb);
    parser.setListener(&listener);
    sprintf(url, "/data/2.5/forecast?id=%s&appid=%s&units=%s&lang=%s",
            locationId, appId, (metric ? "metric" : "imperial"), language);
    return get(&listener);
}

boolean AsyncOpenWeatherMapClient::getForecastsByZip(
    const char *appId, const char *zip,
    OpenWeatherMapForecastDataCallback cb,
    OpenWeatherMapErrorCallback errorcb) {
    if (isInProgress()) {
        Log.notice("request pending");
        return false;
    }
    listener.setCb(cb);
    listener.setErrorCb(errorcb);
    parser.setListener(&listener);
    sprintf(url, "/data/2.5/forecast?zip=%s&appid=%s&units=%s&lang=%s",
            zip, appId, (metric ? "metric" : "imperial"), language);
    return get(&listener);
}

boolean AsyncOpenWeatherMapClient::getUVIForecasts(
    const char *appId, const char *lat, const char *lon,
    OpenWeatherMapUVIForecastDataCallback cb,
    OpenWeatherMapErrorCallback errorcb) {
    if (isInProgress()) {
        Log.notice("request pending");
        return false;
    }
    uvilistener.setCb(cb);
    uvilistener.setErrorCb(errorcb);
    parser.setListener(&uvilistener);
    sprintf(url,
            "/data/2.5/uvi/forecast?lat=%s&lon=%s&appid=%s&units=%s&lang=%s",
            lat, lon, appId, (metric ? "metric" : "imperial"), language);
    return get(&uvilistener);
}


boolean AsyncOpenWeatherMapClient::getOneCallForecasts(
    const char *appId, const char *lat, const char *lon,
    OpenWeatherMapOneCallDailyDataCallback cb,
    OpenWeatherMapErrorCallback errorcb) {
    if (isInProgress()) {
        Log.notice("request pending");
        return false;
    }
    onecalllistener.setCb(cb);
    onecalllistener.setErrorCb(errorcb);
    parser.setListener(&onecalllistener);
    sprintf(url,
            "/data/2.5/onecall?lat=%s&lon=%s&appid=%s&units=%s&lang=%s",
            lat, lon, appId, (metric ? "metric" : "imperial"), language);
    return get(&onecalllistener);
}

boolean AsyncOpenWeatherMapClient::get(OpenWeatherMapListener *jsonlistener) {
    reset();
    Log.notice("url: %s\n", url);

    AsyncClient *aClient = new AsyncClient();
    if (!aClient)  // could not allocate client
        return false;
    setInProgress();
    aClient->onError(
        [this, jsonlistener](void *arg, AsyncClient *client, int error) {
            Log.notice("Connect Error");
            setIdle();
            jsonlistener->error(1);
            delete client;
        },
        NULL);

    aClient->onConnect(
        [this, jsonlistener](void *arg, AsyncClient *client) {
            Log.notice("Connected\n");
            // aClient->onError(NULL, NULL);

            client->onDisconnect(
                [this](void *arg, AsyncClient *c) {
                    Log.notice("Disconnected\n");
                    setIdle();
                    delete c;
                },
                NULL);

            client->onData(
                [this, jsonlistener](void *arg, AsyncClient *c, void *data,
                                     size_t len) {
                    uint8_t *d = (uint8_t *)data;
                    Log.notice("Data: %d\n", len);
                    Log.notice((const char *)d, len);
                    Log.notice("\n");

                    for (size_t i = 0; i < len; i++) {
                        if (statuscode == 0) {
                            // HTTP/1.1 200 OK
                            // look for first ' ' ;)
                            if (d[i] == ' ') {
                                char code[4];
                                code[3] = '\0';
                                code[0] = d[i + 1];
                                code[1] = d[i + 2];
                                code[2] = d[i + 3];
                                statuscode = atoi(code);
                                Log.notice("statuscode: %d\n", statuscode);
                                if (statuscode != 200) {
                                    jsonlistener->error(statuscode);
                                }
                            }
                        }
                        if (d[i] == '{' || d[i] == '[') {
                            isBody = true;
                        }
                        if (isBody) {
                            parser.parse(d[i]);
                        }
                        // Serial.write(d[i]);
                    }
                },
                NULL);

            // send the request
            Log.notice("client url: %s\n", this->url);
            client->write("GET ");
            client->write(this->url);
            client->write(" HTTP/1.0\r\nHost: ");
            client->write(_hostname);
            client->write("\r\n\r\n");
        },
        NULL);

    if (!aClient->connect(_hostname, 80)) {
        Log.notice("Connect Fail");
        AsyncClient *client = aClient;
        aClient = NULL;
        setIdle();
        jsonlistener->error(1);
        delete client;
    }
    return true;
}
