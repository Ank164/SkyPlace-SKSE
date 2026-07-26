#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

class FileWriter {
private:
    std::ofstream file;

public:
    FileWriter(std::string& fileName);
    ~FileWriter();
    void WriteUInt32(uint32_t value);
    void WriteUInt8(uint8_t value);
    void WriteFloat(float value);
    bool IsOpen();
    void WriteString(const std::string& value);
    void WriteFormId(RE::FormID id);
};
