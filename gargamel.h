/*
 * gargamel.h
 * author: Jameson Thatcher (a.k.a. SirEel). jameson@bluescreenofdoom.com
 *
 * This system provides a quick and easy way of dealing with commandline
 * arguments. It requires a C++11 compliant compiler.
 *
 * USAGE
 *
 * You need an enum (or enum class) uniquely identifying all commandline
 * arguments supported, and an array of descriptors, for example:
 *
 * enum EArgs { arg1, arg2, arg3 };
 * Gargamel::ArgumentList Arguments {
 *	GGM_DESCRIBE_ARG        (arg1, 'a',  "alpha",  NoArgument,               "help message")
 *	GGM_DESCRIBE_ARG_DEFAULT(arg2, '\0', "dinner", RequiredArgument, "eggs", "dinner")
 *	GGM_DESCRIBE_ARG_ARRAY  (arg3, '\0', "shopping")
 * };
 *
 * To set Gargamel up to use an argument list, call Gargamel::SetArguments(). Its
 * second argument is the number of 'positional' arguments expected - this many
 * arguments from the end of the argument list will be ignored for processing.
 * Use Gargamel::Process() to set up the actual values, passing in argc and argv
 * for it to read. The results may then be queried like so:
 *
 * Gargamel::ArgumentValues[Identifier].isArgumentPresent; //a flag
 * Gargamel::ArgumentValues[Identifier].argumentValue;     //a string
 * Gargamel::ArgumentValues[Identifier].argumentArray;     //an array of strings.
 *
 * There are additionaly some courtesy functions for querying the value as an int
 * or a float. The functions intValue() and floatValue() take an optional index
 * to allow querying into the array.
 *
 * MACRO ARGUMENTS
 *
 * shortName - an argument that may be used like "-h or -X". These can be
 *             combined like "-hX"
 *
 * longName - A longer arguement like "---help"
 *
 * style - one of "NoArgument", "OptionalArgument", "RequiredArgument". Only
 *         long arguments can take an argument, and it is done like so:
 *         "--something theArgument"
 *
 * defaultVal - This will be populated into the argumentValue field, regardless
 *              of whether or not the argument is passed into the program
 *
 * helpText - This is used to create an automatic usage message, which may be
 *            shown by calling Gargamel::ShowUsage()
 *
 */

#pragma once
#include <string>
#include <iostream>
#include <vector>

#define GGM_DESCRIBE_ARG(id, shortName, longName, style, helpText) \
{ static_cast<int>(id), shortName, longName,                       \
	Gargamel::ArgumentStyle::style, false, "", helpText },
#define GGM_DESCRIBE_ARG_DEFAULT(id, shortName, longName, style,   \
	defaultVal, helpText )                                     \
{ static_cast<int>(id), shortName, longName,                       \
	Gargamel::ArgumentStyle::style, false, defaultVal, helpText },
#define GGM_DESCRIBE_ARG_ARRAY(id, longName, helpText)             \
{ static_cast<int>(id), '\0', longName,                            \
 Gargamel::ArgumentStyle::RequiredArgument, true, "", helpText },

namespace Gargamel {
	enum ArgumentStyle {
		NoArgument,
		OptionalArgument,
		RequiredArgument,
	};

	struct ArgumentDescription {
	public:
		int const id;
		char const shortOptionName;
		std::string const longOptionName;
		ArgumentStyle const argumentStyle;
		bool const isArgumentArray;
		std::string defaultValue;
		std::string const helpText;
	};
	typedef std::vector<ArgumentDescription> ArgumentList;

	struct ArgumentValue {
	public:
		std::string argumentValue;
		std::vector<std::string> argumentArray;
		bool isArgumentPresent;

		float floatValue() {
			return std::stof(argumentValue);
		}

		float floatValue(int index) {
			return std::stof(argumentArray[index]);
		}

		float intValue() {
			return std::stoi(argumentValue);
		}

		float intValue(int index) {
			return std::stoi(argumentArray[index]);
		}
	};

	void SetArguments(ArgumentList const& argumentList, int PositionalArguments);
	bool Process(int argc, char* argv[]);
	bool ProcessLongArgument(int& cur, int argc, char* argv[] );
	bool ProcessFlagList(char const* flags);
	void ShowUsage();

	static ArgumentList const* Arguments;
	static std::vector<ArgumentValue> ArgumentValues;
	static int PositionalArguments;

	void SetArguments(
		ArgumentList const& argumentList,
		int PositionalArguments
	) {
		Gargamel::PositionalArguments = PositionalArguments;
		Arguments = &argumentList;
		ArgumentValues.clear();
		for(auto& arg : *Arguments) {
			if(static_cast<int>(arg.id) >= ArgumentValues.size())
				ArgumentValues.resize(static_cast<int>(arg.id) + 1);
			ArgumentValues[arg.id].argumentValue = arg.defaultValue;
		}
	}

	bool Process(int argc, char* argv[]) {
		bool badCommandLine = false;
		for(int i = 1; i < argc - PositionalArguments; ++i) {
			if(argv[i][0] != '-') {
				badCommandLine = true;
				continue;
			}
			if(argv[i][1] != '\0') {
				if(argv[i][1] != '-')
					badCommandLine |= ProcessFlagList(argv[i] + 1);
				else
					badCommandLine |= ProcessLongArgument(i, argc, argv);
			} else
				badCommandLine = true;
		}
		return badCommandLine;
	}

	bool ProcessLongArgument(int& cur, int argc, char* argv[] ) {
		for(auto& arg : *Arguments) {
			if(arg.longOptionName.length() == 0)
				continue;
			if(arg.longOptionName.compare(argv[cur] + 2) == 0) {
				ArgumentValues[arg.id].isArgumentPresent = true;
				switch(arg.argumentStyle) {
				case ArgumentStyle::NoArgument:
					break;
				case ArgumentStyle::OptionalArgument:
				case ArgumentStyle::RequiredArgument:
					if(cur + 1 < argc
						&& argv[cur + 1][0] != '-') {
						++cur;
						if(arg.isArgumentArray)
							ArgumentValues[arg.id].argumentArray.push_back(argv[cur]);
						else
							ArgumentValues[arg.id].argumentValue = argv[cur];
					} else if(arg.argumentStyle == ArgumentStyle::RequiredArgument)
						return false;
				}
			}
		}
		return true;
	}

	bool ProcessFlagList(char const* flags) {
		bool flagNotUnderstood = false;
		for(; *flags != '\0'; ++flags) {
			bool FlagUsed = false;
			for(auto& arg : *Arguments)
				if(*flags == arg.shortOptionName) {
					ArgumentValues[arg.id].isArgumentPresent = true;
					FlagUsed = true;
				}
			flagNotUnderstood |= FlagUsed == false;
		}
		return flagNotUnderstood;
	}

	void ShowUsage() {
		using std::cout;
		using std::endl;

		for(auto& arg : *Arguments) {
			bool shouldTabPrecede = false;
			if(arg.shortOptionName != '\0') {
				cout << "-" << arg.shortOptionName;
				if(arg.longOptionName.length() != 0)
					cout << ", ";
				shouldTabPrecede = true;
			}
			if(arg.longOptionName.length() != 0) {
				cout << "--" << arg.longOptionName;
				switch(arg.argumentStyle) {
				case ArgumentStyle::OptionalArgument:
					cout << " [Argument],";
					break;
				case ArgumentStyle::RequiredArgument:
					cout << " Argument,";
				case ArgumentStyle::NoArgument:
				default:
					break;
				}
				shouldTabPrecede = true;
			}
			if(shouldTabPrecede)
				cout << "\n\t";
			cout << arg.helpText << endl;
		}
	}
}
