# C++ 依赖包升级总结

## 主要更新

### ArduinoJson 库升级
- **从版本**: ^6.0.0
- **升级到**: ^7.4.2 (最新版本)
- **升级原因**: 
  - 性能改进和内存优化
  - 更好的类型安全
  - 修复已知安全漏洞
  - 更简洁的API

### 项目版本更新
- **从版本**: 2.2.0
- **升级到**: 2.3.0

## API 兼容性更新

### 已弃用的API替换
ArduinoJson 7.x中以下API已被弃用，已全部更新为新的语法：

1. **getMember() 方法**
   ```cpp
   // 旧语法 (6.x)
   json->getMember(name).as<T>()
   
   // 新语法 (7.x)
   (*json)[name].as<T>()
   ```

2. **getOrAddMember() 方法**
   ```cpp
   // 旧语法 (6.x)
   json->getOrAddMember(name).set(value)
   
   // 新语法 (7.x)
   (*json)[name] = value
   ```

### 更新的文件
- `src/ConfigParameterGroup.cpp`
- `src/WiFiConfig.h` (模板类中的JSON操作)

### 保持兼容的API
以下API在7.x中保持兼容，无需修改：
- `DynamicJsonDocument`
- `serializeJson()` / `deserializeJson()`
- `containsKey()`
- `createNestedObject()` / `createNestedArray()`
- `as<T>()` 类型转换

## 新增配置文件

### PlatformIO 配置
创建了 `platformio.ini` 文件，支持：
- ESP32 开发板 (esp32dev)
- ESP8266 开发板 (nodemcuv2)
- 自动依赖管理

### 测试文件
创建了 `test/test_json_compatibility.cpp` 用于验证ArduinoJson 7.x兼容性

## 升级优势

1. **性能提升**: ArduinoJson 7.x在解析和序列化方面有显著性能改进
2. **内存优化**: 更高效的内存使用
3. **类型安全**: 更好的编译时类型检查
4. **API简化**: 使用 `[]` 操作符更直观
5. **安全性**: 修复了6.x版本中的已知安全问题

## 验证方法

使用PlatformIO进行编译测试：
```bash
# 编译ESP32版本
pio run -e esp32dev

# 编译ESP8266版本  
pio run -e esp8266

# 运行单元测试
pio test
```

## 注意事项

1. 确保使用的ESP32/ESP8266开发环境支持ArduinoJson 7.x
2. 如果有自定义的JSON处理代码，需要检查是否使用了已弃用的API
3. 建议在实际硬件上测试以确保完全兼容

## 后续建议

1. 定期检查ArduinoJson的更新版本
2. 考虑升级其他相关依赖库（如M5Stack库）
3. 添加更多单元测试覆盖JSON处理功能