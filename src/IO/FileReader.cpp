#include "FileReader.h"


FileReader::FileReader(const std::string& fileName)  {
    if (std::filesystem::exists(fileName)) {
        file.open(fileName, std::ios::binary);
    }
}

FileReader::~FileReader() {
    if (file.is_open()) {
        file.close();
    }
}

bool FileReader::IsOpen() { return file.is_open(); }

uint32_t FileReader::ReadUInt32() {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    return value;
}

uint8_t FileReader::ReadUInt8() {
    uint8_t value;
    file.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
    return value;
}

float FileReader::ReadFloat() {
    float value;
    file.read(reinterpret_cast<char*>(&value), sizeof(float));
    return value;
}

std::string FileReader::ReadString() {
    uint32_t length;
    file.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
    std::string result(length, '\0');
    file.read(&result[0], length);
    return result;
}

RE::FormID FileReader::ReadFormId() { 
    char fileRef = ReadUInt8();

    if (fileRef == 0) {
        logger::trace("nullref");
        return 0;
    }

    if (fileRef == 1) {
        auto fileName = ReadString();
        uint32_t localId = ReadUInt32();
        logger::trace("fileName: {}", fileName);
        logger::trace("fileName: {:x}", localId);

        return RE::TESDataHandler::GetSingleton()->LookupFormID(localId, fileName);
    } else if (fileRef == 2) {
        uint32_t id = ReadUInt32();
        logger::trace("dynamic form: {:x}",id);
        return id;
    }

    return 0;
}
