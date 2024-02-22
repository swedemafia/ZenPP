#include "Precomp.h"

SemanticVersion::SemanticVersion(CONST std::string VersionString)
{
	ProcessVersionString(VersionString);
}

CONST CHAR SemanticVersion::GetBuild(VOID) CONST
{
	return m_Build;
}

CONST CHAR SemanticVersion::GetMajor(VOID) CONST
{
	return m_Major;
}

CONST CHAR SemanticVersion::GetMinor(VOID) CONST
{
	return m_Minor;
}

CONST CHAR SemanticVersion::GetRevision(VOID) CONST
{
	return m_Revision;
}

CONST BOOLEAN SemanticVersion::IsBeta(VOID) CONST
{
	return m_Beta;
}

VOID SemanticVersion::ProcessVersionString(_In_ CONST std::string VersionString)
{
	std::unique_ptr<std::regex> Pattern = std::make_unique<std::regex>(R"((\d+)\.(\d+)\.(\d+)(?:-beta\.(\d+))?)");
	std::smatch Matches;

	// Reset current version information
	m_Major = m_Minor = m_Revision = m_Beta = m_Build = 0;

	try
	{
		// Check for a regex match of the version string against the desired pattern
		if (std::regex_match(VersionString, Matches, *Pattern) && Matches[1].matched && Matches[2].matched && Matches[3].matched) {
			// Set version information
			m_Major = std::stoi(Matches[1]);
			m_Minor = std::stoi(Matches[2]);
			m_Revision = std::stoi(Matches[3]);

			// Check for Cronus Zen beta firmware
			if (Matches[4].matched && (m_Beta = (Matches.size() == 5))) {
				m_Build = std::stoi(Matches[4]);
			}
		}
	}
	catch (CONST std::regex_error& Error) {
		std::string AnsiError(Error.what());
		std::wstring UnicodeError(AnsiError.begin(), AnsiError.end());
		App->DisplayError(UnicodeError);
	}
}