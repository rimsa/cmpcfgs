/*

   Compare two CFGs in CFGgrind format.

   Copyright (C) 2019, Andrei Rimsa (andrei@cefetmg.br)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include <set>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <getopt.h>

#include <SimpleStrategy.h>
#include <SpecificStrategy.h>

inline std::string& ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

inline std::string& rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

inline std::string& trim(std::string &s) {
	return rtrim(ltrim(s));
}

void usage(char* progname) {
	std::cout << "Usage: " << progname << " <Options> [CFG file A] [CFG file B]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "   -c               Compress CFGs (merge sequential nodes when possible)" << std::endl;
	std::cout << "   -p               Print detailed statistics for each CFG" << std::endl;
	std::cout << "   -s   Strategy    Comparission strategy, where Strategy is:" << std::endl;
	std::cout << "                        simple: high level comparisson [default]" << std::endl;
	std::cout << "                        specific: fine-grain comparisson" << std::endl;
	std::cout << "   -b               Consider only CFGs in both files" << std::endl;
	std::cout << "   -r   Range       Consider only CFGs in the range (start:end)" << std::endl;
	std::cout << "                        can be used multiple times" << std::endl;
	std::cout << "   -a   Addr        Consider only CFGs with given address" << std::endl;
	std::cout << "                        can be used multiple times" << std::endl;
	std::cout << "   -A   File        Load file with addresses, one address per line" << std::endl;
	std::cout << "                        can be used multiple times" << std::endl;
	std::cout << "   -i   File        Instructions map (address:size:assembly per entry) file" << std::endl;
	std::cout << "   -o   File        Output statistics report file" << std::endl;
	std::cout << "   -d   Directory   Dump DOT cfgs in directory" << std::endl;
	std::cout << std::endl;

	exit(1);
}

StrategyConfig readoptions(int argc, char* argv[]) {
	int opt;
	char* idx;
	Addr start, end;
	std::ifstream input;
	StrategyConfig config;

	while ((opt = getopt(argc, argv, ":cps:br:a:A:i:o:d:")) != -1) {
		switch (opt) {
			case 'c':
				config.compress = true;
				break;
			case 'p':
				config.detailed = true;
				break;
			case 's':
				if (strcasecmp(optarg, "simple") == 0)
					config.specific = false;
				else if (strcasecmp(optarg, "specific") == 0)
					config.specific = true;
				else
					throw std::string("Invalid strategy: ") + optarg;
				break;
			case 'b':
				config.both = true;
				break;
			case 'r':
				idx = strchr(optarg, ':');
				if (!idx)
					throw std::string("invalid range: ") + optarg;

				*idx = 0;
				idx++;

				if (strncasecmp(optarg, "0x", 2))
					optarg += 2;
				if (strncasecmp(idx, "0x", 2))
					idx += 2;

				start = std::stoul(optarg, 0, 16);
				end = std::stoul(idx, 0, 16);

				if (end < start) {
					std::stringstream ss;
					ss << std::hex;
					ss << "invalid range: " << end << " < " << start;

					throw ss.str();
				}

				config.ranges.push_back(std::make_pair(start, end));
				break;
			case 'a':
				if (strncasecmp(optarg, "0x", 2))
					optarg += 2;

				start = std::stoul(optarg, 0, 16);
				if (start != 0)
					config.ranges.push_back(std::make_pair(start, start));
				break;
			case 'A':
				input = std::ifstream(optarg);
				for (std::string line; getline(input, line); ) {
					trim(line);

					if (line.length() >= 2 && line.compare(0, 2, "0x"))
						line = line.substr(2);

					if (line.empty())
						continue;

					start = std::stoul(line, 0, 16);
					if (start != 0)
						config.ranges.push_back(std::make_pair(start, start));
				}
				input.close();

				break;
			case 'i':
				config.instrs = optarg;
				break;
			case 'o':
				config.output = optarg;
				break;
			case 'd':
				config.dump = optarg;
				break;
			default:
				throw std::string("Invalid option: ") + (char) optopt;
		}
	}

	if (optind+1 >= argc)
		usage(argv[0]);

	config.input1 = argv[optind++];
	config.input2 = argv[optind++];

	if (optind < argc)
		throw std::string("Unknown extra option: ") + argv[optind];

	if (!config.both && config.specific)
		throw std::string("-b must be used with specific strategy");

	return config;
}

int main(int argc, char* argv[]) {
	StrategyConfig config;
	Strategy* strategy = 0;

	try {
		config = readoptions(argc, argv);
		if (config.specific)
			strategy = new SpecificStrategy(config);
		else
			strategy = new SimpleStrategy(config);

		strategy->process();
	} catch (const std::string& e) {
		std::cerr << e << std::endl;
	}

	if (strategy)
		delete strategy;

	return 0;
}
