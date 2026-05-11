#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using namespace std;

static constexpr uint64_t JAVA_MULT = 25214903917ULL;
static constexpr uint64_t JAVA_ADD  = 11ULL;
static constexpr uint64_t JAVA_MASK = (1ULL << 48) - 1;

static constexpr int TEMPLE_SPACING = 32;
static constexpr int TEMPLE_SEPARATION = 8;
static constexpr int TEMPLE_OFFSET = TEMPLE_SPACING - TEMPLE_SEPARATION;
static constexpr int TEMPLE_SALT = 14357617;

struct JavaRandom {
    uint64_t seed = 0;

    void setSeed(int64_t s, bool scramble = true) {
        uint64_t u = static_cast<uint64_t>(s);
        seed = (scramble ? (u ^ JAVA_MULT) : u) & JAVA_MASK;
    }

    int32_t nextBits(int bits) {
        seed = (seed * JAVA_MULT + JAVA_ADD) & JAVA_MASK;
        uint64_t v = seed >> (48 - bits);

        if (bits == 32 && v >= 0x80000000ULL) {
            return static_cast<int32_t>(v - 0x100000000ULL);
        }

        return static_cast<int32_t>(v);
    }

    int32_t nextInt(int bound) {
        if (bound <= 0) throw runtime_error("nextInt bound must be positive");

        if ((bound & -bound) == bound) {
            return static_cast<int32_t>((static_cast<int64_t>(bound) * nextBits(31)) >> 31);
        }

        int32_t bits, value;
        do {
            bits = nextBits(31);
            value = bits % bound;
        } while (bits - value + (bound - 1) < 0);

        return value;
    }

    int64_t nextLong() {
        int64_t hi = static_cast<int64_t>(nextBits(32));
        int64_t lo = static_cast<int64_t>(nextBits(32));
        return hi * (1LL << 32) + lo;
    }

    void advance(uint64_t calls) {
        for (uint64_t i = 0; i < calls; i++) {
            nextBits(32);
        }
    }
};

static int64_t truncDiv2(int64_t x) {
    if (x >= 0) return x / 2;
    return -((-x) / 2);
}

static void setPopulationSeed(JavaRandom& rng, int64_t worldSeed, int chunkX, int chunkZ) {
    rng.setSeed(worldSeed);

    int64_t a = truncDiv2(rng.nextLong()) * 2 + 1;
    int64_t b = truncDiv2(rng.nextLong()) * 2 + 1;

    uint64_t ux = static_cast<uint64_t>(static_cast<int64_t>(chunkX));
    uint64_t uz = static_cast<uint64_t>(static_cast<int64_t>(chunkZ));
    uint64_t ua = static_cast<uint64_t>(a);
    uint64_t ub = static_cast<uint64_t>(b);
    uint64_t ws = static_cast<uint64_t>(worldSeed);

    uint64_t seed = (ux * ua + uz * ub) ^ ws;
    rng.setSeed(static_cast<int64_t>(seed));
}

static void setRegionSeed(JavaRandom& rng, int64_t worldSeed, int regionX, int regionZ) {
    uint64_t seed =
        static_cast<uint64_t>(static_cast<int64_t>(regionX)) * 341873128712ULL +
        static_cast<uint64_t>(static_cast<int64_t>(regionZ)) * 132897987541ULL +
        static_cast<uint64_t>(worldSeed) +
        static_cast<uint64_t>(TEMPLE_SALT);

    rng.setSeed(static_cast<int64_t>(seed));
}

struct ChunkPos {
    int x;
    int z;
};

static ChunkPos templeInRegion(int64_t seed, int regionX, int regionZ) {
    JavaRandom rng;
    setRegionSeed(rng, seed, regionX, regionZ);

    return {
        regionX * TEMPLE_SPACING + rng.nextInt(TEMPLE_OFFSET),
        regionZ * TEMPLE_SPACING + rng.nextInt(TEMPLE_OFFSET)
    };
}

struct Enchant {
    string name;
    int level;

    bool operator==(const Enchant& other) const {
        return name == other.name && level == other.level;
    }

    bool operator<(const Enchant& other) const {
        return tie(name, level) < tie(other.name, other.level);
    }
};

struct Stack {
    string item;
    int count;
    vector<Enchant> enchants;
};

static const vector<pair<string, int>> ENCHANTS = {
    {"protection", 4},
    {"fire_protection", 4},
    {"feather_falling", 4},
    {"blast_protection", 4},
    {"projectile_protection", 4},
    {"respiration", 3},
    {"aqua_affinity", 1},
    {"thorns", 3},
    {"depth_strider", 3},
    {"frost_walker", 2},
    {"binding_curse", 1},
    {"sharpness", 5},
    {"smite", 5},
    {"bane_of_arthropods", 5},
    {"knockback", 2},
    {"fire_aspect", 2},
    {"looting", 3},
    {"sweeping", 3},
    {"efficiency", 5},
    {"silk_touch", 1},
    {"unbreaking", 3},
    {"fortune", 3},
    {"power", 5},
    {"punch", 2},
    {"flame", 1},
    {"infinity", 1},
    {"luck_of_the_sea", 3},
    {"lure", 3},
    {"mending", 1},
    {"vanishing_curse", 1}
};

static bool isSingleLevelEnchant(const string& name) {
    return
        name == "aqua_affinity" ||
        name == "binding_curse" ||
        name == "silk_touch" ||
        name == "flame" ||
        name == "infinity" ||
        name == "mending" ||
        name == "vanishing_curse";
}

static Enchant randomEnchant(JavaRandom& rng) {
    auto [name, maxLevel] = ENCHANTS[rng.nextInt(static_cast<int>(ENCHANTS.size()))];

    int level = 1;
    if (!isSingleLevelEnchant(name)) {
        level = rng.nextInt(maxLevel) + 1;
    }

    return {name, level};
}

struct Entry {
    string item;
    int weight;
    int minCount;
    int maxCount;
    bool enchantedBook;
    bool empty;
};

static const vector<Entry> RARE_POOL = {
    {"diamond", 5, 1, 3, false, false},
    {"iron_ingot", 15, 1, 5, false, false},
    {"gold_ingot", 15, 2, 7, false, false},
    {"emerald", 15, 1, 3, false, false},
    {"bone", 25, 4, 6, false, false},
    {"spider_eye", 25, 1, 3, false, false},
    {"rotten_flesh", 25, 3, 7, false, false},
    {"saddle", 20, 1, 1, false, false},
    {"iron_horse_armor", 15, 1, 1, false, false},
    {"golden_horse_armor", 10, 1, 1, false, false},
    {"diamond_horse_armor", 5, 1, 1, false, false},
    {"enchanted_book", 20, 1, 1, true, false},
    {"golden_apple", 20, 1, 1, false, false},
    {"enchanted_golden_apple", 2, 1, 1, false, false},
    {"", 15, 0, 0, false, true}
};

static const vector<Entry> COMMON_POOL = {
    {"bone", 10, 1, 8, false, false},
    {"gunpowder", 10, 1, 8, false, false},
    {"rotten_flesh", 10, 1, 8, false, false},
    {"string", 10, 1, 8, false, false},
    {"sand", 10, 1, 8, false, false}
};

static optional<Stack> rollEntry(const vector<Entry>& pool, JavaRandom& rng) {
    int totalWeight = 0;
    for (const auto& e : pool) totalWeight += e.weight;

    int pick = rng.nextInt(totalWeight);
    int cursor = 0;

    for (const auto& e : pool) {
        if (pick >= cursor + e.weight) {
            cursor += e.weight;
            continue;
        }

        if (e.empty) return nullopt;

        int count = e.minCount;
        if (e.maxCount > e.minCount) {
            count = rng.nextInt(e.maxCount - e.minCount + 1) + e.minCount;
        }

        Stack s{e.item, count, {}};

        if (e.enchantedBook) {
            s.enchants.push_back(randomEnchant(rng));
        }

        return s;
    }

    throw runtime_error("weighted roll failed");
}

static vector<Stack> mergeStacks(const vector<Stack>& raw) {
    map<pair<string, vector<Enchant>>, int> merged;

    for (const auto& s : raw) {
        merged[{s.item, s.enchants}] += s.count;
    }

    vector<Stack> out;
    for (const auto& [key, count] : merged) {
        out.push_back({key.first, count, key.second});
    }

    return out;
}

static vector<Stack> generateChest(int64_t lootSeed) {
    JavaRandom rng;
    rng.setSeed(lootSeed);

    vector<Stack> raw;

    int rareRolls = rng.nextInt(3) + 2; // Uniform 2..4
    for (int i = 0; i < rareRolls; i++) {
        auto s = rollEntry(RARE_POOL, rng);
        if (s) raw.push_back(*s);
    }

    for (int i = 0; i < 4; i++) {
        auto s = rollEntry(COMMON_POOL, rng);
        if (s) raw.push_back(*s);
    }

    return mergeStacks(raw);
}

static vector<vector<Stack>> generateTempleLoot(int64_t seed, ChunkPos temple) {
    JavaRandom pop;
    setPopulationSeed(pop, seed, temple.x, temple.z);

    vector<vector<Stack>> chests;
    for (int i = 0; i < 4; i++) {
        chests.push_back(generateChest(pop.nextLong()));
    }

    return chests;
}

static int countItem(const vector<Stack>& chest, const string& item) {
    int total = 0;

    for (const auto& s : chest) {
        if (s.item == item && s.enchants.empty()) {
            total += s.count;
        }
    }

    return total;
}

static bool hasBook(const vector<Stack>& chest, const string& name, int level) {
    for (const auto& s : chest) {
        if (s.item != "enchanted_book") continue;

        for (const auto& e : s.enchants) {
            if (e.name == name && e.level == level) return true;
        }
    }

    return false;
}

static bool isExactBook(const Stack& s, const string& name, int level) {
    return
        s.item == "enchanted_book" &&
        s.enchants.size() == 1 &&
        s.enchants[0].name == name &&
        s.enchants[0].level == level;
}

static bool matchFirstChest(const vector<Stack>& chest) {
    if (countItem(chest, "bone") != 13) return false;
    if (countItem(chest, "rotten_flesh") != 9) return false;
    if (countItem(chest, "spider_eye") != 2) return false;

    int exactBooks = 0;

    for (const auto& s : chest) {
        if (s.item == "bone" || s.item == "rotten_flesh" || s.item == "spider_eye") {
            continue;
        }

        if (isExactBook(s, "fire_aspect", 1)) {
            exactBooks++;
            continue;
        }

        return false;
    }

    return exactBooks == 1;
}

static bool matchSecondChest(const vector<Stack>& chest) {
    if (countItem(chest, "bone") != 16) return false;
    if (countItem(chest, "sand") != 4) return false;
    if (countItem(chest, "saddle") != 1) return false;
    if (countItem(chest, "golden_apple") != 1) return false;
    if (countItem(chest, "rotten_flesh") != 4) return false;

    for (const auto& s : chest) {
        if (
            s.item == "bone" ||
            s.item == "sand" ||
            s.item == "saddle" ||
            s.item == "golden_apple" ||
            s.item == "rotten_flesh"
        ) {
            continue;
        }

        return false;
    }

    return true;
}

struct Region {
    int x;
    int z;
};

struct Hit {
    int64_t seed;
    Region region;
    ChunkPos temple;
    int firstChest;
    int secondChest;
    vector<vector<Stack>> chests;
};

static optional<Hit> findHit(int64_t seed, Region region, ChunkPos temple, const vector<vector<Stack>>& chests) {
    for (int first = 0; first < 4; first++) {
        if (!matchFirstChest(chests[first])) continue;

        for (int second = 0; second < 4; second++) {
            if (second == first) continue;
            if (!matchSecondChest(chests[second])) continue;

                    return Hit{seed, region, temple, first, second, chests};
            for (int resp = 0; resp < 4; resp++) {
                if (resp == first || resp == second) continue;
                if (!hasBook(chests[resp], "respiration", 2)) continue;

                for (int inf = 0; inf < 4; inf++) {
                    if (inf == first || inf == second || inf == resp) continue;
                    if (!hasBook(chests[inf], "infinity", 1)) continue;

                }
            }
        }
    }

    return nullopt;
}

static string stackToString(const Stack& s) {
    stringstream ss;
    ss << s.count << "x " << s.item;

    if (!s.enchants.empty()) {
        ss << " [";
        for (size_t i = 0; i < s.enchants.size(); i++) {
            if (i) ss << ", ";
            ss << "(" << s.enchants[i].name << ", " << s.enchants[i].level << ")";
        }
        ss << "]";
    }

    return ss.str();
}

static string formatHit(const Hit& h) {
    stringstream ss;

    ss << "SEED " << h.seed
       << " templeChunk=(" << h.temple.x << "," << h.temple.z << ")"
       << " approxBlock=(" << (h.temple.x << 4) << "," << (h.temple.z << 4) << ")"
       << " region=(" << h.region.x << "," << h.region.z << ")"
       << " firstChest=" << h.firstChest
       << " secondChest=" << h.secondChest
       << "\n";

    for (int i = 0; i < 4; i++) {
        ss << "CHEST_" << i << "\n";
        for (const auto& s : h.chests[i]) {
            ss << "  - " << stackToString(s) << "\n";
        }
    }

    ss << "---\n";
    return ss.str();
}

static vector<int64_t> readSeeds(const string& path) {
    ifstream in(path);
    if (!in) throw runtime_error("cannot open seeds file: " + path);

    vector<int64_t> seeds;
    string line;

    while (getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        seeds.push_back(stoll(line));
    }

    return seeds;
}

static vector<Region> buildRegions(int radius) {
    vector<Region> regions;

    for (int x = -radius; x <= radius; x++) {
        for (int z = -radius; z <= radius; z++) {
            regions.push_back({x, z});
        }
    }

    sort(regions.begin(), regions.end(), [](const Region& a, const Region& b) {
        int64_t da = int64_t(a.x) * a.x + int64_t(a.z) * a.z;
        int64_t db = int64_t(b.x) * b.x + int64_t(b.z) * b.z;
        return da < db;
    });

    return regions;
}

struct Counters {
    atomic<uint64_t> seedsDone{0};
    atomic<uint64_t> regionsDone{0};
    atomic<uint64_t> lootDone{0};
    atomic<uint64_t> firstMatches{0};
    atomic<uint64_t> secondMatches{0};
    atomic<uint64_t> bothMatches{0};
    atomic<uint64_t> hits{0};
};

static string duration(uint64_t seconds) {
    uint64_t h = seconds / 3600;
    uint64_t m = (seconds % 3600) / 60;
    uint64_t s = seconds % 60;

    stringstream ss;
    if (h) ss << h << "h " << (m < 10 ? "0" : "") << m << "m " << (s < 10 ? "0" : "") << s << "s";
    else if (m) ss << m << "m " << (s < 10 ? "0" : "") << s << "s";
    else ss << s << "s";

    return ss.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage:\n";
        cerr << "  " << argv[0] << " seeds.txt [hits.txt] [threads] [regionRadius]\n\n";
        cerr << "Example:\n";
        cerr << "  " << argv[0] << " seeds.txt hits.txt 32 32\n";
        return 1;
    }

    string seedsPath = argv[1];
    string outPath = argc >= 3 ? argv[2] : "hits.txt";
    int threads = argc >= 4 ? stoi(argv[3]) : 32;
    int radius = argc >= 5 ? stoi(argv[4]) : 32;

    vector<int64_t> seeds = readSeeds(seedsPath);
    vector<Region> regions = buildRegions(radius);

    cerr << "Seeds: " << seeds.size() << "\n";
    cerr << "Threads: " << threads << "\n";
    cerr << "Region radius: " << radius << "\n";
    cerr << "Regions per seed: " << regions.size() << "\n";
    cerr << "Output: " << outPath << "\n\n";

    ofstream out(outPath);
    if (!out) throw runtime_error("cannot open output file: " + outPath);

    atomic<size_t> nextSeed{0};
    atomic<bool> running{true};
    Counters counters;
    mutex outMutex;

    auto start = chrono::steady_clock::now();

    thread reporter([&]() {
        uint64_t lastSeeds = 0;
        uint64_t lastRegions = 0;
        auto lastTime = chrono::steady_clock::now();

        while (running.load()) {
            this_thread::sleep_for(chrono::seconds(5));

            auto now = chrono::steady_clock::now();
            double elapsed = chrono::duration<double>(now - start).count();
            double recent = chrono::duration<double>(now - lastTime).count();

            uint64_t sd = counters.seedsDone.load();
            uint64_t rd = counters.regionsDone.load();

            double avgSeeds = sd / max(elapsed, 1.0);
            double avgRegions = rd / max(elapsed, 1.0);
            double recentSeeds = (sd - lastSeeds) / max(recent, 1.0);
            double recentRegions = (rd - lastRegions) / max(recent, 1.0);

            uint64_t remaining = seeds.size() > sd ? seeds.size() - sd : 0;
            uint64_t eta = avgSeeds > 0 ? uint64_t(remaining / avgSeeds) : 0;

            double pct = seeds.empty() ? 0.0 : 100.0 * double(sd) / double(seeds.size());

            cerr << "[progress] seeds=" << sd << "/" << seeds.size()
                 << " " << pct << "%"
                 << " | regions=" << rd
                 << " | loot=" << counters.lootDone.load()
                 << " | first=" << counters.firstMatches.load()
                 << " | second=" << counters.secondMatches.load()
                 << " | both=" << counters.bothMatches.load()
                 << " | hits=" << counters.hits.load()
                 << " | avg=" << avgSeeds << " seeds/s, " << avgRegions << " regions/s"
                 << " | recent=" << recentSeeds << " seeds/s, " << recentRegions << " regions/s"
                 << " | ETA=" << duration(eta)
                 << "\n";

            lastSeeds = sd;
            lastRegions = rd;
            lastTime = now;
        }
    });

    vector<thread> workers;

    for (int t = 0; t < threads; t++) {
        workers.emplace_back([&]() {
            uint64_t localRegions = 0;
            uint64_t localLoot = 0;
            uint64_t localFirst = 0;
            uint64_t localSecond = 0;
            uint64_t localBoth = 0;

            auto flush = [&]() {
                counters.regionsDone += localRegions;
                counters.lootDone += localLoot;
                counters.firstMatches += localFirst;
                counters.secondMatches += localSecond;
                counters.bothMatches += localBoth;

                localRegions = 0;
                localLoot = 0;
                localFirst = 0;
                localSecond = 0;
                localBoth = 0;
            };

            while (true) {
                size_t idx = nextSeed.fetch_add(1);
                if (idx >= seeds.size()) break;

                int64_t seed = seeds[idx];

                for (const auto& region : regions) {
                    localRegions++;

                    ChunkPos temple = templeInRegion(seed, region.x, region.z);
                    auto chests = generateTempleLoot(seed, temple);
                    localLoot++;

                    int firstIdx = -1;
                    int secondIdx = -1;

                    for (int i = 0; i < 4; i++) {
                        if (firstIdx < 0 && matchFirstChest(chests[i])) firstIdx = i;
                        if (secondIdx < 0 && matchSecondChest(chests[i])) secondIdx = i;
                    }

                    if (firstIdx >= 0) localFirst++;
                    if (secondIdx >= 0) localSecond++;
                    if (firstIdx >= 0 && secondIdx >= 0 && firstIdx != secondIdx) localBoth++;

                    auto hit = findHit(seed, region, temple, chests);
                    if (hit) {
                        lock_guard<mutex> lock(outMutex);

                        counters.hits++;
                        out << formatHit(*hit);
                        out.flush();

                        cout << "BtSCTF{" << seed << "}\n";
                        cout << formatHit(*hit);
                    }

                    if ((localRegions & 0x3FFF) == 0) {
                        flush();
                    }
                }

                flush();
                counters.seedsDone++;
            }

            flush();
        });
    }

    for (auto& w : workers) w.join();

    running = false;
    reporter.join();

    cerr << "\nDone.\n";
    cerr << "Seeds checked: " << counters.seedsDone.load() << "\n";
    cerr << "Regions checked: " << counters.regionsDone.load() << "\n";
    cerr << "Loot checked: " << counters.lootDone.load() << "\n";
    cerr << "First chest matches: " << counters.firstMatches.load() << "\n";
    cerr << "Second chest matches: " << counters.secondMatches.load() << "\n";
    cerr << "Both chest matches: " << counters.bothMatches.load() << "\n";
    cerr << "Full hits: " << counters.hits.load() << "\n";

    return 0;
}
