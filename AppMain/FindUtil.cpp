#include "StdAfx.h"
#include "VPNSubsystem.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionCallback.h"

using namespace Poco::Util;
using Poco::Logger;

namespace VPN {

	CVPNSubsystem::CVPNSubsystem() :
		m_pLogger(&Logger::get(name())),
		m_spConf(new XMLConfiguration)
	{

	}

	CVPNSubsystem::~CVPNSubsystem()
	{

	} 

	const char* CVPNSubsystem::name() const
	{
		return "CVPNSubsystem";
	}

	void CVPNSubsystem::initialize(Poco::Util::Application& app)
	{
		m_xmlFileConfigPath = (app.config().getString("VPNFileConfigPath", ""));
		if (!m_xmlFileConfigPath.empty())
		{
			m_spConf = new Poco::Util::XMLConfiguration(m_xmlFileConfigPath);
		}
	}

	void CVPNSubsystem::uninitialize()
	{
		if (!m_xmlFileConfigPath.empty())
		{
			m_spConf->save(m_xmlFileConfigPath);
		}
	}

	void CVPNSubsystem::reinitialize(Poco::Util::Application& app)
	{
		
	}

	void CVPNSubsystem::defineOptions(Poco::Util::OptionSet& options)
	{
		options.addOption(
			Option("include-dir", "I", "specify an include search path")
			.required(false)
			.repeatable(true)
			.argument("path")
			.callback(OptionCallback<CVPNSubsystem>(this, &CVPNSubsystem::handleTest)));
	}

	void CVPNSubsystem::handleTest(const std::string& name, const std::string& value)
	{
		poco_trace_f2(logger(), "name[%s] value[%s]", name, value);
	}

	Poco::Util::XMLConfiguration& CVPNSubsystem::config() const
	{
		return *const_cast<Poco::Util::XMLConfiguration*>(m_spConf.get());
	}

} // VPN
