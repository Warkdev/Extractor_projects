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


#include "Poco/Logger.h"

#include "ExtractorApp.h"
#include "Poco/Util/IntValidator.h"
#include "client/ClientHelper.h"
#include "client/classic/ExtractorClassic.h"
#include "client/burningcrusade/ExtractorBurningCrusade.h"
#include "client/wrathlichking/ExtractorWrathLichKing.h"
#include "client/cataclysm/ExctractorCataclysm.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::Logger;

void ExtractorApp::initialize(Application& self)
{
	int loaded = loadConfiguration(); // load default configuration files, if present
	Application::initialize(self);
	// add your own initialization code here
}

void ExtractorApp::uninitialize()
{
	// add your own uninitialization code here
	Application::uninitialize();
}

void ExtractorApp::reinitialize(Application& self)
{
	Application::reinitialize(self);
	// add your own reinitialization code here
}

void ExtractorApp::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleHelp)));

	options.addOption(
		Option("extract-map", "e", "Extract Maps from the game archives")
			.required(false)
			.repeatable(false)
			.argument("<0|1>")
			.validator(new IntValidator(0, 1))
			.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleExtractMap)));

	options.addOption(
		Option("extract-dbc", "d", "Extract DBCs from the game archives")
			.required(false)
			.repeatable(false)
			.argument("<0|1>")
			.validator(new IntValidator(0, 1))
			.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleExtractDbc)));

	options.addOption(
		Option("generate-vmap", "v", "Generate Vmaps from the game archives")
		.required(false)
		.repeatable(false)
		.argument("<0|1>")
		.validator(new IntValidator(0, 1))
		.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleGenerateVmap)));

	options.addOption(
		Option("generate-mmap", "m", "Generate Mmaps from the game archives")
		.required(false)
		.repeatable(false)
		.argument("<0|1>")
		.validator(new IntValidator(0, 1))
		.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleGenerateMmap)));

	options.addOption(
		Option("client-path", "c", "Indicates the game folder location")
			.required(false)
			.repeatable(false)
			.argument("path")
			.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleClientPath)));

	options.addOption(
		Option("output-path", "o", "Indicates the folder into which the data needs to be extracted")
			.required(false)
			.repeatable(false)
			.argument("path")
			.callback(OptionCallback<ExtractorApp>(this, &ExtractorApp::handleOutputPath)));
}

void ExtractorApp::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void ExtractorApp::handleExtractMap(const std::string& name, const std::string& value)
{
	if (!std::stoi(value)) 
	{
		_extractMaps = false;
	}
	config().setBool(FLAG_EXPORT_MAPS, _extractMaps);
}

void ExtractorApp::handleExtractDbc(const std::string& name, const std::string& value)
{
	if (!std::stoi(value))
	{
		_extractDbcs = false;
	}
	config().setBool(FLAG_EXTRACT_DBCS, _extractDbcs);
}

void ExtractorApp::handleClientPath(const std::string& name, const std::string& value) 
{
	if (value.c_str()) 
	{
		_clientPath = value;
	}
	config().setString(FLAG_CLIENT_PATH, _clientPath);
}

void ExtractorApp::handleOutputPath(const std::string& name, const std::string& value)
{
	if (value.c_str())
	{
		_outputPath = value;
	}
	config().setString(FLAG_EXPORT_PATH, _outputPath);
}

void ExtractorApp::handleGenerateVmap(const std::string& name, const std::string& value)
{
	if (!std::stoi(value))
	{
		_generateVmaps = false;
	}
	config().setBool(FLAG_EXPORT_VMAPS, _generateVmaps);
}

void ExtractorApp::handleGenerateMmap(const std::string& name, const std::string& value)
{
	if (!std::stoi(value))
	{
		_generateMmaps = false;
	}
	config().setBool(FLAG_EXPORT_MMAPS, _generateMmaps);
}

void ExtractorApp::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("Extractor helps you to extract game data (Maps, DBCs, VMaps, MMaps) from MPQ archives.");
	helpFormatter.setUnixStyle(true);
	helpFormatter.format(std::cout);
}

int ExtractorApp::main(const ArgVec& args)
{
	if (!_helpRequested)
	{
		printWebsiteBanner();
		printSummary();
		detectBuild();
		extractData();
	}
	return Application::EXIT_OK;
}

void ExtractorApp::printWebsiteBanner()
{
	logger().information("");
	logger().information("        __  __      _  _  ___  ___  ___      ");
	logger().information("       |  \\/  |__ _| \\| |/ __|/ _ \\/ __|  ");
	logger().information("       | |\\/| / _` | .` | (_ | (_) \\__ \\  ");
	logger().information("       |_|  |_\\__,_|_|\\_|\\___|\\___/|___/ ");
	logger().information("");
	logger().information("  ________________________________________________");
	logger().information("    For help and support please visit:            ");
	logger().information("    Website / Forum / Wiki: https://getmangos.eu  ");
	logger().information("  ________________________________________________");
}

void ExtractorApp::printSummary()
{
	logger().information("------ Extractor Options ------");
	logger().information("----- General settings -----");
	logger().information("Client Path: %s", _clientPath);
	logger().information("Output Path: %s", _outputPath);
	logger().information("----- Map extractor arguments -----");
	logger().information("Extract Maps: %s", std::string(_extractMaps ? "true" : "false"));
	logger().information("Extract DBCs: %s", std::string(_extractDbcs ? "true" : "false"));
	logger().information("----- VMap generator arguments ----");
	logger().information("Generate VMaps: %s", std::string(_generateVmaps ? "true" : "false"));
	logger().information("----- MMap generator arguments ----");
	logger().information("Generate MMaps: %s", std::string(_generateMmaps ? "true" : "false"));
	logger().information("");
	logger().information("");

}

void ExtractorApp::detectBuild()
{
	_version = ClientHelper::getBuildVersion(_clientPath);
	logger().information("Detected Client Build: %s", ClientHelper::getVersionName(_version));
	// Handling versions
	switch (_version)
	{
		case Version::CLIENT_UNKNOWN:
		case Version::CLIENT_MOP:
		case Version::CLIENT_WOD:
		case Version::CLIENT_LEGION:
		case Version::CLIENT_BFA:
		case Version::CLIENT_SHADOWLANDS:
			logger().error("This build is not supported by this extractor");
		case Version::CLIENT_CLASSIC:
			_extractor = new ExtractorClassic(&config());
			break;
		case Version::CLIENT_TBC:
			//_extractor = new ExtractorBurningCrusade(&config());
			break;
		case Version::CLIENT_WOTLK:
			//_extractor = new ExtractorWrathLichKing(&config());
			break;
		case Version::CLIENT_CATA:
			//_extractor = new ExtractorCataclysm(&config());
			break;
	}
}

void ExtractorApp::extractData()
{
	_extractor->init();
	_extractor->loadMPQs();
	if (_extractDbcs)
	{
		_extractor->exportDBC();
	}
	if (_extractMaps || _generateVmaps || _generateMmaps)
	{
		_extractor->exportMaps();
	}
}