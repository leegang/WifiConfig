#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

void test_json_basic_operations() {
    // Test basic JSON operations with ArduinoJson 7.x
    DynamicJsonDocument doc(1024);
    
    // Test setting values using [] operator
    doc["name"] = "WiFiConfig";
    doc["version"] = "2.3.0";
    doc["enabled"] = true;
    doc["port"] = 8080;
    
    // Test getting values
    TEST_ASSERT_EQUAL_STRING("WiFiConfig", doc["name"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("2.3.0", doc["version"].as<const char*>());
    TEST_ASSERT_TRUE(doc["enabled"].as<bool>());
    TEST_ASSERT_EQUAL_INT(8080, doc["port"].as<int>());
}

void test_json_nested_objects() {
    DynamicJsonDocument doc(1024);
    
    // Test nested objects
    JsonObject config = doc.createNestedObject("config");
    config["ssid"] = "TestNetwork";
    config["password"] = "TestPassword";
    
    JsonArray networks = doc.createNestedArray("networks");
    JsonObject network1 = networks.createNestedObject();
    network1["name"] = "Network1";
    network1["signal"] = -45;
    
    // Test access
    TEST_ASSERT_EQUAL_STRING("TestNetwork", doc["config"]["ssid"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("Network1", doc["networks"][0]["name"].as<const char*>());
    TEST_ASSERT_EQUAL_INT(-45, doc["networks"][0]["signal"].as<int>());
}

void test_json_serialization() {
    DynamicJsonDocument doc(1024);
    doc["test"] = "serialization";
    doc["number"] = 42;
    
    String output;
    serializeJson(doc, output);
    
    // Should contain the test data
    TEST_ASSERT_TRUE(output.indexOf("serialization") > -1);
    TEST_ASSERT_TRUE(output.indexOf("42") > -1);
}

void setup() {
    delay(2000); // Wait for serial monitor
    
    UNITY_BEGIN();
    RUN_TEST(test_json_basic_operations);
    RUN_TEST(test_json_nested_objects);
    RUN_TEST(test_json_serialization);
    UNITY_END();
}

void loop() {
    // Empty loop
}