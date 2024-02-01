#include "Precomp.h"

CommandBase::CommandBase(CONST UCHAR Command)
{
	InsertByte(0);
	InsertByte(Command);
}