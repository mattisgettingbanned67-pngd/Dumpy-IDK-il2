#include <algorithm>
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

struct Il2CppFieldDefinition {
    int32_t nameIndex;
    int32_t typeIndex;
    int32_t customAttributeIndex;
    uint32_t token;
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
    bool is64Bit = false;
    std::vector<Section> sections;
};

struct Args {
    fs::path metadataPath;
    fs::path gameAssemblyPath;
    fs::path outputPath = "dump.cs";
};

struct CandidateTable {
    uint32_t fileOffset = 0;
    size_t count = 0;
    uint32_t sectionVA = 0;
};

static std::string trimQuotes(std::string s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return trimQuotes(input);
}

static void printUsage(const char* exe) {
    std::cout << "Usage:\n";
    std::cout << "  " << exe << " <global-metadata.dat> <GameAssembly.dll> [--out <file>]\n";
    std::cout << "  " << exe << "  (interactive drag-and-drop prompts)\n\n";
}

static std::optional<Args> parseArgs(int argc, char* argv[]) {
    Args args;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string current = trimQuotes(argv[i]);
        if (current == "--help" || current == "-h") {
            printUsage(argv[0]);
            return std::nullopt;
        }
        if (current == "--out") {
            if (i + 1 >= argc) {
                std::cerr << "[!] Missing value for --out\n";
                return std::nullopt;
            }
            args.outputPath = trimQuotes(argv[++i]);
            continue;
        }
        positional.push_back(current);
    }

    if (positional.size() >= 2) {
        args.metadataPath = positional[0];
        args.gameAssemblyPath = positional[1];
    } else {
        args.metadataPath = readLine("[1/2] Drop/Paste path to global-metadata.dat, then Enter:\n> ");
        args.gameAssemblyPath = readLine("[2/2] Drop/Paste path to GameAssembly.dll, then Enter:\n> ");
    }

    return args;
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

static std::string readCString(const std::vector<uint8_t>& blob, uint32_t stringBaseOffset, int32_t idx) {
    if (idx < 0) return "";
    const size_t start = static_cast<size_t>(stringBaseOffset) + static_cast<size_t>(idx);
    if (start >= blob.size()) return "";

    std::string out;
    for (size_t i = start; i < blob.size(); ++i) {
        char c = static_cast<char>(blob[i]);
        if (c == '\0') break;
        out.push_back(c);
    }
    return out;
}

static bool rangeInBounds(size_t off, size_t len, size_t total) {
    return off <= total && len <= total - off;
}

static std::optional<PEInfo> parsePE(const std::vector<uint8_t>& bin) {
    if (bin.size() < 0x100 || bin[0] != 'M' || bin[1] != 'Z') return std::nullopt;

    uint32_t peOffset = *reinterpret_cast<const uint32_t*>(&bin[0x3C]);
    if (!rangeInBounds(peOffset, 24, bin.size())) return std::nullopt;
    if (bin[peOffset] != 'P' || bin[peOffset + 1] != 'E') return std::nullopt;

    uint16_t numberOfSections = *reinterpret_cast<const uint16_t*>(&bin[peOffset + 6]);
    uint16_t optHeaderSize = *reinterpret_cast<const uint16_t*>(&bin[peOffset + 20]);
    size_t optHeader = peOffset + 24;
    if (!rangeInBounds(optHeader, optHeaderSize, bin.size())) return std::nullopt;

    PEInfo info;
    uint16_t magic = *reinterpret_cast<const uint16_t*>(&bin[optHeader]);
    if (magic == 0x20B) {
        info.is64Bit = true;
        if (!rangeInBounds(optHeader + 24, sizeof(uint64_t), bin.size())) return std::nullopt;
        info.imageBase = *reinterpret_cast<const uint64_t*>(&bin[optHeader + 24]);
    } else if (magic == 0x10B) {
        info.is64Bit = false;
        if (!rangeInBounds(optHeader + 28, sizeof(uint32_t), bin.size())) return std::nullopt;
        info.imageBase = *reinterpret_cast<const uint32_t*>(&bin[optHeader + 28]);
    } else {
        return std::nullopt;
    }

    size_t sectionTable = optHeader + optHeaderSize;
    size_t sectionSize = 40;
    if (!rangeInBounds(sectionTable, static_cast<size_t>(numberOfSections) * sectionSize, bin.size())) return std::nullopt;

    info.sections.reserve(numberOfSections);
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

static bool ptrToRva(const PEInfo& pe, uint64_t ptr, uint32_t& outRva) {
    if (ptr < pe.imageBase) return false;
    const uint64_t rva64 = ptr - pe.imageBase;
    if (rva64 > 0xFFFFFFFFull) return false;
    outRva = static_cast<uint32_t>(rva64);
    return true;
}

static std::optional<Section> findSection(const PEInfo& pe, const std::string& name) {
    for (const auto& s : pe.sections) {
        if (s.name == name) return s;
    }
    return std::nullopt;
}

static std::optional<CandidateTable> scanSectionForMethodPtrTable(
    const std::vector<uint8_t>& gameAsm,
    const PEInfo& pe,
    const Section& scan,
    uint32_t textStart,
    uint32_t textEnd,
    size_t ptrSize,
    size_t methodCount) {

    if (!rangeInBounds(scan.rawPtr, scan.rawSize, gameAsm.size()) || ptrSize == 0) return std::nullopt;

    CandidateTable best;
    for (uint32_t off = scan.rawPtr; off + ptrSize <= scan.rawPtr + scan.rawSize; off += static_cast<uint32_t>(ptrSize)) {
        uint64_t ptr = (ptrSize == 8)
            ? *reinterpret_cast<const uint64_t*>(&gameAsm[off])
            : *reinterpret_cast<const uint32_t*>(&gameAsm[off]);

        uint32_t rva = 0;
        if (!ptrToRva(pe, ptr, rva) || rva < textStart || rva >= textEnd) continue;

        uint32_t runStart = off;
        size_t runCount = 0;
        uint32_t probe = off;
        while (probe + ptrSize <= scan.rawPtr + scan.rawSize) {
            uint64_t v = (ptrSize == 8)
                ? *reinterpret_cast<const uint64_t*>(&gameAsm[probe])
                : *reinterpret_cast<const uint32_t*>(&gameAsm[probe]);
            uint32_t vrva = 0;
            if (!ptrToRva(pe, v, vrva) || vrva < textStart || vrva >= textEnd) break;
            ++runCount;
            probe += static_cast<uint32_t>(ptrSize);
        }

        if (runCount > best.count) {
            best.fileOffset = runStart;
            best.count = runCount;
            best.sectionVA = scan.virtualAddress;
        }

        if (runCount > methodCount && methodCount > 0) {
            break;
        }

        off = probe;
    }

    if (best.count == 0) return std::nullopt;
    return best;
}

static std::optional<CandidateTable> findBestMethodPointerTable(
    const std::vector<uint8_t>& gameAsm,
    const PEInfo& pe,
    size_t methodCount) {

    auto textOpt = findSection(pe, ".text");
    if (!textOpt) return std::nullopt;
    const Section text = *textOpt;
    const uint32_t textStart = text.virtualAddress;
    const uint32_t textEnd = text.virtualAddress + std::max(text.virtualSize, text.rawSize);

    const size_t ptrSize = pe.is64Bit ? 8 : 4;
    std::vector<Section> scanSections;
    if (auto rdata = findSection(pe, ".rdata")) scanSections.push_back(*rdata);
    if (auto data = findSection(pe, ".data")) scanSections.push_back(*data);

    CandidateTable best;
    bool found = false;
    for (const auto& s : scanSections) {
        auto cand = scanSectionForMethodPtrTable(gameAsm, pe, s, textStart, textEnd, ptrSize, methodCount);
        if (!cand) continue;

        const size_t c = cand->count;
        const size_t b = best.count;
        const size_t cDist = (c > methodCount) ? (c - methodCount) : (methodCount - c);
        const size_t bDist = (b > methodCount) ? (b - methodCount) : (methodCount - b);

        if (!found || cDist < bDist || (cDist == bDist && c > b)) {
            best = *cand;
            found = true;
        }
    }

    if (!found || best.count < std::min<size_t>(methodCount, 512)) return std::nullopt;
    return best;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string a1 = trimQuotes(argv[1]);
        if (a1 == "--help" || a1 == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    std::cout << "========================================\n";
    std::cout << " IL2CPP OFFSET DUMPER (Improved)\n";
    std::cout << " Drag-and-drop metadata + GameAssembly\n";
    std::cout << "========================================\n\n";

    auto argsOpt = parseArgs(argc, argv);
    if (!argsOpt) return 1;
    const Args args = *argsOpt;

    if (!fs::exists(args.metadataPath) || !fs::exists(args.gameAssemblyPath)) {
        std::cerr << "[!] Input file does not exist.\n";
        return 1;
    }

    std::vector<uint8_t> metadata;
    std::vector<uint8_t> gameAsm;
    if (!loadFile(args.metadataPath, metadata)) {
        std::cerr << "[!] Failed to load metadata file.\n";
        return 1;
    }
    if (!loadFile(args.gameAssemblyPath, gameAsm)) {
        std::cerr << "[!] Failed to load GameAssembly file.\n";
        return 1;
    }

    if (metadata.size() < sizeof(Il2CppGlobalMetadataHeader)) {
        std::cerr << "[!] Metadata is too small.\n";
        return 1;
    }

    const auto* header = reinterpret_cast<const Il2CppGlobalMetadataHeader*>(metadata.data());
    if (header->sanity != 0xFAB11BAF) {
        std::cerr << "[!] Invalid metadata sanity: 0x" << std::hex << header->sanity << std::dec << "\n";
        return 1;
    }

    const uint32_t typeDefCount = header->typeDefinitionsCount / sizeof(Il2CppTypeDefinition);
    const uint32_t methodCount = header->methodsCount / sizeof(Il2CppMethodDefinition);
    const uint32_t fieldCount = header->fieldsCount / sizeof(Il2CppFieldDefinition);
    const uint32_t imageCount = header->imagesCount / sizeof(Il2CppImageDefinition);

    if (!rangeInBounds(header->typeDefinitionsOffset, header->typeDefinitionsCount, metadata.size()) ||
        !rangeInBounds(header->methodsOffset, header->methodsCount, metadata.size()) ||
        !rangeInBounds(header->fieldsOffset, header->fieldsCount, metadata.size()) ||
        !rangeInBounds(header->imagesOffset, header->imagesCount, metadata.size())) {
        std::cerr << "[!] Metadata table range check failed.\n";
        return 1;
    }

    const auto* typeDefs = reinterpret_cast<const Il2CppTypeDefinition*>(metadata.data() + header->typeDefinitionsOffset);
    const auto* methods = reinterpret_cast<const Il2CppMethodDefinition*>(metadata.data() + header->methodsOffset);
    const auto* fields = reinterpret_cast<const Il2CppFieldDefinition*>(metadata.data() + header->fieldsOffset);
    const auto* images = reinterpret_cast<const Il2CppImageDefinition*>(metadata.data() + header->imagesOffset);

    auto pe = parsePE(gameAsm);
    if (!pe) {
        std::cerr << "[!] Failed to parse GameAssembly PE.\n";
        return 1;
    }

    auto table = findBestMethodPointerTable(gameAsm, *pe, methodCount);

    std::vector<uint64_t> methodPtrs;
    if (table) {
        const size_t ptrSize = pe->is64Bit ? 8 : 4;
        const size_t capped = std::min(table->count, static_cast<size_t>(methodCount));
        methodPtrs.reserve(capped);
        for (size_t i = 0; i < capped; ++i) {
            const size_t off = table->fileOffset + i * ptrSize;
            if (!rangeInBounds(off, ptrSize, gameAsm.size())) break;
            const uint64_t ptr = (ptrSize == 8)
                ? *reinterpret_cast<const uint64_t*>(&gameAsm[off])
                : *reinterpret_cast<const uint32_t*>(&gameAsm[off]);
            methodPtrs.push_back(ptr);
        }
    }

    std::ofstream out(args.outputPath);
    if (!out) {
        std::cerr << "[!] Failed to create output file: " << args.outputPath.string() << "\n";
        return 1;
    }

    out << "// IL2CPP Dump\n";
    out << "// MetadataVersion: " << header->version << "\n";
    out << "// TypeCount: " << typeDefCount << "\n";
    out << "// MethodCount: " << methodCount << "\n";
    out << "// FieldCount: " << fieldCount << "\n";
    out << "// PE: " << (pe->is64Bit ? "x64" : "x86") << "\n";
    out << "// RVA mode: " << (table ? "heuristic" : "unresolved") << "\n\n";

    for (uint32_t i = 0; i < imageCount; ++i) {
        const auto& img = images[i];
        const std::string imageName = readCString(metadata, header->stringOffset, img.nameIndex);
        out << "// ===== Image: " << imageName << " =====\n";

        const uint32_t start = (img.typeStart < 0) ? 0u : static_cast<uint32_t>(img.typeStart);
        const uint32_t end = std::min(typeDefCount, start + img.typeCount);

        for (uint32_t t = start; t < end; ++t) {
            const auto& td = typeDefs[t];
            const std::string ns = readCString(metadata, header->stringOffset, td.namespaceIndex);
            const std::string typeName = readCString(metadata, header->stringOffset, td.nameIndex);

            out << "namespace " << (ns.empty() ? "<global>" : ns) << " {\n";
            out << "class " << (typeName.empty() ? "<unnamed_type>" : typeName)
                << " // TypeDefIndex: " << t
                << " Token: 0x" << std::hex << td.token << std::dec
                << "\n{\n";

            for (uint16_t fi = 0; fi < td.field_count; ++fi) {
                const int32_t fIndex = td.fieldStart + fi;
                if (fIndex < 0 || static_cast<uint32_t>(fIndex) >= fieldCount) continue;
                const auto& fd = fields[fIndex];
                const std::string fieldName = readCString(metadata, header->stringOffset, fd.nameIndex);
                out << "    // FieldDefIndex: " << fIndex
                    << " | Token: 0x" << std::hex << fd.token << std::dec << "\n";
                out << "    <field> " << (fieldName.empty() ? "<unnamed_field>" : fieldName) << ";\n";
            }

            if (td.field_count > 0) out << "\n";

            for (uint16_t mi = 0; mi < td.method_count; ++mi) {
                const int32_t mIndex = td.methodStart + mi;
                if (mIndex < 0 || static_cast<uint32_t>(mIndex) >= methodCount) continue;

                const auto& md = methods[mIndex];
                const std::string methodName = readCString(metadata, header->stringOffset, md.nameIndex);
                out << "    // MethodDefIndex: " << mIndex
                    << " | Token: 0x" << std::hex << md.token << std::dec
                    << " | MethodIndex: " << md.methodIndex
                    << " | Params: " << md.parameterCount;

                if (md.methodIndex >= 0 && static_cast<size_t>(md.methodIndex) < methodPtrs.size()) {
                    const uint64_t va = methodPtrs[md.methodIndex];
                    uint32_t rva = 0;
                    if (ptrToRva(*pe, va, rva)) {
                        out << " | RVA: 0x" << std::hex << rva << std::dec;
                    } else {
                        out << " | RVA: N/A";
                    }
                } else {
                    out << " | RVA: N/A";
                }
                out << "\n";

                out << "    void " << (methodName.empty() ? "<unnamed_method>" : methodName) << "();\n\n";
            }

            out << "}\n}\n\n";
        }

        out << "\n";
    }

    std::cout << "[+] Done!\n";
    std::cout << "[+] Output: " << args.outputPath.string() << "\n";
    std::cout << "[+] Metadata version: " << header->version << "\n";
    std::cout << "[+] Types: " << typeDefCount << ", Methods: " << methodCount << ", Fields: " << fieldCount << "\n";
    std::cout << "[+] PE: " << (pe->is64Bit ? "x64" : "x86") << "\n";

    if (table) {
        std::cout << "[+] Method pointer table found at file offset 0x" << std::hex << table->fileOffset << std::dec
                  << " with " << table->count << " entries\n";
    } else {
        std::cout << "[-] Method pointer table not found. Dump still contains classes/methods/fields/tokens.\n";
    }

    return 0;
}
