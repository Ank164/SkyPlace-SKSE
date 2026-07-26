#include "Persistence.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "ObjectGroup.h"
#include "HUD.h"

namespace {
    constexpr std::uint32_t persistenceMagic = 0x32504C53;
    constexpr std::uint32_t persistenceVersion = 7;

    void WritePoint(FileWriter& writer, const RE::NiPoint3& point) {
        writer.WriteFloat(point.x);
        writer.WriteFloat(point.y);
        writer.WriteFloat(point.z);
    }

    RE::NiPoint3 ReadPoint(FileReader& reader) {
        return {
            reader.ReadFloat(),
            reader.ReadFloat(),
            reader.ReadFloat()
        };
    }

    void WriteMatrix(FileWriter& writer, const RE::NiMatrix3& matrix) {
        for (std::uint32_t row = 0; row < 3; ++row) {
            for (std::uint32_t column = 0; column < 3; ++column) {
                writer.WriteFloat(matrix.entry[row][column]);
            }
        }
    }

    RE::NiMatrix3 ReadMatrix(FileReader& reader) {
        RE::NiMatrix3 matrix;
        for (std::uint32_t row = 0; row < 3; ++row) {
            for (std::uint32_t column = 0; column < 3; ++column) {
                matrix.entry[row][column] = reader.ReadFloat();
            }
        }
        return matrix;
    }
}

std::string removeEssSuffix(const std::string& input) {
    if (input.size() >= 4 && input.compare(input.size() - 4, 4, ".ess") == 0) {
        return input.substr(0, input.size() - 4);
    }
    return input;
}
void Persistence::Load(std::string fileName) {
    fileName = removeEssSuffix(fileName) + "_Place.bin";
    logger::trace("loading: {}", fileName);

    ObjectGroup::Clear();
    HUD::SetIsEnabled(true);
    FileReader reader(fileName);

    if (reader.IsOpen()) {
        
        const std::uint32_t firstValue = reader.ReadUInt32();
        const bool versioned = firstValue == persistenceMagic;
        const std::uint32_t version = versioned ? reader.ReadUInt32() : 1;
        const std::uint32_t size = versioned ? reader.ReadUInt32() : firstValue;
        for (auto i = 0; i < size; ++i) {
            reader.ReadFormId();
            reader.ReadFormId();
        }

        if (version >= 7) {
            const std::uint32_t inventoryChestCount = reader.ReadUInt32();
            for (std::uint32_t chestIndex = 0; chestIndex < inventoryChestCount; ++chestIndex) {
                reader.ReadFormId();
                reader.ReadFormId();
            }
        }

        if (version >= 2) {
            const std::uint32_t groupCount = reader.ReadUInt32();
            for (std::uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
                ObjectGroup::Data group;
                group.itemFormID = reader.ReadFormId();
                group.guid = reader.ReadString();
                group.meshPath = reader.ReadString();
                group.name = reader.ReadString();
                group.weight = reader.ReadFloat();
                group.value = static_cast<std::int32_t>(reader.ReadUInt32());
                if (version >= 4) {
                    group.playerFacingYaw = reader.ReadFloat();
                    group.preservesPlayerFacing = reader.ReadUInt32() != 0;
                }

                const std::uint32_t memberCount = reader.ReadUInt32();
                group.members.reserve(memberCount);
                for (std::uint32_t memberIndex = 0; memberIndex < memberCount; ++memberIndex) {
                    ObjectGroup::Member member;
                    member.objectFormID = reader.ReadFormId();
                    member.relativePosition = ReadPoint(reader);
                    member.relativeAngle = ReadPoint(reader);
                    member.scale = reader.ReadFloat();
                    if (version >= 3) {
                        member.previewPosition = ReadPoint(reader);
                        if (version >= 6) {
                            member.previewRotation = ReadMatrix(reader);
                        } else {
                            const RE::NiPoint3 previewAngle = ReadPoint(reader);
                            member.previewRotation.SetEulerAnglesXYZ(previewAngle);
                        }
                        member.previewScale = reader.ReadFloat();
                    } else {
                        member.previewPosition = member.relativePosition;
                        const RE::NiPoint3 previewAngle = {
                            -member.relativeAngle.x,
                            -member.relativeAngle.y,
                            -member.relativeAngle.z
                        };
                        member.previewRotation.SetEulerAnglesXYZ(previewAngle);
                        member.previewScale = member.scale;
                    }
                    if (version >= 7) {
                        member.inventoryChestRefID = reader.ReadFormId();
                    }
                    group.members.push_back(member);
                }

                ObjectGroup::Restore(std::move(group));
            }
        }

        if (version >= 5) {
            HUD::SetIsEnabled(reader.ReadUInt8() != 0);
        }
        logger::trace("File exist");
    } else {
        logger::trace("File do not exists");
    }
}
void Persistence::Save(std::string fileName) {
    fileName = removeEssSuffix(fileName) + "_Place.bin";
    logger::trace("saving: {}", fileName);

    FileWriter writer(fileName);
    if (writer.IsOpen()) {
        writer.WriteUInt32(persistenceMagic);
        writer.WriteUInt32(persistenceVersion);
        writer.WriteUInt32(0);
        writer.WriteUInt32(0);

        const auto& groups = ObjectGroup::GetAll();
        writer.WriteUInt32(static_cast<std::uint32_t>(groups.size()));
        for (const auto& [itemFormID, group] : groups) {
            writer.WriteFormId(itemFormID);
            writer.WriteString(group.guid);
            writer.WriteString(group.meshPath);
            writer.WriteString(group.name);
            writer.WriteFloat(group.weight);
            writer.WriteUInt32(static_cast<std::uint32_t>(group.value));
            writer.WriteFloat(group.playerFacingYaw);
            writer.WriteUInt32(group.preservesPlayerFacing ? 1 : 0);
            writer.WriteUInt32(static_cast<std::uint32_t>(group.members.size()));

            for (const ObjectGroup::Member& member : group.members) {
                writer.WriteFormId(member.objectFormID);
                WritePoint(writer, member.relativePosition);
                WritePoint(writer, member.relativeAngle);
                writer.WriteFloat(member.scale);
                WritePoint(writer, member.previewPosition);
                WriteMatrix(writer, member.previewRotation);
                writer.WriteFloat(member.previewScale);
                writer.WriteFormId(member.inventoryChestRefID);
            }
        }

        writer.WriteUInt8(HUD::GetIsEnabled() ? 1 : 0);
    } else {
        logger::error("failed to open file");
    }
}
