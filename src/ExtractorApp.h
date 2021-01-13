/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2021 MaNGOS <https://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */


#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/AutoPtr.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "client/Extractor.h"
#include "client/Version.h"
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include "Poco/WindowsConsoleChannel.h"
#endif

#ifdef __linux__
#include "Poco/ConsoleChannel.h"
#endif

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::OptionCallback;
using Poco::Message;
using Poco::Logger;
using Poco::AutoPtr;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
#ifdef _WIN32
using Poco::WindowsConsoleChannel;
#endif
#ifdef __linux__
using Poco::ConsoleChannel;
#endif


class ExtractorApp: public Application
{
public:
	ExtractorApp(): _helpRequested(false), _extractMaps(true), _extractDbcs(true), _flatMap(true)
	{
		setUnixOptions(true);
		FormattingChannel* pFCConsole = new FormattingChannel(new PatternFormatter("%Y-%m-%d %H:%M:%S.%c [%p][%s] - %t"));
#ifdef _WIN32
		pFCConsole->setChannel(new WindowsConsoleChannel);
#endif
#ifdef __linux__
		pFCConsole->setChannel(new ConsoleChannel);
#endif
		pFCConsole->open();
		_logger.setChannel(pFCConsole);
		_logger.setLevel(Message::PRIO_INFORMATION);
		_clientPath = ".";
		_outputPath = ".";
	}

	~ExtractorApp()
	{
		delete _extractor;
	}

protected:
	void initialize(Application& self);

	void uninitialize();

	void reinitialize(Application& self);

	void defineOptions(OptionSet& options);

	void handleHelp(const std::string& name, const std::string& value);

	void handleExtractMap(const std::string& name, const std::string& value);
	
	void handleExtractDbc(const std::string& name, const std::string& value);

	void handleFlatMap(const std::string& name, const std::string& value);

	void handleClientPath(const std::string& name, const std::string& value);

	void handleOutputPath(const std::string& name, const std::string& value);

	void displayHelp();

	int main(const ArgVec& args);

	void printWebsiteBanner();

	void printSummary();

	void detectBuild();

private:
	Logger& _logger = Logger::get("Extractor");
	bool _helpRequested;
	bool _extractMaps;
	bool _extractDbcs;
	std::string _clientPath;
	std::string _outputPath;
	bool _flatMap;
	Version _version;
	Extractor* _extractor;
};