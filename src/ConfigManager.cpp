#include "ConfigManager.h"

const byte DNS_PORT = 53;
const char magicBytes[2] = {'C', 'M'};

const char mimeHTML[] PROGMEM = "text/html";
const char mimeJSON[] PROGMEM = "application/json";
const char mimePlain[] PROGMEM = "text/plain";

void ConfigParameterGroup::toJsonSchema(JsonObject *json)
{
    json->set("name", name);
    json->set("label", label);

    JsonArray &params = json->createNestedArray("params");
    std::list<BaseParameter *>::iterator it;
    for (it = parameters.begin(); it != parameters.end(); ++it)
    {
        JsonObject &obj = params.createNestedObject();
        (*it)->toJsonSchema(&obj);
    }
}

Mode ConfigManager::getMode()
{
    return this->mode;
}

void ConfigManager::setAPName(const char *name)
{
    this->apName = (char *)name;
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

void ConfigManager::loop()
{
    if (mode == ap && apTimeout > 0 && ((millis() - apStart) / 1000) > apTimeout)
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

void ConfigManager::save()
{
    this->writeConfig();
}

JsonObject &ConfigManager::decodeJson(String jsonString)
{
    DynamicJsonBuffer jsonBuffer;

    if (jsonString.length() == 0)
    {
        return jsonBuffer.createObject();
    }

    JsonObject &obj = jsonBuffer.parseObject(jsonString);

    if (!obj.success())
    {
        return jsonBuffer.createObject();
    }

    return obj;
}

void ConfigManager::handleGetSettings()
{
    // build json with settings values
}

/**
 * Return JSON with data schema
 */
void ConfigManager::handleGetSettingsSchema()
{
    DynamicJsonBuffer jsonBuffer;
    JsonArray &res = jsonBuffer.createArray();

    std::list<ConfigParameterGroup *>::iterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        JsonObject &obj = res.createNestedObject();
        (*it)->toJsonSchema(&obj);
    }

    String body;
    res.printTo(body);

    server->send(200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleGetScan()
{
}

void ConfigManager::handleAPGet()
{
    SPIFFS.begin();

    File f = SPIFFS.open(apFilename, "r");
    if (!f)
    {
        Serial.println(F("file open failed"));
        server->send(404, FPSTR(mimeHTML), F("File not found"));
        return;
    }

    server->streamFile(f, FPSTR(mimeHTML));

    f.close();
}

void ConfigManager::handleAPPost()
{
    bool isJson = server->header("Content-Type") == FPSTR(mimeJSON);
    String ssid;
    String password;
    char ssidChar[32];
    char passwordChar[64];

    if (isJson)
    {
        JsonObject &obj = this->decodeJson(server->arg("plain"));

        ssid = obj.get<String>("ssid");
        password = obj.get<String>("password");
    }
    else
    {
        ssid = server->arg("ssid");
        password = server->arg("password");
    }

    if (ssid.length() == 0)
    {
        server->send(400, FPSTR(mimePlain), F("Invalid ssid or password."));
        return;
    }

    strncpy(ssidChar, ssid.c_str(), sizeof(ssidChar));
    strncpy(passwordChar, password.c_str(), sizeof(passwordChar));

    EEPROM.put(0, magicBytes);
    EEPROM.put(WIFI_OFFSET, ssidChar);
    EEPROM.put(WIFI_OFFSET + 32, passwordChar);
    EEPROM.commit();

    server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

    ESP.restart();
}

void ConfigManager::handleRESTGet()
{
    // DynamicJsonBuffer jsonBuffer;
    // JsonObject& obj = jsonBuffer.createObject();

    // std::list<BaseParameter*>::iterator it;
    // for (it = parameters.begin(); it != parameters.end(); ++it) {
    //     if ((*it)->getMode() == set) {
    //         continue;
    //     }

    //     (*it)->toJson(&obj);
    // }

    // String body;
    // obj.printTo(body);

    // server->send(200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleRESTPut()
{
    // JsonObject& obj = this->decodeJson(server->arg("plain"));
    // if (!obj.success()) {
    //     server->send(400, FPSTR(mimeJSON), "");
    //     return;
    // }

    // std::list<BaseParameter*>::iterator it;
    // for (it = parameters.begin(); it != parameters.end(); ++it) {
    //     if ((*it)->getMode() == get) {
    //         continue;
    //     }

    //     (*it)->fromJson(&obj);
    // }

    // writeConfig();

    // server->send(204, FPSTR(mimeJSON), "");
}

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

void ConfigManager::setup()
{
    char magic[2];
    char ssid[32];
    char password[64];

    //
    Serial.println(F("Reading saved configuration"));

    EEPROM.get(0, magic);
    EEPROM.get(WIFI_OFFSET, ssid);
    EEPROM.get(WIFI_OFFSET + 32, password);

    readConfig();

    if (memcmp(magic, magicBytes, 2) == 0)
    {
        WiFi.begin(ssid, password[0] == '\0' ? NULL : password);

        if (wifiConnected())
        {
            Serial.print(F("Connected to "));
            Serial.print(ssid);
            Serial.print(F(" with "));
            Serial.println(WiFi.localIP());

            WiFi.mode(WIFI_STA);

            startApi();
        }
    }
    else
    {
        // We are at a cold start, don't bother timeing out.
        apTimeout = 0;

        startAP();
    }

    startWebServer();
}

void ConfigManager::startWebServer()
{
    const char *headerKeys[] = {"Content-Type"};
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char *);

    server.reset(new WebServer(80));

    server->collectHeaders(headerKeys, headerKeysSize);

    server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleAPGet, this));

    // wifi parameters
    server->on("/wifi/scan/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetScan, this));

    // configuration settings
    server->on("/settings/", HTTPMethod::HTTP_OPTIONS, std::bind(&ConfigManager::handleGetSettingsSchema, this));
    server->on("/settings/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleGetSettings, this));
    server->on("/settings/", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleAPPost, this));

    // not found handling
    server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));

    if (apCallback)
    {
        apCallback(server.get());
    }

    server->begin();
}

void ConfigManager::startAP()
{
    mode = ap;

    Serial.println(F("Starting Access Point"));

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName, apPassword);

    delay(500); // Need to wait to get IP

    IPAddress ip(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, NMask);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    dnsServer.reset(new DNSServer);
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, "*", ip);

    apStart = millis();
}

void ConfigManager::startApi()
{
    mode = api;
}

void ConfigManager::readConfig()
{
    byte *ptr = (byte *)config;

    for (unsigned int i = 0; i < configSize; i++)
    {
        *(ptr++) = EEPROM.read(CONFIG_OFFSET + i);
    }
}

void ConfigManager::writeConfig()
{
    byte *ptr = (byte *)config;

    for (unsigned int i = 0; i < configSize; i++)
    {
        EEPROM.write(CONFIG_OFFSET + i, *(ptr++));
    }
    EEPROM.commit();
}

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

String ConfigManager::toStringIP(IPAddress ip)
{
    String res = "";
    for (unsigned int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String((ip >> (8 * 3)) & 0xFF);
    return res;
}
