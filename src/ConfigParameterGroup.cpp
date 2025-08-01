#include "WiFiConfig.h"



void ConfigParameterGroup::toJson(JsonObject *json)
{
    JsonObject obj = json->createNestedObject(this->name);
    std::list<ConfigParameterInterface *>::iterator it;
    for (it = parameters.begin(); it != parameters.end(); ++it)
    {
        (*it)->toJson(&obj);
    }
}

void ConfigParameterGroup::toJsonSchema(JsonObject *json)
{
    (*json)["name"] = (const char*) name;
    if (this->metadata != NULL)
    {
        (*json)["label"] = this->metadata->label();

        if (this->metadata->description() != NULL)
        {
            (*json)["description"] = this->metadata->description();
        }
    }

    JsonArray params = json->createNestedArray("params");
    std::list<ConfigParameterInterface *>::iterator it;
    for (it = parameters.begin(); it != parameters.end(); ++it)
    {
        JsonObject obj = params.createNestedObject();
        (*it)->toJsonSchema(&obj);
    }
}

void ConfigParameterGroup::fromJson(JsonObject *json)
{
    if (json->containsKey(name) && (*json)[name].is<JsonObject>())
    {
        JsonVariant var = (*json)[name].as<JsonVariant>();
        JsonObject  group = var.as<JsonObject>();
        Serial.print("from json ing ... ");
        serializeJson(group,Serial);

        std::list<ConfigParameterInterface *>::iterator it;
        for (it = parameters.begin(); it != parameters.end(); it++)
        {
            (*it)->fromJson(&group);
        }
    }
}
