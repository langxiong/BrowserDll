/******************************************************************************
*  版权所有（C）2010-2015，上海二三四五网络科技有限公司                      *
*  保留所有权利。                                                            *
******************************************************************************
*  作者 : 熊浪
*  版本 : 1.0
*****************************************************************************/
/*  修改记录:
日期       版本    修改人             修改内容
--------------------------------------------------------------------------
******************************************************************************/
#ifndef VPNSubsystem_h__
#define VPNSubsystem_h__ 1

#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/Util/XMLConfiguration.h"

namespace Poco
{
	namespace Util
	{
		class XMLConfiguration;
	}
}

namespace VPN {
	
	class CPeerGraph;
	class MYVPN_API CVPNSubsystem :
		public	Poco::Util::Subsystem
	{
	public:
		CVPNSubsystem();

		virtual ~CVPNSubsystem();

	public:

		Poco::Logger& logger() const;

		Poco::Util::XMLConfiguration& config() const;

		/** Subsystem interface 
		*/
	protected:
		virtual const char* name() const;
		/// Returns the name of the subsystem.
		/// Must be implemented by subclasses.

		virtual void initialize(Poco::Util::Application& app);
		/// Initializes the subsystem.

		virtual void uninitialize();
		/// Uninitializes the subsystem.

		virtual void reinitialize(Poco::Util::Application& app);
		/// Re-initializes the subsystem.
		///
		/// The default implementation just calls
		/// uninitialize() followed by initialize().
		/// Actual implementations might want to use a
		/// less radical and possibly more performant 
		/// approach.

		virtual void defineOptions(Poco::Util::OptionSet& options);
		/// Called before the Application's command line processing begins.
		/// If a subsystem wants to support command line arguments,
		/// it must override this method.
		/// The default implementation does not define any options.
		///
		/// To effectively handle options, a subsystem should either bind
		/// the option to a configuration property or specify a callback
		/// to handle the option.

	private:

		void handleTest(const std::string& name, const std::string& value);

	private:

		Poco::Logger* m_pLogger;

		Poco::AutoPtr<Poco::Util::XMLConfiguration> m_spConf;

		std::string m_xmlFileConfigPath;
	};

	inline Poco::Logger& CVPNSubsystem::logger() const
	{
		poco_check_ptr(m_pLogger);
		return *m_pLogger;
	}

} // VPN
#endif // VPNSubsystem_h__