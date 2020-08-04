#include "NetworkedMenu.h"

DEFINE_LOG_CATEGORY(NetworkedMenu);

#define LOCTEXT_NAMESPACE "FNetworkedMenu"

void FNetworkedMenu::StartupModule()
{
	UE_LOG(NetworkedMenu, Warning, TEXT("NetworkedMenu module has started!"));
}

void FNetworkedMenu::ShutdownModule()
{
	UE_LOG(NetworkedMenu, Warning, TEXT("NetworkedMenu module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNetworkedMenu, NetworkedMenu)