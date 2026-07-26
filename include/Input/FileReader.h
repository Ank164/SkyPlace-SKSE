
class FileReader {
private:
    std::ifstream file;
public:
    FileReader(const std::string& fileName);

    ~FileReader();

    bool IsOpen();

    uint32_t ReadUInt32();
    uint8_t ReadUInt8();
    float ReadFloat();
    std::string ReadString();
    RE::FormID ReadFormId();
};
