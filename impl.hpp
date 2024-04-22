#include "sdk.hpp"
#include <__msvc_chrono.hpp>

std::vector<Caches::FortPawnCache> TempCache;
tarray<AFortPawn*> ReturnArray;

inline unsigned __int64 CurrentFrame = 0;
inline std::chrono::time_point<std::chrono::steady_clock> CurrentTime;

namespace FortPawn {
	void Tick();
	void Update_Tick();

	enum BoneID_ : uint8_t {
		Head = 1,           // "head"
		Neck = 2,           // "neck_01"

		// Bottom contains the root component position. We have two with the same value for code readability
		Root = 3,           // "Root"

		ChestLeft = 4,	    // "clavicle_l"
		ChestRight = 5,     // "clavicle_r"

		// There is no chest bone, so this is a dummy value used for the hierarchy, chest should be calculated based on left and right chest bones
		Chest = 6,          // none

		LeftShoulder = 7,   // "upperarm_l"
		LeftElbow = 8,	    // "lowerarm_l"
		LeftHand = 9,	    // "Hand_L"
		RightShoulder = 10, // "upperarm_r"
		RightElbow = 11,    // "lowerarm_r"
		RightHand = 12,	    // "hand_r"

		LeftLeg = 13,	    // "thigh_l"
		LeftKnee = 14,	    // "calf_l"
		LeftFoot = 15,	    // "foot_l"
		RightLeg = 16,	    // "thigh_r"
		RightKnee = 17,	    // "calf_r"
		RightFoot = 18,	    // "foot_r"

		Pelvis = 19,	    // "pelvis"

		BONEID_MAX = 20,           // Max value for looping

		None = 0,		    // "None"
	};

	inline std::vector<Caches::FortPawnCache> CachedPlayers;

	inline const float IntervalSeconds = 0.25f;
	inline std::chrono::steady_clock::time_point LastCacheTime = std::chrono::steady_clock::now();
}

bool PopulateBones(Caches::FortPawnCache& FortPawnCache) {
	// Resize the bone register to avoid out of range errors
	FortPawnCache.BonePositions3D.resize(FortPawn::BoneID_::BONEID_MAX);
	FortPawnCache.BonePositions2D.resize(FortPawn::BoneID_::BONEID_MAX);
	FortPawnCache.BoneVisibilityStates.resize(FortPawn::BoneID_::BONEID_MAX);

	bool FoundBoneOnScreen = false;

	for (int i = FortPawn::BoneID_::Head; i < FortPawn::BoneID_::BONEID_MAX; i++) {
		FortPawnCache.BonePositions3D[i] = FortPawnCache.Mesh->GetBonePosition(i);

		if (i == FortPawn::BoneID_::Head) {
			if (FortPawnCache.BonePositions3D[FortPawn::BoneID_::Head] == FVector(0, 0, 0)) {
				return false;
			}
		}

		if (i == FortPawn::BoneID_::Chest) {
			FortPawnCache.BonePositions3D[FortPawn::BoneID_::Chest] = (FortPawnCache.BonePositions3D[FortPawn::BoneID_::ChestLeft] + FortPawnCache.BonePositions3D[FortPawn::BoneID_::ChestRight]) /*/ 2*/;
		}

		// To avoid W2Sing players that aren't on the screen
		if (i <= 5 || FoundBoneOnScreen) {
			FortPawnCache.BonePositions2D[i] = SDK::Project(FortPawnCache.BonePositions3D[i]);

			if (IsOnScreen(FortPawnCache.BonePositions2D[i])) {
				FoundBoneOnScreen = true;
			}
		}
	}

	return FoundBoneOnScreen;
}

//void Features::FortPawnHelper::PopulateVisibilities(Actors::Caches::FortPawnCache& FortPawnCache) {
//	// Resize the vectors to the maximum bone ID
//	FortPawnCache.BonePositions3D.resize(Bone::BONEID_MAX);
//	FortPawnCache.BonePositions2D.resize(Bone::BONEID_MAX);
//	FortPawnCache.BoneVisibilityStates.resize(Bone::BONEID_MAX);
//
//	// Loop over each bone ID
//	for (int boneId = 0; boneId < Bone::BONEID_MAX; ++boneId) {
//		// Check if the bone position is visible
//		bool isVisible = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[boneId], FortPawnCache.FortPawn, SDK::GetLocalPawn());
//
//		// Update the visibility state of the bone
//		FortPawnCache.BoneVisibilityStates[boneId] = isVisible;
//	}
//}

void PopulateVisibilities(Caches::FortPawnCache& FortPawnCache) {
	FortPawnCache.BonePositions3D.resize(FortPawn::BoneID_::BONEID_MAX);
	FortPawnCache.BonePositions2D.resize(FortPawn::BoneID_::BONEID_MAX);
	FortPawnCache.BoneVisibilityStates.resize(FortPawn::BoneID_::BONEID_MAX);

	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::Head] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::Head], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::Chest] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::Chest], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftShoulder] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftShoulder], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightShoulder] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightShoulder], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftElbow] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftElbow], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightElbow] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightElbow], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftHand] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftHand], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightHand] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightHand], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftLeg] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftLeg], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightLeg] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightLeg], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftKnee] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftKnee], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightKnee] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightKnee], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::LeftFoot] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::LeftFoot], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::RightFoot] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::RightFoot], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
	FortPawnCache.BoneVisibilityStates[FortPawn::BoneID_::Pelvis] = SDK::IsPositionVisible(FortPawnCache.FortPawn, FortPawnCache.BonePositions3D[FortPawn::BoneID_::Pelvis], FortPawnCache.FortPawn, player_struct._world.GetLocalPawn());
}

void PopulateBoundCorners(Caches::FortPawnCache& FortPawnCache, fvector2d& TopLeft, fvector2d& BottomRight) {
	TopLeft = fvector2d(FLT_MAX, FLT_MAX);
	BottomRight = fvector2d(-FLT_MAX, -FLT_MAX);

	for (int i = FortPawn::BoneID_::Head; i < FortPawn::BoneID_::BONEID_MAX; i++) {
		TopLeft.x = min(TopLeft.x, FortPawnCache.BonePositions2D[i].x);
		TopLeft.y = min(TopLeft.y, FortPawnCache.BonePositions2D[i].y);

		BottomRight.x = max(BottomRight.x, FortPawnCache.BonePositions2D[i].x);
		BottomRight.y = max(BottomRight.y, FortPawnCache.BonePositions2D[i].y);
	}
	
	// Adjust the bounding box to make it more visually appealing
	float BoxSizeMultiplier = CalculateInterpolatedValue(FortPawnCache.DistanceFromLocalPawn, 100.f, 1.0f, 4.0f);

	// Increase the size of the bounding box by a percentage of the original size. This is to make the bounding box more visually appealing.
	float LeftRightOffset = (BottomRight.x - TopLeft.x) * (0.36f * BoxSizeMultiplier);
	float TopBottomOffset = (BottomRight.y - TopLeft.y) * (0.14f * BoxSizeMultiplier);

	TopLeft.x -= LeftRightOffset;
	TopLeft.y -= TopBottomOffset;

	BottomRight.x += LeftRightOffset;
	BottomRight.y += TopBottomOffset;
}

void FortPawn::Tick() {
	std::vector<Caches::FortPawnCache> CachedPlayersLocal = FortPawn::CachedPlayers;
	for (auto it = CachedPlayersLocal.begin(); it != CachedPlayersLocal.end(); ++it) {
		Caches::FortPawnCache& CurrentPlayer = *it;

		AFortPawn* FortPawn = CurrentPlayer.FortPawn;
		CurrentPlayer.Mesh = FortPawn->Mesh();

		// LocalPawn caching and exploit ticks
		if (FortPawn == player_struct._world.GetLocalPawn()) {

			//LocalPawnCache.Position = CurrentPlayer.Mesh->GetBonePosition(FortPawn::BoneID_::Head);		

			continue;
		}
	}



	//cheat here




}

void FortPawn::Update_Tick() { {
		double ElapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(CurrentTime - FortPawn::LastCacheTime).count();

		if (ElapsedTime >= FortPawn::IntervalSeconds) {
			FortPawn::LastCacheTime = CurrentTime;

			/*ReturnArray = SDK::UGameplayStatics::GetAllActorsOfClass(SDK::GetWorld(), SDK::AFortPawn::StaticClass())*/

			for (int i = 0; i < ReturnArray.size(); i++) {
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

	FortPawn::Tick();
	FortPawn::Update_Tick();

	return DrawTransitionOriginal(ViewPortClient, Canvas);
}
