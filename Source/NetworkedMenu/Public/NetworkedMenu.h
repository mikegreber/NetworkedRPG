#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(NetworkedMenu, All, All);

class FNetworkedMenu : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	
	virtual void ShutdownModule() override;
};
