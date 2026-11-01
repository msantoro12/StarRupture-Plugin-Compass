#pragma once

#include "plugin_interface.h"
#include "Engine_classes.hpp"
#include "Chimera_classes.hpp"
#include "Chimera_structs.hpp"
#include "MassAIPrototypeEnemyRuntime_classes.hpp"
#include "plugin_map.h"
#include "aob_patterns.h"

#include <iterator>

// Every member of a game class the compass reads, taken from the loader's map
// for the running game build (see plugin_map.h). Resolved once in
// OnPluginLoadHooks; read on the game thread with In(object), which is null
// when the object is null or the member did not resolve. Engine members
// (UWorld::GameState, UObject::Outer, the texture pinning flags) are not game
// classes and stay compiled in.

#define COMPASS_FIELD(Class, Member) SR_FIELD(decltype(SDK::Class::Member), Class, Member, 0)

namespace CompassFields
{
	inline auto BaseCoreUpgradeLevel = COMPASS_FIELD(ACrBaseCore, UpgradeLevel);
	inline auto BaseCoreIsAttacked   = COMPASS_FIELD(ACrBaseCore, bIsAttacked);
	inline auto BaseCoreIsInfected   = COMPASS_FIELD(ACrBaseCore, IsInfected);
	inline auto EnemyDissolve        = COMPASS_FIELD(AMassEnemyCharacterBase, Dissolve);

	inline auto MapMenuHelper         = COMPASS_FIELD(ACrGameStateBase, MapMenuDataReplicationHelper);
	inline auto PlayerPersonalMarkers = COMPASS_FIELD(ACrGameStateBase, PlayerPersonalMarkers);

	inline auto BuildingsContainer = COMPASS_FIELD(ACrMapMenuDataReplicationHelper, BuildingsMarkerDataContainer);
	inline auto FoundableContainer = COMPASS_FIELD(ACrMapMenuDataReplicationHelper, FoundableMarkerDataContainer);
	inline auto PlayersContainer   = COMPASS_FIELD(ACrMapMenuDataReplicationHelper, PlayersMarkerDataContainer);
	inline auto BuildingsData      = COMPASS_FIELD(FBuildingsMarkerDataContainer, BuildingsMarkerData);
	inline auto FoundableData      = COMPASS_FIELD(FFoundableMarkerDataContainer, FoundableMarkerData);
	inline auto PlayersData        = COMPASS_FIELD(FPlayersMarkerDataContainer, PlayersMarkerData);

	inline auto BuildingLocation   = COMPASS_FIELD(FBuildingMarkerDataFastArrayItem, Location);
	inline auto BuildingUniqueName = COMPASS_FIELD(FBuildingMarkerDataFastArrayItem, BuildingUniqueName);
	inline auto FoundableLocation  = COMPASS_FIELD(FFoundableMarkerDataFastArrayItem, Location);
	inline auto FoundableType      = COMPASS_FIELD(FFoundableMarkerDataFastArrayItem, FoundableType);
	inline auto PlayerName         = COMPASS_FIELD(FPlayerMarkerDataFastArrayItem, Player);
	inline auto PlayerLocation     = COMPASS_FIELD(FPlayerMarkerDataFastArrayItem, Location);
	inline auto PlayerColor        = COMPASS_FIELD(FPlayerMarkerDataFastArrayItem, PlayerColor);
	inline auto PlayerRotationZ    = COMPASS_FIELD(FPlayerMarkerDataFastArrayItem, RotationZ);
	inline auto PlayerFlags        = COMPASS_FIELD(FPlayerMarkerDataFastArrayItem, PlayerMarkerFlags);
	inline auto PinPosition        = COMPASS_FIELD(FCrPlayerPersonalMarkerData, MarkerPosition);
	inline auto PinPlayerName      = COMPASS_FIELD(FCrPlayerPersonalMarkerData, PlayerName);
	inline auto PinColor           = COMPASS_FIELD(FCrPlayerPersonalMarkerData, PlayerColor);

	// The marker arrays are walked with the compiled element size, so each
	// element struct is checked against the running build.
	inline auto BuildingItemSize  = SR_SIZE_CHECK(FBuildingMarkerDataFastArrayItem);
	inline auto FoundableItemSize = SR_SIZE_CHECK(FFoundableMarkerDataFastArrayItem);
	inline auto PlayerItemSize    = SR_SIZE_CHECK(FPlayerMarkerDataFastArrayItem);
	inline auto PinItemSize       = SR_SIZE_CHECK(FCrPlayerPersonalMarkerData);

	// ACrPlayerControllerBase::bMapMenuOpen is a native bool with no UPROPERTY.
	// UCrMapManuSubsystem::UpdateMapMenuData tests it on each player controller
	// and only then gathers the marker data. A stale offset would write into
	// whatever now sits there, so on a build the loader has no row for, the
	// compass is refused instead.
	// CL-125218 (Update 2) and CL-127004: 0xF68; the layout diff lists no move
	// on CL-128786. The rows below are this plugin's own until the loader's map
	// carries them.
	inline auto MapMenuOpen = PluginField<uint8_t>{ { "ACrPlayerControllerBase::bMapMenuOpen",
		"ACrPlayerControllerBase", "bMapMenuOpen", PLUGIN_FIELD_NATIVE, sizeof(uint8_t),
		static_cast<int32_t>(AobPatterns::Offsets::ACrPlayerControllerBase_bMapMenuOpen) } };

	inline const PluginFieldCandidate kMapMenuOpenRows[] = {
		{ 127004, static_cast<int32_t>(AobPatterns::Offsets::ACrPlayerControllerBase_bMapMenuOpen), 1, 0, nullptr },
		{ 128786, static_cast<int32_t>(AobPatterns::Offsets::ACrPlayerControllerBase_bMapMenuOpen), 1, 0, nullptr },
	};

	// Asks the loader for every row above. False when any did not resolve; the
	// loader has then recorded the failure and refuses the plugin.
	inline bool Resolve(IPluginSelf* self, IPluginHookScanner* scanner)
	{
		const bool nativeOk = PluginMap::ResolveField(scanner, self, MapMenuOpen,
			kMapMenuOpenRows, static_cast<uint32_t>(std::size(kMapMenuOpenRows)));
		return PluginMap::ResolveFields(scanner, self,
			BaseCoreUpgradeLevel, BaseCoreIsAttacked, BaseCoreIsInfected, EnemyDissolve,
			MapMenuHelper, PlayerPersonalMarkers,
			BuildingsContainer, FoundableContainer, PlayersContainer,
			BuildingsData, FoundableData, PlayersData,
			BuildingLocation, BuildingUniqueName, FoundableLocation, FoundableType,
			PlayerName, PlayerLocation, PlayerColor, PlayerRotationZ, PlayerFlags,
			PinPosition, PinPlayerName, PinColor,
			BuildingItemSize, FoundableItemSize, PlayerItemSize, PinItemSize) && nativeOk;
	}

	// The member's value, or a fallback when the member did not resolve.
	template <class T, class Object>
	T ValueOr(const PluginField<T>& field, const Object* object, T fallback = T{})
	{
		const T* at = field.In(static_cast<const void*>(object));
		return at ? *at : fallback;
	}

	// The array a container member of the map helper holds, or null when either
	// member did not resolve.
	template <class Container, class Array, class Helper>
	const Array* ArrayIn(const PluginField<Container>& container, const PluginField<Array>& array, const Helper* helper)
	{
		const Container* held = container.In(static_cast<const void*>(helper));
		return held ? array.In(static_cast<const void*>(held)) : nullptr;
	}

	// The game state's replicated map-menu helper, or null.
	inline SDK::ACrMapMenuDataReplicationHelper* MapHelperOf(const SDK::ACrGameStateBase* gameState)
	{
		return ValueOr<SDK::ACrMapMenuDataReplicationHelper*>(MapMenuHelper, gameState, nullptr);
	}
}
