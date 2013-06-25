/*
 * Gargamel - Gargamel ARGument AMaLgamator
 * This file is intended to simplify processing command-line arguments
 * Author: Jameson Thatcher
 */

/*
 * TODO
 * add a license
 */

/*
 * Usage:
 *	After including this file, create a list of arguments with the START_ARGS,
 *	DESCRIBE_ARG, and END_ARGS macros (listed shortly after this comment)
 *
 * Example code:
 *
 * #include <gargamel.h>
 * enum ArgNames {
 *		Unknown,
 *		Help,
 * };
 * START_ARGS(Args)
 *		DESCRIBE_ARG(Help, 'h', "help", NoArg, "\tPrint help text")
 * END_ARGS
 *
 * int main(int argc, char* argv[]) {
 *		Gargamel::Process(Args, argc, argv);
 *		if( Gargamel::ArgumentSet[Help].isArgumentPresent )
 *			Gargamel::ShowUsage();
 * }
 *
 * Notes:
 *	The arguments can also take arguments (long form only), which will be taken
 *	as the next argument in the programs argument list. If a default value for
 *	an argument is required, use the DESCRIBE_ARG_DEFAULT macro, which allows
 *	you to specify a default string, as the argument after the argument style
 *
 *	Use dummy entries to help formatting help text, if nicer usage instructions
 *	are desired
 *
 *	If the argument type conversions, and the usage helper aren't needed,
 *	define GARGAMEL_LEAN_AND_MEAN before including this file, and they won't be
 *	compiled in.
 */

//if you don't support pragma once, I don't want to be friends with you anyway
#pragma once
#include <string.h>
#include <stdio.h>

#define START_ARGS(name) Gargamel::ArgumentDescription name[] = {

#ifndef GARGAMEL_LEAN_AND_MEAN
#	define DESCRIBE_ARG(id, shortName, longName, style, helpText) \
	{ id, shortName, longName, Gargamel::ArgStyle::style, 0, false, false, helpText },
#	define DESCRIBE_ARG_DEFAULT(id, shortName, longName, style, defaultValue, false, helpText) \
	{ id, shortName, longName, Gargamel::ArgStyle::style, defaultValue, false, false, helpText },
#	define DESCRIBE_ARG_ARRAY(id, longName, helpText) \
	{ id, 0, longName, Gargamel::ArgStyle::RequiredArg, new std::vector<char const*>(), false, true, helpText },
#else
#	define DESCRIBE_ARG(id, shortName, longName, style) \
	{ id, shortName, longName, Gargamel::ArgStyle::style, 0, false, false },
#	define DESCRIBE_ARG_DEFAULT(id, shortName, longName, style, defaultValue) \
	{ id, shortName, longName, Gargamel::ArgStyle::style, defaultValue, false, false },
#	define DESCRIBE_ARG_ARRAY(id, longName) \
	{ id, 0, longName, Gargamel::ArgStyle::RequiredArg, new std::vector<char const*>(), false, true },
#endif

#define END_ARGS {0, 0, 0, Gargamel::ArgStyle::NoArg, 0, false, false} };

//We can ignore the warning about _s versions of functions because the strings
// being used should either be literals, or arguments, which should be
// guaranteed to be safe. If the library is used outside recommendations here,
// all behaviour is undefined. All of it.
#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning( disable: 4996 )
#endif

namespace Gargamel {

	namespace ArgStyle {
		enum Type {
			NoArg,
			OptionalArg,
			RequiredArg
		};
	}
	typedef ArgStyle::Type EArgStyle;

	struct ArgumentDescription {
	public:
		int				id;
		char			shortOptName;
		char const*		longOptName;
		EArgStyle		argumentStyle;
		union {
			std::vector<char const*>* argumentArray;
			char const*		  argumentValue;
		};
		bool			isArgumentPresent;
		bool			isArgumentArray;

#ifndef GARGAMEL_LEAN_AND_MEAN
		char const*		helpText;

		float floatVal() {
			float Ret = 0.f;
			if( !isArgumentArray )
				sscanf(argumentValue, "%f", &Ret);
			return Ret;
		}

		float floatVal(int idx) {
			float Ret = 0.f;
			if( isArgumentArray )
				sscanf(argumentArray->at(idx), "%f", &Ret);
			return Ret;
		}

		int intVal() {
			int Ret = 0;
			if( !isArgumentArray )
				sscanf(argumentValue, "%d", &Ret);
			return Ret;
		}

		int intVal(int idx) {
			int Ret = 0;
			if( isArgumentArray )
				sscanf(argumentArray->at(idx), "%d", &Ret);
			return Ret;
		}
#endif

		/************************************************************
		   Anything below this point need not be read in order to
		  understand how to use this header, only for how it works!
		************************************************************/

		bool IsEmpty() const {
			return id == 0
				&& shortOptName == 0
				&& longOptName == 0
				&& argumentStyle == 0
				&& isArgumentArray == false
				&& argumentValue == 0
#ifndef GARGAMEL_LEAN_AND_MEAN
				&& helpText == 0
#endif
				;
		}
	};

	bool SetArguments(ArgumentDescription const * descs);
	bool ProcessLongArgument(int& cur, int argc, char* argv[]);
	bool ProcessFlagList(char const * flags);

	ArgumentDescription* ArgumentSet;
#ifndef GARGAMEL_LEAN_AND_MEAN
	ArgumentDescription const * Original;
#endif

	int NumArgs;

	bool Process(ArgumentDescription const * descs, int argc, char* argv[]) {
		if( !SetArguments(descs) )
			return true;

#ifndef GARGAMEL_LEAN_AND_MEAN
		Original = descs;
#endif

		bool badCommandLine = false;
		for(int i = 1; i < argc ; ++i ) {
			if( argv[i][0] != '-' ) {
				badCommandLine = true;
				continue;
			}

			if( argv[i][1] != '\0' ) {
				if( argv[i][1] != '-' )
					badCommandLine |= ProcessFlagList(argv[i] + 1);
				else
					ProcessLongArgument(i, argc, argv);
			}
		}
		return badCommandLine;
	}

	bool ProcessLongArgument(int& cur, int argc, char* argv[]) {
		if( argv[cur][0] != '-' || argv[cur][1] != '-' )
			return false;
		for( int i = 0; i < NumArgs; ++i ) {
			if( ArgumentSet[i].longOptName == NULL )
				continue;
			if( strcmp(argv[cur] + 2, ArgumentSet[i].longOptName ) == 0 ) {
				ArgumentSet[i].isArgumentPresent = true;
				switch( ArgumentSet[i].argumentStyle ) {
				case ArgStyle::NoArg:
					break;
				case ArgStyle::OptionalArg:
					if( cur + 1 < argc
							&& argv[cur+1][0] != '-' ) {
						if( ArgumentSet[i].isArgumentArray )
							ArgumentSet[i].argumentArray
								->push_back(argv[cur+1]);
						else
							ArgumentSet[i].argumentValue
								= argv[cur+1];
						++cur;
					}
					break;
				case ArgStyle::RequiredArg:
					if( cur + 1 < argc ) {
						++cur;
						if( ArgumentSet[i].isArgumentArray )
							ArgumentSet[i].argumentArray
								->push_back(argv[cur]);
						else
							ArgumentSet[i].argumentValue
								= argv[cur];
						return true;
					}
					else
						return false;
					break;
				default:
					//oops?
					break;
				}
			}
		}
		return true;
	}

	bool ProcessFlagList(char const * flags) {
		bool FlagNotUnderstood = false;
		for(; *flags != '\0'; ++flags) {
			bool FlagUsed = false;
			for( int i = 0; i < NumArgs; ++i )
				if( *flags == ArgumentSet[i].shortOptName ) {
					ArgumentSet[i].isArgumentPresent = true;
					FlagUsed = true;
				}
			if( !FlagUsed )
				FlagNotUnderstood = true;
		}
		return !FlagNotUnderstood;
	}

	bool SetArguments(ArgumentDescription const * descs) {
		ArgumentDescription Empty = {0,0,0,ArgStyle::NoArg,0};

		//need to find the max descs value
		int maxIdx = -1;
		int CurIdx = -1;
		while( !descs[++CurIdx].IsEmpty() ) {
			if( descs[CurIdx].id > maxIdx ) {
				maxIdx = descs[CurIdx].id;
			}
		}

		NumArgs = maxIdx+1;
		if( NumArgs <= 0 )
			return false;
		ArgumentSet = new ArgumentDescription[NumArgs];
		for( int i = 0; i < NumArgs; ++i )
			ArgumentSet[i] = Empty;

		CurIdx = -1;
		while( !descs[++CurIdx].IsEmpty() ) {
			ArgumentSet[descs[CurIdx].id] = descs[CurIdx];
		}
		return true;
	}

#ifndef GARGAMEL_LEAN_AND_MEAN
	void ShowUsage() {
		int CurIdx = -1;
		while( !Original[++CurIdx].IsEmpty() ) {
			if( Original[CurIdx].shortOptName != '\0' ) {
				printf("-%c", Original[CurIdx].shortOptName);
				if( Original[CurIdx].longOptName != NULL )
					printf(", ");
			}
			if( Original[CurIdx].longOptName != NULL )
			{
				printf("--%s", Original[CurIdx].longOptName );
				switch( Original[CurIdx].argumentStyle )
				{
				case ArgStyle::OptionalArg:
					printf(" [Argument]\n");
					break;
				case ArgStyle::RequiredArg:
					printf(" Argument\n");
					break;
				case ArgStyle::NoArg:
				default:
					break;
				}
			}
			printf("%s\n", Original[CurIdx].helpText);
		}
	}
#endif
}

#ifdef _MSC_VER
#	pragma warning(pop)
#endif

