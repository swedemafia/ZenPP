#ifndef UTILITIES_SEMANTICVERSION_H
#define UTILITIES_SEMANTICVERSION_H

#ifdef _MSC_VER
#pragma once
#endif

class SemanticVersion
{
public:
	// Constructor for initialization
	explicit SemanticVersion(_In_ CONST std::string VersionString);

	// Public methods for semantic version information retrieval
	CONST CHAR GetBuild(VOID) CONST;
	CONST CHAR GetMajor(VOID) CONST;
	CONST CHAR GetMinor(VOID) CONST;
	CONST CHAR GetRevision(VOID) CONST;
	CONST BOOLEAN IsBeta(VOID) CONST;

	// Public method for processing a version string
	VOID ProcessVersionString(_In_ CONST std::string VersionString);

private:
	// Private members for semantic version information
	BOOLEAN m_Beta = FALSE;
	CHAR m_Major = 0, m_Minor = 0, m_Revision = 0, m_Build = 0;
};

#endif