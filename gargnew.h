#pragma once
#include <string>
#include <iostream>
#include <vector>

#define GGM_DESCRIBE_ARG(id, shortName, longName, style, helpText) \
{ id, shortName, longName, Gargamel::ArgumentStyle::style, false, "", helpText }
#define GGM_DESCRIBE_ARG_DEFAULT(id, shortName, longName, style, defaultVal, helpText ) \
{ id, shortName, longName, Gargamel::ArgumentStyle::style, false, defaultVal, helpText }
#define GGM_DESCRIBE_ARG_ARRAY(id, longName, helpText)
{ id, '\0', longName, Gargamel::ArgumentStyle::RequiredArgument, true "", helpText}

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
			float ret = 0.f;
			std::stof(argumentValue, &ret);
			return ret;
		}

		float floatValue(int index) {
			float ret = 0.f;
			std::stof(argumentArray[index], &ret);
			return ret;
		}

		float intValue() {
			int ret = 0;
			std::stoi(argumentValue, &ret);
			return ret;
		}

		float intValue(int index) {
			int ret = 0;
			std::stoi(argumentArray[index], &ret);
			return ret;
		}
	};

	bool Process(int argc, char* argv[]);
	bool ProcessLongArgument(int& cur, int argc, char* argv[] );
	bool ProcessFlagList(char const* flags);
	void ShowUsage();

	static ArgumentList const* Arguments;
	static std::vector<ArgumentValue> ArgumentValues;
	static int PositionalArguments;

	bool Process(int argc, char* argv[]) {
		return false;
	}

	bool ProcessLongArgument(int& cur, int argc, char* argv[] ) {

	}

	bool ProcessFlagList(char const* flags) {
		return false;
	}

	bool SetArguments(ArgumentList const& argumentList, int PositionalArguments) {
		Gargamel::PositionalArguments = PositionalArguments;
		Arguments = &argumentList;
		ArgumentValues.clear();
		for(auto& arg : argumentList) {
			if(static_cast<int>(arg.id) >= ArgumentValues.size())
				ArgumentValues.resize(static_cast<int>(arg.id) + 1);
			ArgumentValues.argumentValue = arg.defaultValue;
		}
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
			if(arg.longOptionName.length != 0) {
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
