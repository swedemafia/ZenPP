#ifndef BASECLASSES_COMMANDBASE_H
#define BASECLASSES_COMMANDBASE_H

#ifdef _MSC_VER
#pragma once
#endif

class CommandBase : public StoreBuffer
{
public:
	explicit CommandBase(_In_ CONST UCHAR Command);

	~CommandBase() = default;
};

#endif
