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

using DaySchema = std::vector<const Pickee*>;
using Day = std::vector<DaySchema>;

std::vector<Day> generateDayPermutations(const std::vector<Picker>& pickers,
									     const std::vector<Timeslot>& timeslots)
{
	using DaySchemaPermutations = std::vector<DaySchema>;
	std::unordered_map<const Picker*, DaySchemaPermutations> perms;
	std::cout << "Generating permutations...";
	uint64_t nPermutations = 0;
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

		nPermutations += permutations.size();
		perms[&picker] = std::move(permutations);
	}
	std::cout << fmt::format("Done. ({} permutations)\n", nPermutations);

	std::cout << "Generating indices...";
	std::vector<std::vector<int>> iotas;
	iotas.resize(pickers.size());
	for (size_t i = 0; i < pickers.size(); i += 1) {
		const Picker& picker = pickers[i];
		const DaySchemaPermutations& perm = perms[&picker];
		iotas[i].resize(perm.size());
		std::iota(iotas[i].begin(), iotas[i].end(), 0);
	}

	std::vector<std::vector<int>> indices;

	// Initialize iterator vector
	std::vector<std::vector<int>::const_iterator> it;
	for (size_t i = 0; i < iotas.size(); i += 1) {
		it.push_back(iotas[i].begin());
	}

	while (it[0] != iotas[0].end()) {
		// process the pointed-to elements
		std::vector<int> r;
		for (std::vector<int>::const_iterator jj : it) {
			r.push_back(*jj);
		}
		indices.push_back(std::move(r));

		// the following increments the "odometer" by 1
		++it[iotas.size() - 1];
		for (int i = iotas.size() - 1; (i > 0) && (it[i] == iotas[i].end()); --i) {
			it[i] = iotas[i].begin();
			++it[i - 1];
		}
	}
	std::cout << fmt::format("Done. ({} indices)\n", indices.size());

	std::cout << "Applying permutations...";
	std::vector<Day> result;
	result.reserve(indices.size());
	for (const std::vector<int>& dayIdx : indices) {
		Day day;
		day.reserve(pickers.size());
		for (size_t i = 0; i < dayIdx.size(); i +=1 ) {
			const int pickerIdx = static_cast<int>(i);
			const int permIdx = dayIdx[i];

			const Picker& picker = pickers[pickerIdx];
			const std::vector<const Pickee*>& permutation = perms[&picker][permIdx];
			day.push_back(permutation);
		}
		result.push_back(std::move(day));
	}
	std::cout << "Done.\n";
	return result;
}

bool hasDoubleBooking(const Day& day) {
	assert(
		std::all_of(
			day.cbegin(), day.cend(),
			[&day](const DaySchema& ds) { return ds.size() == day.front().size(); }
		)
	);

	for (size_t i = 0; i < day.front().size(); i += 1) {
		// Get the i-th Pickee out of every DaySchema

		std::vector<const Pickee*> transpose;
		transpose.reserve(day.front().size());
		for (const DaySchema& ds : day) {
			// We need to ignore the empty ones as they would otherwise trip the duplicate
			// check below
			if (ds[i]) {
				transpose.push_back(ds[i]);
			}
		}

		// We sort all of the values, and then check if there are two adjacent elements.
		// If so, then we had a duplicate booking and we can eliminate the day
		std::sort(transpose.begin(), transpose.end());
		auto adjacent = std::adjacent_find(transpose.begin(), transpose.end());
		if (adjacent != transpose.end()) {
			return true;
		}
	}

	// If we get this far, there have not been any duplicate entries 
	return false;
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
	std::vector<Pickee> pickees;
	for (const json& pickee : data.at("pickees")) {
		Pickee p;
		p.name = pickee.at("name").get<std::string>();
		p.email = pickee.at("email").get<std::string>();
		pickees.push_back(std::move(p));
	}

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

	std::vector<Timeslot> timeslots;
	for (const json& picker : data.at("timeslots")) {
		Timeslot t;
		t.name = picker.at("name").get<std::string>();
		timeslots.push_back(std::move(t));
	}


	//
	// Generate all permutations of the days
	//
	std::vector<Day> days = generateDayPermutations(pickers, timeslots);
	days.erase(std::remove_if(days.begin(), days.end(), hasDoubleBooking), days.end());



	return EXIT_SUCCESS;
}
