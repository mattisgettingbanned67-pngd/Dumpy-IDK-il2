#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct Il2CppGlobalMetadataHeader {
    uint32_t sanity;
    uint32_t version;

    uint32_t stringLiteralOffset;
    uint32_t stringLiteralCount;
    uint32_t stringLiteralDataOffset;
    uint32_t stringLiteralDataCount;
    uint32_t stringOffset;
    uint32_t stringCount;
    uint32_t eventsOffset;
    uint32_t eventsCount;
    uint32_t propertiesOffset;
    uint32_t propertiesCount;
    uint32_t methodsOffset;
    uint32_t methodsCount;
    uint32_t parameterDefaultValuesOffset;
    uint32_t parameterDefaultValuesCount;
    uint32_t fieldDefaultValuesOffset;
    uint32_t fieldDefaultValuesCount;
    uint32_t fieldAndParameterDefaultValueDataOffset;
    uint32_t fieldAndParameterDefaultValueDataCount;
    uint32_t fieldMarshaledSizesOffset;
    uint32_t fieldMarshaledSizesCount;
    uint32_t parametersOffset;
    uint32_t parametersCount;
    uint32_t fieldsOffset;
    uint32_t fieldsCount;
    uint32_t genericParametersOffset;
    uint32_t genericParametersCount;
    uint32_t genericParameterConstraintsOffset;
    uint32_t genericParameterConstraintsCount;
    uint32_t genericContainersOffset;
    uint32_t genericContainersCount;
    uint32_t nestedTypesOffset;
    uint32_t nestedTypesCount;
    uint32_t interfacesOffset;
    uint32_t interfacesCount;
    uint32_t vtableMethodsOffset;
    uint32_t vtableMethodsCount;
    uint32_t interfaceOffsetsOffset;
    uint32_t interfaceOffsetsCount;
    uint32_t typeDefinitionsOffset;
    uint32_t typeDefinitionsCount;
    uint32_t rgctxEntriesOffset;
    uint32_t rgctxEntriesCount;
    uint32_t imagesOffset;
    uint32_t imagesCount;
    uint32_t assembliesOffset;
    uint32_t assembliesCount;
    uint32_t metadataUsageListsOffset;
    uint32_t metadataUsageListsCount;
    uint32_t metadataUsagePairsOffset;
    uint32_t metadataUsagePairsCount;
    uint32_t fieldRefsOffset;
    uint32_t fieldRefsCount;
    uint32_t referencedAssembliesOffset;
    uint32_t referencedAssembliesCount;
    uint32_t attributesInfoOffset;
    uint32_t attributesInfoCount;
    uint32_t attributeTypesOffset;
    uint32_t attributeTypesCount;
    uint32_t unresolvedVirtualCallParameterTypesOffset;
    uint32_t unresolvedVirtualCallParameterTypesCount;
    uint32_t unresolvedVirtualCallParameterRangesOffset;
    uint32_t unresolvedVirtualCallParameterRangesCount;
    uint32_t windowsRuntimeTypeNamesOffset;
    uint32_t windowsRuntimeTypeNamesSize;
    uint32_t windowsRuntimeStringsOffset;
    uint32_t windowsRuntimeStringsSize;
    uint32_t exportedTypeDefinitionsOffset;
    uint32_t exportedTypeDefinitionsCount;
};

struct Il2CppImageDefinition {
    int32_t nameIndex;
    int32_t assemblyIndex;
    int32_t typeStart;
    uint32_t typeCount;
    int32_t exportedTypeStart;
    uint32_t exportedTypeCount;
    int32_t entryPointIndex;
    uint32_t token;
    int32_t customAttributeStart;
    uint32_t customAttributeCount;
};

struct Il2CppTypeDefinition {
    int32_t nameIndex;
    int32_t namespaceIndex;
    int32_t byvalTypeIndex;
    int32_t byrefTypeIndex;
    int32_t declaringTypeIndex;
    int32_t parentIndex;
    int32_t elementTypeIndex;
    int32_t rgctxStartIndex;
    int32_t rgctxCount;
    int32_t genericContainerIndex;
    uint32_t flags;
    int32_t fieldStart;
    int32_t methodStart;
    int32_t eventStart;
    int32_t propertyStart;
    int32_t nestedTypesStart;
    int32_t interfacesStart;
    int32_t vtableStart;
    int32_t interfaceOffsetsStart;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint32_t bitfield;
    uint32_t token;
};

struct Il2CppMethodDefinition {
    int32_t nameIndex;
    int32_t declaringType;
    int32_t returnType;
    int32_t parameterStart;
    int32_t customAttributeIndex;
    int32_t genericContainerIndex;
    int32_t methodIndex;
    int32_t invokerIndex;
    int32_t reversePInvokeWrapperIndex;
    int32_t rgctxStartIndex;
    int32_t rgctxCount;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t parameterCount;
};
#pragma pack(pop)

struct Section {
    uint32_t virtualAddress;
    uint32_t virtualSize;
    uint32_t rawPtr;
    uint32_t rawSize;
    std::string name;
};

struct PEInfo {
    uint64_t imageBase = 0;
    std::vector<Section> sections;
};

static std::string trimQuotes(std::string s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

static std::string askPath(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return trimQuotes(input);
}

static bool loadFile(const fs::path& p, std::vector<uint8_t>& out) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    auto size = static_cast<size_t>(f.tellg());
    f.seekg(0, std::ios::beg);
    out.resize(size);
    f.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(size));
    return f.good() || f.eof();
}

static std::string readCString(const std::vector<uint8_t>& blob, uint32_t baseOffset, int32_t index) {
    if (index < 0) return "";
    const size_t start = static_cast<size_t>(baseOffset) + static_cast<size_t>(index);
    if (start >= blob.size()) return "";
    std::string out;
    for (size_t i = start; i < blob.size(); ++i) {
        char c = static_cast<char>(blob[i]);
        if (c == '\0') break;
        out.push_back(c);
    }
    return out;
}

static std::optional<PEInfo> parsePE(const std::vector<uint8_t>& bin) {
    if (bin.size() < 0x100) return std::nullopt;
    if (bin[0] != 'M' || bin[1] != 'Z') return std::nullopt;
    uint32_t peOffset = *reinterpret_cast<const uint32_t*>(&bin[0x3C]);
    if (peOffset + 0x18 >= bin.size()) return std::nullopt;
    if (bin[peOffset] != 'P' || bin[peOffset + 1] != 'E') return std::nullopt;

    const uint16_t numberOfSections = *reinterpret_cast<const uint16_t*>(&bin[peOffset + 6]);
    const uint16_t optHeaderSize = *reinterpret_cast<const uint16_t*>(&bin[peOffset + 20]);
    const size_t optionalHeader = peOffset + 24;
    if (optionalHeader + optHeaderSize > bin.size()) return std::nullopt;

    uint16_t magic = *reinterpret_cast<const uint16_t*>(&bin[optionalHeader]);
    PEInfo info;
    if (magic == 0x20B) {
        info.imageBase = *reinterpret_cast<const uint64_t*>(&bin[optionalHeader + 24]);
    } else if (magic == 0x10B) {
        info.imageBase = *reinterpret_cast<const uint32_t*>(&bin[optionalHeader + 28]);
    } else {
        return std::nullopt;
    }

    const size_t sectionTable = optionalHeader + optHeaderSize;
    const size_t sectionSize = 40;
    if (sectionTable + static_cast<size_t>(numberOfSections) * sectionSize > bin.size()) return std::nullopt;

    for (uint16_t i = 0; i < numberOfSections; ++i) {
        size_t s = sectionTable + i * sectionSize;
        Section sec{};
        sec.name.assign(reinterpret_cast<const char*>(&bin[s]), reinterpret_cast<const char*>(&bin[s]) + 8);
        sec.name.erase(std::find(sec.name.begin(), sec.name.end(), '\0'), sec.name.end());
        sec.virtualSize = *reinterpret_cast<const uint32_t*>(&bin[s + 8]);
        sec.virtualAddress = *reinterpret_cast<const uint32_t*>(&bin[s + 12]);
        sec.rawSize = *reinterpret_cast<const uint32_t*>(&bin[s + 16]);
        sec.rawPtr = *reinterpret_cast<const uint32_t*>(&bin[s + 20]);
        info.sections.push_back(sec);
    }
    return info;
}

static bool isPointerToText(const PEInfo& pe, uint64_t ptr, uint32_t textStart, uint32_t textEnd) {
    if (ptr < pe.imageBase) return false;
    uint64_t rva64 = ptr - pe.imageBase;
    if (rva64 > 0xFFFFFFFFull) return false;
    uint32_t rva = static_cast<uint32_t>(rva64);
    return rva >= textStart && rva < textEnd;
}

struct CandidateTable {
    uint32_t fileOffset = 0;
    size_t count = 0;
};

static std::optional<CandidateTable> findMethodPointerTable(const std::vector<uint8_t>& gameAsm, const PEInfo& pe, size_t minCount) {
    Section rdata{}, text{};
    bool hasRdata = false, hasText = false;
    for (const auto& s : pe.sections) {
        if (s.name == ".rdata") {
            rdata = s;
            hasRdata = true;
        } else if (s.name == ".text") {
            text = s;
            hasText = true;
        }
    }
    if (!hasRdata || !hasText) return std::nullopt;

    const uint32_t textStart = text.virtualAddress;
    const uint32_t textEnd = text.virtualAddress + std::max(text.virtualSize, text.rawSize);

    const bool pe64 = true;
    const size_t ptrSize = pe64 ? 8 : 4;
    if (rdata.rawPtr + rdata.rawSize > gameAsm.size()) return std::nullopt;

    CandidateTable best;
    size_t run = 0;
    uint32_t runStart = 0;

    for (uint32_t off = rdata.rawPtr; off + ptrSize <= rdata.rawPtr + rdata.rawSize; off += static_cast<uint32_t>(ptrSize)) {
        uint64_t ptr = 0;
        if (ptrSize == 8) {
            ptr = *reinterpret_cast<const uint64_t*>(&gameAsm[off]);
        } else {
            ptr = *reinterpret_cast<const uint32_t*>(&gameAsm[off]);
        }

        bool ok = isPointerToText(pe, ptr, textStart, textEnd);
        if (ok) {
            if (run == 0) runStart = off;
            run++;
        } else {
            if (run > best.count) {
                best.fileOffset = runStart;
                best.count = run;
            }
            run = 0;
        }
    }
    if (run > best.count) {
        best.fileOffset = runStart;
        best.count = run;
    }

    if (best.count < minCount) return std::nullopt;
    return best;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << " IL2CPP OFFSET DUMPER (CMake/EXE)\n";
    std::cout << " Drag-and-drop global-metadata.dat and GameAssembly.dll\n";
    std::cout << "========================================\n\n";

    fs::path metadataPath;
    fs::path gameAssemblyPath;

    if (argc >= 3) {
        metadataPath = trimQuotes(argv[1]);
        gameAssemblyPath = trimQuotes(argv[2]);
    } else {
        metadataPath = askPath("[1/2] Drop/Paste path to global-metadata.dat, then press Enter:\n> ");
        gameAssemblyPath = askPath("[2/2] Drop/Paste path to GameAssembly.dll, then press Enter:\n> ");
    }

    if (!fs::exists(metadataPath) || !fs::exists(gameAssemblyPath)) {
        std::cerr << "[!] One or both files do not exist.\n";
        return 1;
    }

    std::vector<uint8_t> metadata;
    std::vector<uint8_t> gameAsm;
    if (!loadFile(metadataPath, metadata)) {
        std::cerr << "[!] Failed to load metadata file.\n";
        return 1;
    }
    if (!loadFile(gameAssemblyPath, gameAsm)) {
        std::cerr << "[!] Failed to load GameAssembly.dll.\n";
        return 1;
    }

    if (metadata.size() < sizeof(Il2CppGlobalMetadataHeader)) {
        std::cerr << "[!] Metadata file too small.\n";
        return 1;
    }

    auto* header = reinterpret_cast<const Il2CppGlobalMetadataHeader*>(metadata.data());
    if (header->sanity != 0xFAB11BAF) {
        std::cerr << "[!] Invalid metadata sanity: 0x" << std::hex << header->sanity << "\n";
        return 1;
    }

    const uint32_t typeDefCount = header->typeDefinitionsCount / sizeof(Il2CppTypeDefinition);
    const uint32_t methodCount = header->methodsCount / sizeof(Il2CppMethodDefinition);
    const uint32_t imageCount = header->imagesCount / sizeof(Il2CppImageDefinition);

    if (header->typeDefinitionsOffset + header->typeDefinitionsCount > metadata.size() ||
        header->methodsOffset + header->methodsCount > metadata.size() ||
        header->imagesOffset + header->imagesCount > metadata.size()) {
        std::cerr << "[!] Metadata offsets are out of range.\n";
        return 1;
    }

    const auto* images = reinterpret_cast<const Il2CppImageDefinition*>(metadata.data() + header->imagesOffset);
    const auto* typeDefs = reinterpret_cast<const Il2CppTypeDefinition*>(metadata.data() + header->typeDefinitionsOffset);
    const auto* methods = reinterpret_cast<const Il2CppMethodDefinition*>(metadata.data() + header->methodsOffset);

    auto pe = parsePE(gameAsm);
    if (!pe) {
        std::cerr << "[!] Failed to parse GameAssembly PE.\n";
        return 1;
    }

    auto table = findMethodPointerTable(gameAsm, *pe, std::min<size_t>(methodCount, 2000));
    std::vector<uint64_t> methodPtrs;
    if (table) {
        const size_t ptrSize = 8;
        size_t capped = std::min(table->count, static_cast<size_t>(methodCount));
        methodPtrs.reserve(capped);
        for (size_t i = 0; i < capped; ++i) {
            size_t off = table->fileOffset + i * ptrSize;
            if (off + ptrSize > gameAsm.size()) break;
            methodPtrs.push_back(*reinterpret_cast<const uint64_t*>(&gameAsm[off]));
        }
    }

    fs::path outPath = fs::current_path() / "dump.cs";
    std::ofstream out(outPath);
    if (!out) {
        std::cerr << "[!] Failed to create output file.\n";
        return 1;
    }

    out << "// IL2CPP Offset Dump\n";
    out << "// Metadata Version: " << header->version << "\n";
    out << "// Types: " << typeDefCount << " | Methods: " << methodCount << "\n";
    out << "// RVA resolution: " << (table ? "heuristic method pointer table found" : "not found (names/tokens only)") << "\n\n";

    std::unordered_map<int32_t, std::string> imageNameByTypeStart;
    for (uint32_t i = 0; i < imageCount; ++i) {
        imageNameByTypeStart[images[i].typeStart] = readCString(metadata, header->stringOffset, images[i].nameIndex);
    }

    for (uint32_t t = 0; t < typeDefCount; ++t) {
        const auto& td = typeDefs[t];
        std::string ns = readCString(metadata, header->stringOffset, td.namespaceIndex);
        std::string name = readCString(metadata, header->stringOffset, td.nameIndex);

        std::string img;
        auto it = imageNameByTypeStart.find(static_cast<int32_t>(t));
        if (it != imageNameByTypeStart.end()) img = it->second;

        out << "// Image: " << img << "\n";
        out << "// Namespace: " << ns << "\n";
        out << "// TypeDefIndex: " << t << "\n";
        out << "class " << (name.empty() ? "<unnamed_type>" : name) << " // Token: 0x" << std::hex << td.token << std::dec << "\n";
        out << "{\n";

        for (uint16_t mi = 0; mi < td.method_count; ++mi) {
            const int32_t mIndex = td.methodStart + mi;
            if (mIndex < 0 || static_cast<uint32_t>(mIndex) >= methodCount) continue;
            const auto& md = methods[mIndex];
            std::string mname = readCString(metadata, header->stringOffset, md.nameIndex);

            out << "    // MethodDefIndex: " << mIndex;
            out << " | Token: 0x" << std::hex << md.token << std::dec;
            out << " | MethodIndex: " << md.methodIndex;

            if (md.methodIndex >= 0 && static_cast<size_t>(md.methodIndex) < methodPtrs.size()) {
                uint64_t va = methodPtrs[md.methodIndex];
                uint64_t rva = (va >= pe->imageBase) ? (va - pe->imageBase) : 0;
                out << " | RVA: 0x" << std::hex << rva << std::dec;
            } else {
                out << " | RVA: N/A";
            }
            out << "\n";
            out << "    void " << (mname.empty() ? "<unnamed_method>" : mname) << "();\n\n";
        }

        out << "}\n\n";
    }

    out.close();

    std::cout << "[+] Dump complete.\n";
    std::cout << "[+] Output: " << outPath.string() << "\n";
    std::cout << "[+] Metadata version: " << header->version << "\n";
    std::cout << "[+] Type definitions: " << typeDefCount << "\n";
    std::cout << "[+] Methods: " << methodCount << "\n";
    if (table) {
        std::cout << "[+] Method pointer table candidate found (" << table->count << " entries).\n";
    } else {
        std::cout << "[-] Method pointer table not found; RVAs may be N/A.\n";
    }

    return 0;
}
