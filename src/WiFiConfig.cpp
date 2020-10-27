#include "WiFiConfig.h"

const byte DNS_PORT = 53;
const char magicBytes[MAGIC_LENGTH] = {'C', 'M'};
const char magicBytesEmpty[MAGIC_LENGTH] = {'\0', '\0'};

const char mimeHTML[] PROGMEM = "text/html";
const char mimeJSON[] PROGMEM = "application/json";
const char mimePlain[] PROGMEM = "text/plain";
const char mimeCSS[] PROGMEM = "text/css";
const char mimeJS[] PROGMEM = "application/javascript";

ConfigManager::ConfigManager()
{
    this->setAPFilename("/index.html");
}

void ConfigManager::setAPName(const char *name)
{
    this->apName = (char *)name;
}

void ConfigManager::setVersionName(const int name)
{
    this->VersionName = name;
}

void ConfigManager::setAPPassword(const char *password)
{
    this->apPassword = (char *)password;
}

void ConfigManager::setAPFilename(const char *filename)
{
    this->apFilename = (char *)filename;
}

void ConfigManager::setAPTimeout(const int timeout)
{
    this->apTimeout = timeout;
}

void ConfigManager::setWifiConnectRetries(const int retries)
{
    this->wifiConnectRetries = retries;
}

void ConfigManager::setWifiConnectInterval(const int interval)
{
    this->wifiConnectInterval = interval;
}

void ConfigManager::setAPCallback(std::function<void(WebServer *)> callback)
{
    this->apCallback = callback;
}

void ConfigManager::setAPICallback(std::function<void(WebServer *)> callback)
{
    this->apiCallback = callback;
}

/**
 * @brief
 *
 */
void ConfigManager::loop()
{
    if (mode == MODE_AP && apTimeout > 0 && ((millis() - apStart) / 1000) > apTimeout)
    {
        ESP.restart();
    }

    if (dnsServer)
    {
        dnsServer->processNextRequest();
    }

    if (server)
    {
        server->handleClient();
    }
}

/**
 * @brief
 *
 */
void ConfigManager::save()
{
    this->writeConfig();
}

/**
 * @brief
 *
 * @param jsonString
 * @return JsonObject&
 */
JsonObject ConfigManager::decodeJson(String jsonString)
{
    DynamicJsonDocument doc(1024);

    if (jsonString.length() == 0)
    {
        return doc.as<JsonObject>();
    }

    auto error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return doc.as<JsonObject>();
    }

    return doc.as<JsonObject>();
}

/**
 * Respond with content of index.html
 */
void ConfigManager::handleGetRoot()
{
    SPIFFS.begin();

    File f = SPIFFS.open(apFilename, "r");
    if (!f)
    {
        Serial.print(F("file "));
        Serial.print(apFilename);
        Serial.print(F(" open failed"));
        server->send(404, FPSTR(mimeHTML), F("File not found"));
        return;
    }

    server->streamFile(f, FPSTR(mimeHTML));

    f.close();
}

/**
 *
 */
void ConfigManager::handleReboot()
{
    server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

    // Restart server
    ESP.restart();
}

/**
 * Return current WiFi mode
 */
void ConfigManager::handleGetWifi()
{
    DynamicJsonDocument jsonBuffer(512);
    JsonObject res = jsonBuffer.as<JsonObject>();

    // WiFi mode
    switch (WiFi.getMode())
    {
    case WIFI_OFF:
        // res.set("mode", "off");
        // res.getOrAddMember("mode").set("off");
        res["mode"] = "off";
        break;
    case WIFI_STA:
        // res.set("mode", "sta");
        // res.getOrAddMember("mode").set("sta");
        res["mode"] = "sta";
        break;
    case WIFI_AP:
        // res.set("mode", "ap");
        // res.getOrAddMember("mode").set("ap");
        res["mode"] = "ap";
        break;
    case WIFI_AP_STA:
        // res.set("mode", "ap_sta");
        // res.getOrAddMember("mode").set("ap_sta");
        res["mode"] = "ap_sta";
        break;
    default:
        // res.set("mode", "");
        // res.getOrAddMember("mode").set("");
        res["mode"] = "";
        break;
    }

    // connection status
    // res.set("connected", WiFi.isConnected());
    // res.getOrAddMember("connected").set(WiFi.isConnected());
    res["connected"] = WiFi.isConnected();

    String body;
    // res.printTo(body);
    serializeJson(res, body);

    server->send(200, FPSTR(mimeJSON), body);
}

/**
 * @brief Scan WiFi networks
 *
 */
void ConfigManager::handleGetWifiScan()
{
    DynamicJsonDocument doc(1024);
    JsonArray jsonArray = doc.createNestedArray();

    Serial.println("Scanning WiFi networks...");
    int n = WiFi.scanNetworks();
    Serial.println("scan complete");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found:");

        for (int i = 0; i < n; ++i)
        {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            String security =
                WiFi.encryptionType(i) == WIFI_OPEN ? "none" : "enabled";

            Serial.print("Name: ");
            Serial.print(ssid);
            Serial.print(" - Strength: ");
            Serial.print(rssi);
            Serial.print(" - Security: ");
            Serial.println(security);

            JsonObject obj = doc.createNestedObject();
            obj["ssid"] = ssid;
            obj["strength"] = rssi;
            obj["security"] = security == "none" ? false : true;
            jsonArray.add(obj);
        }
    }

    String jsonSerialized;
    serializeJson(jsonArray, jsonSerialized);
    server->send(200, FPSTR(mimeJSON), jsonSerialized);
}

void ConfigManager::storeWifiSettings(String ssid,
                                      String password)
{
    char ssidChar[SSID_LENGTH];
    char passwordChar[PASSWORD_LENGTH];

    if (!this->memoryInitialized)
    {
        Serial.println(
            F("WiFi Settings cannot be stored before ConfigManager::begin()"));
        return;
    }

    strlcpy(ssidChar, ssid.c_str(), SSID_LENGTH);
    strlcpy(passwordChar, password.c_str(), PASSWORD_LENGTH);

    Serial.print(F("Storing WiFi Settings for SSID: \""));
    Serial.print(ssidChar);
    Serial.println(F("\""));

    EEPROM.put(MAGIC_LENGTH, ssidChar);
    EEPROM.put(MAGIC_LENGTH + SSID_LENGTH, passwordChar);
    bool wroteChange = this->commitChanges();

    Serial.print(F("EEPROM committed: "));
    Serial.println(wroteChange ? F("true") : F("false"));
}

/**
 * @brief Connect to the Access Point
 *
 */
void ConfigManager::handlePostConnect()
{
    Serial.print(F("收到请求了"));
    bool isJson = server->header("Content-Type") == FPSTR(mimeJSON);
    Serial.print(isJson);
    String ssid;
    String password;
    Serial.print(F("信号正常1"));

    if (isJson)
    {
        DynamicJsonDocument doc(1024);

        Serial.print(F("信号正常2"));

        deserializeJson(doc, server->arg("plain"));
        JsonObject obj = doc.as<JsonObject>();

        Serial.print("OBJ:");
        serializeJson(obj, Serial);
        password = obj["password"].as<String>();
        Serial.print(password);

        ssid = obj["ssid"].as<String>();
        Serial.print(F("信号正常4"));
    }
    else
    {
        ssid = server->arg("ssid");
        Serial.print(ssid);
        password = server->arg("password");
        Serial.print(password);
    }

    if (ssid.length() == 0)
    {
        server->send(400, FPSTR(mimeHTML), F("Invalid ssid."));
        return;
    }

    this->storeWifiSettings(ssid, password);

    server->send(201, FPSTR(mimeHTML), F("Saved. Will attempt to reboot."));

    ESP.restart();
}

/**
 * @brief Disconnect from the access point
 *
 */
void ConfigManager::handlePostDisconnect()
{
    char ssidChar[SSID_LENGTH] = {0};
    char passwordChar[PASSWORD_LENGTH] = {0};

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // clear saved connection parameters
    EEPROM.put(0, magicBytes);
    EEPROM.put(WIFI_OFFSET, ssidChar);
    EEPROM.put(WIFI_OFFSET + 32, passwordChar);
    EEPROM.commit();

    server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

    // Restart server
    ESP.restart();
}

/**
 * Return JSON with data schema
 */
void ConfigManager::handleGetSettingsSchema()
{
    DynamicJsonDocument jsonBuffer(2048);
    JsonArray res = jsonBuffer.createNestedArray();

    std::list<ConfigParameterGroup *>::iterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        JsonObject obj = res.createNestedObject();
        (*it)->toJsonSchema(&obj);
    }

    String body;
    // res.printTo(body);
    serializeJson(res, body);
    Serial.print("json data schema:");
    serializeJson(res, Serial);

    server->send(200, FPSTR(mimeJSON), body);
}

/**
 * Return JSON with settings values
 */
void ConfigManager::handleGetSettings()
{
    Serial.print("Received the get settings request");
    DynamicJsonDocument jsonBuffer(2048);
    JsonObject res = jsonBuffer.createNestedObject();

    std::list<ConfigParameterGroup *>::iterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        // if ((*it)->getMode() == set)
        // {
        //     continue;
        // }
        (*it)->toJson(&res);
    }

    String body;
    // res.printTo(body);
    serializeJson(res, body);
    Serial.print("get data:");
    serializeJson(res, Serial);
    server->send(200, FPSTR(mimeJSON), body);
}

/**
 * @brief Update settings
 *
 */
void ConfigManager::handlePostSettings()
{
    DynamicJsonDocument doc(1024);
    Serial.print("Received post settings request...");
    // JsonObject obj = this->decodeJson(server->arg("plain"));

    auto error = deserializeJson(doc, server->arg("plain"));
    if (error)
    {
        server->send(400, FPSTR(mimeJSON), "");
        Serial.print("Bad requests");
        return;
    }

    JsonObject obj = doc.as<JsonObject>();
    Serial.print("obj=");
    serializeJson(obj, Serial);

    std::list<ConfigParameterGroup *>::iterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {

        (*it)->fromJson(&obj);
    }

    writeConfig();
    Serial.println("Wrote settings");
    server->send(201, FPSTR(mimeHTML), "OK");
    delay(1000);
    ESP.restart();
}

/**
 * Clear settings
 */
void ConfigManager::handleDeleteSettings()
{
    EEPROM.put(0, magicBytes);
    EEPROM.commit();

    server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

    // Restart server
    ESP.restart();
}

/**
 * @brief Handle 404 Not Found error
 *
 */
void ConfigManager::handleNotFound()
{
    if (!isIp(server->hostHeader()))
    {
        server->sendHeader("Location", String("http://") + toStringIP(server->client().localIP()), true);
        server->send(302, FPSTR(mimePlain), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop();
        return;
    }

    server->send(404, FPSTR(mimePlain), "");
    server->client().stop();
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool ConfigManager::wifiConnected()
{
    Serial.print(F("Waiting for WiFi to connect"));

    int i = 0;
    while (i < wifiConnectRetries)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("");
            return true;
        }

        Serial.print(".");

        delay(wifiConnectInterval);
        i++;
    }

    Serial.println("");
    Serial.println(F("Connection timed out"));

    return false;
}

/**
 * @brief
 *
 */
void ConfigManager::setup()
{
    char magic[MAGIC_LENGTH];
    char ssid[SSID_LENGTH];
    char password[PASSWORD_LENGTH];

    //
    Serial.println(F("Reading saved configuration"));

    EEPROM.get(0, magic);

    if (memcmp(magic, magicBytes, 2) == 0)
    {
        // config read sucessfully
        EEPROM.get(WIFI_OFFSET, ssid);
        EEPROM.get(WIFI_OFFSET + 32, password);

        readConfig();

        Serial.println(F("Config read successfully"));
    }
    else
    {
        // config is incorrect, save default one
        EEPROM.put(0, magicBytes);
        EEPROM.put(WIFI_OFFSET, ssid);
        EEPROM.put(WIFI_OFFSET + 32, password);

        writeConfig();

        Serial.println(F("Config incorrect, overwriting"));
    }

    // try to connect to access point
    if (ssid[0] != '\0')
    {
        WiFi.begin(ssid, password[0] == '\0' ? NULL : password);

        delay(1000);

        if (wifiConnected())
        {
            startApi(ssid);
        }
    }

    // could not connect to AP or there is no configuration for it
    if (WiFi.status() != WL_CONNECTED)
    {
        apTimeout = 0;

        startAP();
    }

    //
    startWebServer();
    Serial.print("服务器开启了");
}

/**
 * @brief
 *
 */
void ConfigManager::startWebServer()
{
    const char *headerKeys[] = {"Content-Type"};
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char *);

    server.reset(new WebServer(80));

    server->collectHeaders(headerKeys, headerKeysSize);

    server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetRoot, this));
    server->on("/home", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetRoot, this));
    server->on("/set", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetRoot, this));
    server->on("/status", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetRoot, this));
    server->on("/update", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleOtaUpdate, this));

    server->on("/reboot", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleReboot, this));

    // wifi parameters
    server->on("/wifi", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetWifi, this));
    server->on("/wifi/scan", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetWifiScan, this));
    server->on("/wifi/connect", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handlePostConnect, this));
    server->on("/wifi/disconnect", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handlePostDisconnect, this));

    // configuration settings
    server->on("/settings", HTTPMethod::HTTP_OPTIONS, std::bind(&ConfigManager::handleGetSettingsSchema, this));
    server->on("/settings", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetSettings, this));
    server->on("/settings", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handlePostSettings, this));
    server->on("/settings", HTTPMethod::HTTP_DELETE, std::bind(&ConfigManager::handleDeleteSettings, this));

    // not found handling
    server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));

    if (apCallback)
    {
        apCallback(server.get());
    }

    server->begin();
    this->webserverRunning = true;
}

/**
 * @brief
 *
 */
void ConfigManager::startAP()
{
    mode = MODE_AP;

    Serial.println(F("Starting Access Point"));

    WiFi.disconnect();
    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_OFF);

    delay(500);

    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAP(apName, apPassword);

    delay(1000); // Need to wait to get IP
    WiFi.softAPConfig(ip, ip, NMask);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    dnsServer.reset(new DNSServer);
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, "*", IPAddress(192, 168, 1, 1));

    apStart = millis();
}

void ConfigManager::handleOtaUpdate()
{
    Serial.print("Updating firmware, please wait ... ");
    M5.Lcd.setBrightness(100);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 18);
    M5.Lcd.setTextSize(1);
    // M5.Lcd.setFreeFont(FMB9);
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.println("FIRMWARE UPDATE");
    M5.Lcd.println();
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    M5.Lcd.print("Free Heap: ");
    M5.Lcd.println(ESP.getFreeHeap());
    M5.Lcd.println();

    String message  = "<!DOCTYPE HTML>\r\n";
    message += "<html>\r\n";
    message += "<head>\r\n";
    message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
    message += "<meta http-equiv=\"refresh\" content=\"30;url=/\" />\r\n";
    message += "<style>\r\n";
    message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
    message += "</style>\r\n";
    message += "<title>TomatoM5 - ";
    message += "</title>\r\n";
    message += "</head>\r\n";
    message += "<body>\r\n";

    HTTPClient http;

    String binUrl;
    int needUpgrade = 1;
    int err;
    int webVer = 0 ;

    if (WiFi.status() == WL_CONNECTED)
    {
        http.begin("https://app.tomato.cool/5013");         //Specify destination for HTTP request
        http.addHeader("Content-Type", "application/json"); //Specify content-type header
        char tmpstr[64];

        sprintf(tmpstr, "{\"fwversion\" : %d, \"miaomiao_v\" :4}", this->VersionName);
        String jsonString = tmpstr;

        int httpCode = http.POST(jsonString);
        if (httpCode > 0)
        {
            if (httpCode == 200)
            {
                String json = http.getString();
                Serial.print(json);
                // remove any non text characters (just for sure)
                for (int i = 0; i < json.length(); i++)
                {
                    // Serial.print(json.charAt(i), DEC); Serial.print(" = "); Serial.println(json.charAt(i));
                    if (json.charAt(i) < 32 /* || json.charAt(i)=='\\' */)
                    {
                        json.setCharAt(i, 32);
                    }
                }
                DynamicJsonDocument JSONdoc(512);
                DeserializationError JSONerr = deserializeJson(JSONdoc, json);
                if (JSONerr)
                { //Check for errors in parsing
                    if (JSONerr)
                    {
                        err = 1001; // "JSON parsing failed"
                    }
                    else
                    {
                        err = 1002; // "No data from Nightscout"
                    }
                    Serial.print("err:");
                    Serial.println(err);
                }
                else
                {
                    Serial.println("JSON deserialized OK");
                    needUpgrade = JSONdoc["data"]["need_upgrade"];
                    binUrl = JSONdoc["data"]["file_url"].as<String>();
                    Serial.println(binUrl);
                    webVer = JSONdoc["data"]["new_fwversion"];
                }
            }
        }
        http.end();
        // return;
    }
    delay(1000);
    if (WiFi.status() == WL_CONNECTED)
    {
        if (needUpgrade == 1)
        {
            message += "<p>Updating firmware to version ";
            message += webVer;
            message += ", please wait ... </p>\r\n";
            message += "<p>Device will restart automatically.</p>\r\n";
            message += "</body>\r\n";
            message += "</html>\r\n";
            server->send(200, "text/html", message);


            M5.Lcd.println();
            M5.Lcd.println("Updating the firmware... ");
            M5.Lcd.println();
            ESPhttpUpdate.rebootOnUpdate(false);
            delay(100);
            Serial.print("binUrl:");
            Serial.println(binUrl);
            t_httpUpdate_return ret = ESPhttpUpdate.update(binUrl);

            switch (ret)
            {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                M5.Lcd.setTextColor(RED);
                M5.Lcd.println("UPDATE FAILED");
                delay(1000);
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                M5.Lcd.setTextColor(YELLOW);
                M5.Lcd.println("NO UPDATES");
                delay(1000);
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK, restarting ...");
                M5.Lcd.setTextColor(GREEN);
                M5.Lcd.println("UPDATED SUCCESSFULLY");
                M5.Lcd.println("Restarting ...");
                delay(1000);
                M5.update();
                delay(1000);
                ESP.restart();
                break;
            }
        }
        else
        {
            message += "<p>Nothing to update. Firmware version ";
            message += webVer;
            message += " is current.</p>\r\n";
            message += "</body>\r\n";
            message += "</html>\r\n";
            server->send(200, "text/html", message);
            Serial.println("Nothing to update");
            M5.Lcd.println();
            M5.Lcd.setTextColor(YELLOW);
            M5.Lcd.println("NOTHING TO UPDATE");
            return;
        }
        return;
    }
    else
    {
        Serial.println("HTTP error: connection refused");
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.println("HTTP error: connection refused");
        // return;
    }

    M5.update();
    delay(2000);
    M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief
 *
 * @param ssid
 */
void ConfigManager::startApi(const char *ssid)
{
    mode = MODE_API;
    char hname[19];
    Serial.print(F("Connected to "));
    Serial.print(ssid);
    Serial.print(F(" with IP "));
    Serial.println(WiFi.localIP());

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hname);
    // //
    WiFi.begin();
    WiFi.config(0u, 0u, 0u, 0u);
}

bool ConfigManager::commitChanges()
{
    EEPROM.put(0, magicBytes);
    return EEPROM.commit();
}

/**
 * @brief
 *
 */
void ConfigManager::readConfig()
{
    byte *ptr = (byte *)config;

    for (unsigned int i = 0; i < configSize; i++)
    {
        *(ptr++) = EEPROM.read(CONFIG_OFFSET + i);
        Serial.print(*ptr);
    }
}

/**
 * @brief
 *
 */
void ConfigManager::writeConfig()
{
    byte *ptr = (byte *)config;
    Serial.println(configSize);
    for (int i = 0; i < (int16_t)configSize; i++)
    {
        EEPROM.write(CONFIG_OFFSET + i, *(ptr++));
        // Serial.println(*ptr);
    }
    this->commitChanges();
}

/**
 * @brief
 *
 * @param str
 * @return boolean
 */
boolean ConfigManager::isIp(String str)
{
    for (unsigned int i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief
 *
 * @param ip
 * @return String
 */
String ConfigManager::toStringIP(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

#ifdef localbuild
// used to compile the project from the project
// and not as a library from another one
void setup() {}
void loop() {}
#endif