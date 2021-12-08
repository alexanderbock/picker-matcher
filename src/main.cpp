#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <tracy.hpp>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory_resource>
#include <unordered_map>
#include <vector>

#pragma optimize ("", off)

struct Pickee {
	uint8_t name = 0;  // Index into `Names`
	uint8_t email = 0; // Index into `Emails`
};

struct Picker {
	uint8_t name = 0;  // Index into `Names`
	uint8_t email = 0; // Index into `Emails`
	std::vector<const Pickee*> picks;
};

struct Timeslot {
	std::string name;
};

std::vector<std::string> Names;
std::vector<std::string> Emails;

std::vector<Pickee> Pickees;
std::vector<Picker> Pickers;
std::vector<Timeslot> Timeslots;

using DaySchema = std::vector<const Pickee*>;
using Day = std::vector<DaySchema>;

bool isCompatibleWith(const Day& incompleteDay, const DaySchema& incoming) {
	// Test if the pickees of the incoming schema have been used in the same slot in the
	// day already
	for (size_t i = 0; i < incoming.size(); i += 1) {
		for (const DaySchema& ds : incompleteDay) {
			if (incoming[i] && ds[i] && incoming[i] == ds[i]) {
				return false;
			}
		}
	}
	return true;
}

std::vector<Day> generateDayPermutations() {
	ZoneScoped

	using DaySchemaPermutations = std::vector<DaySchema>;
	std::unordered_map<const Picker*, DaySchemaPermutations> perms;
	std::cout << "Generating permutations...";
	uint64_t nPermutations = 0;
	{
		ZoneScopedN("Generating permutations")
		for (const Picker& picker : Pickers) {
			ZoneScoped

			DaySchemaPermutations permutations;

			// Generate base
			DaySchema picks;
			picks.resize(Timeslots.size(), nullptr);
			for (size_t j = 0; j < picker.picks.size(); j += 1) {
				picks[j] = picker.picks[j];
			}
			std::sort(picks.begin(), picks.end());

			do {
				permutations.push_back(picks);
			} while (std::next_permutation(picks.begin(), picks.end()));

			nPermutations += permutations.size();
			perms[&picker] = std::move(permutations);
		}
	}
	std::cout << fmt::format("Done. ({} permutations)\n", nPermutations);



	std::cout << "Generating indices...";
	std::vector<uint64_t> it;
	std::vector<uint64_t> max;
	max.reserve(Pickers.size());
	for (size_t i = 0; i < Pickers.size(); i += 1) {
		const Picker& picker = Pickers[i];
		const DaySchemaPermutations& perm = perms[&picker];
		max.push_back(static_cast<uint64_t>(perm.size()));

		it.push_back(static_cast<uint64_t>(0));
	}

	//std::mutex mutex;
	std::vector<Day> result;

	//std::vector<uint64_t> first;
	//std::iota(first.begin(), first.end(), 0);
	//std::for_each(
	//	std::execution::par_unseq,
	//	first.begin(), first.end(),
	//	[&mutex, &result, it, &max](uint64_t) {

	//	}
	//);


	{
		Day day;
		day.reserve(Pickers.size());

		ZoneScopedN("Permuting indices")
		while (it[0] != max[0]) {
			ZoneScopedN("Outer loop")
			day.clear();

			for (size_t j = 0; j < it.size(); j += 1) {
				const int pickerIdx = static_cast<int>(j);
				const uint64_t permIdx = it[j];

				const Picker& picker = Pickers[pickerIdx];
				const std::vector<const Pickee*>& permutation = perms[&picker][permIdx];

				if (!isCompatibleWith(day, permutation)) {
					++it[j];
					break;
				}

				day.push_back(permutation);
			}

			if (day.size() == it.size()) {
				//for (size_t j = 0; j < it.size(); j += 1) {
				//	std::cout << int(it[j]) << ' ';
				//}
				//std::cout << '\n';
				result.push_back(std::move(day));

			}

			++it[max.size() - 1];
			for (int i = max.size() - 1; (i > 0) && it[i] >= (max[i] - 1); --i) {
				ZoneScopedN("Inner loop")

				it[i] = static_cast<uint64_t>(0);
				++it[i - 1];
			}
		}
	}
	std::cout << fmt::format("Done. ({} indices)\n", result.size());
	return result;
}

using namespace nlohmann;

int main(int argc, char** argv) {

	//
	// Load the data
	//
	if (argc != 2) {
		std::cerr << "Application must be started with the data file as an argument\n";
		return EXIT_FAILURE;
	}

	std::filesystem::path path = argv[1];
	if (!std::filesystem::exists(path)) {
		std::cerr << "Could not find provided data file\n";
		return EXIT_FAILURE;
	}
	
	json data;
	{
		std::ifstream i(path);
		i >> data;
	}


	//
	// Parse the data
	//
	for (const json& pickee : data.at("pickees")) {
		Pickee p;
		std::string name = pickee.at("name").get<std::string>();
		Names.push_back(name);
		assert(Names.size() < std::numeric_limits<uint8_t>::max());
		p.name = static_cast<uint8_t>(Names.size() - 1);
		
		std::string email = pickee.at("email").get<std::string>();
		Emails.push_back(email);
		assert(Emails.size() < std::numeric_limits<uint8_t>::max());
		p.email = static_cast<uint8_t>(Emails.size() - 1);
		Pickees.push_back(std::move(p));
	}

	for (const json& picker : data.at("pickers")) {
		Picker p;
		std::string name = picker.at("name").get<std::string>();
		Names.push_back(name);
		assert(Names.size() < std::numeric_limits<uint8_t>::max());
		p.name = static_cast<uint8_t>(Names.size() - 1);

		std::string email = picker.at("email").get<std::string>();
		Emails.push_back(email);
		assert(Emails.size() < std::numeric_limits<uint8_t>::max());
		p.email = static_cast<uint8_t>(Emails.size() - 1);

		std::vector<std::string> picks =
			picker.at("picks").get<std::vector<std::string>>();
		for (const std::string& pick : picks) {
			const auto it = std::find_if(
				Pickees.cbegin(), Pickees.cend(),
				[&pick](const Pickee& p) { return pick == Names[p.name]; }
			);

			if (it == Pickees.cend()) {
				std::cerr <<
					fmt::format("Could not find pick {} of picker {}\n", pick, p.name);
				return EXIT_FAILURE;
			}

			p.picks.push_back(&*it);
		}

		Pickers.push_back(std::move(p));
	}

	for (const json& picker : data.at("timeslots")) {
		Timeslot t;
		t.name = picker.at("name").get<std::string>();
		Timeslots.push_back(std::move(t));
	}


	//
	// Generate all permutations of the days
	//
	std::vector<Day> days = generateDayPermutations();



	return EXIT_SUCCESS;
}
