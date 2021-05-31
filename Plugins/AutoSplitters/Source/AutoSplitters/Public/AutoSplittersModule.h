#pragma once

#include "Buildables/FGBuildableFactory.h"

// define to 1 to get more debug output to console when the debug flag is set in the splitter UI
#define AUTO_SPLITTERS_DEBUG 0

class FAutoSplittersModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
};
