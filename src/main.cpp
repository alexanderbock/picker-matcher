#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

struct Pickee {
	std::string name;
	std::string email;
};

struct Picker {
	std::string name;
	std::string email;
	std::vector<const Pickee*> picks;
};

struct Timeslot {
	std::string name;
};

using namespace nlohmann;

int main(int argc, char** argv) {
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


	// Parse Pickees
	std::vector<Pickee> pickees;
	for (const json& pickee : data.at("pickees")) {
		Pickee p;
		p.name = pickee.at("name").get<std::string>();
		p.email = pickee.at("email").get<std::string>();
		pickees.push_back(std::move(p));
	}

	// Parse Pickers
	std::vector<Picker> pickers;
	for (const json& picker : data.at("pickers")) {
		Picker p;
		p.name = picker.at("name").get<std::string>();
		p.email = picker.at("email").get<std::string>();

		std::vector<std::string> picks =
			picker.at("picks").get<std::vector<std::string>>();
		for (const std::string& pick : picks) {
			const auto it = std::find_if(
				pickees.cbegin(), pickees.cend(),
				[&pick](const Pickee& p) { return pick == p.name; }
			);

			if (it == pickees.cend()) {
				std::cerr <<
					fmt::format("Could not find pick {} of picker {}\n", pick, p.name);
				return EXIT_FAILURE;
			}

			p.picks.push_back(&*it);
		}

		pickers.push_back(std::move(p));
	}

	// Parse time slots
	std::vector<Timeslot> timeslots;
	for (const json& picker : data.at("timeslots")) {
		Timeslot t;
		t.name = picker.at("name").get<std::string>();
		timeslots.push_back(std::move(t));
	}

	using DaySchema = std::vector<const Pickee*>;
	using DaySchemaPermutations = std::vector<DaySchema>;
	std::unordered_map<const Picker*, DaySchemaPermutations> perms;
	for (const Picker& picker : pickers) {
		DaySchemaPermutations permutations;

		// Generate base
		DaySchema picks;
		picks.resize(timeslots.size(), nullptr);
		for (size_t j = 0; j < picker.picks.size(); j += 1) {
			picks[j] = picker.picks[j];
		}
		std::sort(picks.begin(), picks.end());

		do {
			permutations.push_back(picks);
		} while (std::next_permutation(picks.begin(), picks.end()));

		perms[&picker] = std::move(permutations);
	}

	//const uint64_t t1 = static_cast<uint64_t>(std::tgamma(timeslots.size()));
	//const uint64_t nCombinations = static_cast<uint64_t>(std::pow(t1, pickers.size()));

	//using Day = std::unordered_map<const Picker*, DaySchema>;
	//std::vector<Day> combinations;
	//combinations.reserve(nCombinations);
	//
	//auto func = []( ) {
	//};

	//for (const Picker& picker : pickers) {
	//	
	//}

	//using DayIdx = std::vector<int>;
	//std::vector<DayIdx> combinations;


	//auto func = []() {

	//};


	std::vector<std::vector<int>> indices;
	indices.resize(pickers.size());
	for (size_t i = 0; i < pickers.size(); i += 1) {
		const Picker& picker = pickers[i];
		const DaySchemaPermutations& perm = perms[&picker];
		indices[i].resize(perm.size());
		std::iota(indices[i].begin(), indices[i].end(), 0);
	}



	std::vector<std::vector<int>> result;

	// Initialize iterator vector
	std::vector<std::vector<int>::const_iterator> it;
	for (size_t i = 0; i < indices.size(); i += 1) {
		it.push_back(indices[i].begin());
	}

	while (it[0] != indices[0].end()) {
		// process the pointed-to elements
		std::vector<int> r;
		for (std::vector<int>::const_iterator jj : it) {
			r.push_back(*jj);
		}
		result.push_back(r);

		// the following increments the "odometer" by 1
		++it[indices.size() - 1];
		for (int i = indices.size() - 1; (i > 0) && (it[i] == indices[i].end()); --i) {
			it[i] = indices[i].begin();
			++it[i - 1];
		}
	}



	return EXIT_SUCCESS;
}
