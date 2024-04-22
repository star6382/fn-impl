#include "sdk.hpp"
#include <__msvc_chrono.hpp>

inline unsigned __int64 CurrentFrame = 0;
inline std::chrono::time_point<std::chrono::steady_clock> CurrentTime;

std::vector<Caches::FortPawnCache> TempCache;
tarray<uobject*> ReturnArray;

void FortPawn::Tick() {
	double ElapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(CurrentTime - FortPawn::LastCacheTime).count();
	if (ElapsedTime >= FortPawn::IntervalSeconds) {
		FortPawn::LastCacheTime = CurrentTime;

		ReturnArray = engine_structure::engine_objects::gameplaystatistics->get_all_actors_of_class((uobject*)uworld, SDK::AFortPawn::StaticClass());
		for (int i = 0; i < ReturnArray.Num(); i++) {
			Caches::FortPawnCache FortPawnCache{};

			FortPawnCache.FortPawn = reinterpret_cast<AFortPawn*, true>(ReturnArray[i]);
			/*	SDK::APlayerState* PlayerState = FortPawnCache.FortPawn->PlayerState();
				if (SDK::IsValidPointer(PlayerState)) {
					FortPawnCache.PlayerName = PlayerState->GetPlayerName();
					FortPawnCache.TeamIndex = SDK::Cast<SDK::AFortPlayerState>(PlayerState)->TeamIndex();
				}*/

			FortPawnCache.BonePositions3D.resize(FortPawn::BoneID_::BONEID_MAX);
			FortPawnCache.BonePositions2D.resize(FortPawn::BoneID_::BONEID_MAX);
			FortPawnCache.BoneVisibilityStates.resize(FortPawn::BoneID_::BONEID_MAX);

			TempCache.push_back(FortPawnCache);
		}

		FortPawn::CachedPlayers = TempCache;
	}

	std::vector<Caches::FortPawnCache> CachedPlayersLocal = FortPawn::CachedPlayers;
	for (auto it = CachedPlayersLocal.begin(); it != CachedPlayersLocal.end(); ++it) {
		Caches::FortPawnCache& CurrentPlayer = *it;

		AFortPawn* FortPawn = CurrentPlayer.FortPawn;
		CurrentPlayer.Mesh = FortPawn->Mesh();

		// LocalPawn caching and exploit ticks
		if (FortPawn == player_struct._world.GetLocalPawn()) {
			LocalPawnCache.Position = CurrentPlayer.Mesh->GetBonePosition(FortPawn::BoneID_::Head);
			// call any tick you want actors...
			// or call expoilts
			continue;
		}

		// Bone positions and visibility caching
		// If this returns false, the player isn't on the screen and only 5 of the bones were WorldToScreened
		CurrentPlayer.IsBoneRegister2DPopulated = PopulateBones(CurrentPlayer);
		PopulateVisibilities(CurrentPlayer);

		// Update IsPlayerVisibleOnScreen based on if any of the bones 2D positions are on the screen
		CurrentPlayer.IsPlayerVisibleOnScreen = false;
		for (int i = 0; i < CurrentPlayer.BonePositions2D.size(); i++) {
			if (CurrentPlayer.BonePositions2D[i] == FVector2D()) continue;

			if (IsOnScreen(CurrentPlayer.BonePositions2D[i])) {
				CurrentPlayer.IsPlayerVisibleOnScreen = true;
			}
		}

		CurrentPlayer.DistanceFromLocalPawn = LocalPawnCache.Position.Distance(CurrentPlayer.BonePositions3D[FortPawn::BoneID_::Root]) / 100.f;

		// Hardcoded max distance, should move to bone population for optimisation
		if (CurrentPlayer.DistanceFromLocalPawn > 500.f) continue;

		// Update any bone visibility
		CurrentPlayer.IsAnyBoneVisible = false;
		for (int i = 0; i < CurrentPlayer.BoneVisibilityStates.size(); i++) {
			if (CurrentPlayer.BoneVisibilityStates[i]) {
				CurrentPlayer.IsAnyBoneVisible = true;
				break;
			}
		}

		// Visuals
		if (CurrentPlayer.IsPlayerVisibleOnScreen) {
			FVector2D TopLeft, BottomRight;
			PopulateBoundCorners(CurrentPlayer, TopLeft, BottomRight);

			float FontSize = CalculateInterpolatedValue(150.f, CurrentPlayer.DistanceFromLocalPawn, 12.f, 20.f);
			float PrimaryThicknessMultiplier = CalculateInterpolatedValue(75.f, CurrentPlayer.DistanceFromLocalPawn, 1.f, 3.f);
			float SecondaryThicknessMultiplier = CalculateInterpolatedValue(75.f, CurrentPlayer.DistanceFromLocalPawn, 1.f, 2.f);

			float PrimaryThickness = 1.f * PrimaryThicknessMultiplier;
			float SecondaryThickness = 1.f * SecondaryThicknessMultiplier;

			flinearcolor PrimaryColor = flinearcolor(1.f, 1.f, 1.f, 1.f);
			if (CurrentPlayer.IsAnyBoneVisible) {
				PrimaryColor = flinearcolor(1.f, 0.f, 0.f, 1.f);
			}

			flinearcolor SecondaryColor = flinearcolor(1.0f, 0.f, 0.f, 1.0f);
			if (CurrentPlayer.IsAnyBoneVisible) {
				SecondaryColor = flinearcolor(0.0f, 1.f, 1.f, 1.0f);
			}

			if (CurrentPlayer.IsPlayerVisibleOnScreen) {


                        // your cheat esp here...


			}
		}
	}
}


void(*DrawTransitionOriginal)(UGameViewportClient*, UCanvas*) = nullptr;
void DrawTransition(UGameViewportClient* ViewPortClient, UCanvas* Canvas) {
	if (!ViewPortClient || !Canvas) {
		return;
	}

	screenx = reinterpret_cast<UCanvas*>(Canvas)->SZE().x;
	screeny = reinterpret_cast<UCanvas*>(Canvas)->SZE().y;
	if (!screenx || !screeny) {
		return;
	}

	//FortPawn::Update_Tick();
	FortPawn::Tick();

	return DrawTransitionOriginal(ViewPortClient, Canvas);
}


// Update the current weapon and magazine ammo count
/*	SDK::AFortWeapon* CurrentWeapon = FortPawn->CurrentWeapon();
	if (CurrentWeapon != CurrentPlayer.Weapon) {
		CurrentPlayer.Weapon = CurrentWeapon;
		CurrentPlayer.BulletsPerClip = CurrentWeapon->GetBulletsPerClip();
	}*/
