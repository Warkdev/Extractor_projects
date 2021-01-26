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

#include "client/Extractor.h"
#include "client/Version.h"
#include <iostream>
#include <sstream>
#include <string>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::OptionCallback;


class ExtractorApp: public Application
{
public:
	ExtractorApp(): _helpRequested(false), _extractMaps(true), _extractDbcs(true), _generateVmaps(true), _generateMmaps(true)
	{
		setUnixOptions(true);
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

	void handleGenerateVmap(const std::string& name, const std::string& value);

	void handleGenerateMmap(const std::string& name, const std::string& value);

	void handleClientPath(const std::string& name, const std::string& value);

	void handleOutputPath(const std::string& name, const std::string& value);

	void displayHelp();

	int main(const ArgVec& args);

	void printWebsiteBanner();

	void printSummary();

	void detectBuild();

	void extractData();

private:
	bool _helpRequested;
	bool _extractMaps;
	bool _extractDbcs;
	bool _generateVmaps;
	bool _generateMmaps;
	std::string _clientPath;
	std::string _outputPath;
	Version _version;
	Extractor* _extractor;
};