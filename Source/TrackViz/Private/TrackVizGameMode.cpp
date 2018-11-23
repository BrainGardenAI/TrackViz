// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackVizGameMode.h"
#include "TrackVizBPLibrary.h"
#include "Core.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "GameFramework/PlayerInput.h"
#include "Slate/SceneViewport.h"
#include "StaticCameraPawn.h"
#include "TrackViz.h"


ATrackVizGameMode::ATrackVizGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("TrackViz_LMB", EKeys::LeftMouseButton));
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("TrackViz_X", EKeys::X));
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("TrackViz_Z", EKeys::Z));
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("TrackViz_R", EKeys::R));
}


void ATrackVizGameMode::DrawTracks()
{
	for (int32 iTrack = 0; iTrack < TrackRecords.Num(); ++iTrack) {
		const FTrackRecord& record = TrackRecords[iTrack];
		const FColor& color = Colors[iTrack];
		UTrackVizBPLibrary::DrawTrackRecord(this, record, startPosition, color, LineThickness);
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(iTrack), 999999, color, record.FileName);
	}
}

void ATrackVizGameMode::Reload()
{
	FlushPersistentDebugLines(GEngine->GetWorldFromContextObjectChecked(this));
	DrawTracks();
}

void ATrackVizGameMode::BeginPlay()
{
	PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	DefaultPawn = PC->GetPawn();
	DefaultPawn->SetActorEnableCollision(false);
	if (PC)	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		PC->ClickEventKeys.Add(EKeys::RightMouseButton);
	}
	PC->SetIgnoreLookInput(true);
	PC->InputComponent->BindAction(FName("TrackViz_LMB"), IE_Pressed, this, &ATrackVizGameMode::OnClick);
	PC->InputComponent->BindAction(FName("TrackViz_LMB"), IE_Released, this, &ATrackVizGameMode::OnRelease);
	PC->InputComponent->BindAction(FName("TrackViz_X"), IE_Pressed, this, &ATrackVizGameMode::OnPressedX);
	PC->InputComponent->BindAction(FName("TrackViz_Z"), IE_Pressed, this, &ATrackVizGameMode::TogglePawnsVisibility);
	PC->InputComponent->BindAction(FName("TrackViz_R"), IE_Pressed, this, &ATrackVizGameMode::Reload);

    TActorIterator<APlayerStart> itr(GetWorld());
    if (itr) {
        startPosition = itr->GetActorLocation();
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Failed to locate PlayerStart, using origin as start position for tracks");
    }

    const FString& absTracksDir = isRelativePath ? FPaths::ProjectDir() + tracksDir : tracksDir;
	if (!FPaths::DirectoryExists(absTracksDir)) {
		GEngine->AddOnScreenDebugMessage(-1, 999999, FColor::Red, "Failed to locate tracks directory " + absTracksDir);
		return;
	}

    TrackRecords = UTrackVizBPLibrary::ReadTrackRecordsFromDir(absTracksDir);
    Colors = UTrackVizBPLibrary::GetColorsForTrackRecords(TrackRecords);
	
	DrawTracks();

	for (int32 iTrack = 0; iTrack < TrackRecords.Num(); ++iTrack) {
		const FTrackRecord& record = TrackRecords[iTrack];
		const FColor& color = Colors[iTrack];
		for (int32 iPoint = 0; iPoint < record.Positions.Num(); ++iPoint) {
			const FVector& position = record.Positions[iPoint];
			auto p = GetWorld()->SpawnActor<AStaticCameraPawn>(startPosition + position, FRotator(0, 0, 0), FActorSpawnParameters());
			p->TrackIndex = iTrack;
			p->PointIndex = iPoint;
			p->Color = color;
			Pawns.Add(p);
			p->OnClicked.AddDynamic(this, &ATrackVizGameMode::OnPressedStaticPawn);
			UMaterialInterface * Material = p->Mesh->GetMaterial(0);
			UMaterialInstanceDynamic* MatInstance = p->Mesh->CreateDynamicMaterialInstance(0, Material);
			if (MatInstance) {
				MatInstance->SetVectorParameterValue("Color", FLinearColor(color));
			}
		}
	}
	bPawnsVisible = true;

	TArray<UStaticMeshComponent*> Components;
	DefaultPawn->GetComponents<UStaticMeshComponent>(Components);
	Components[0]->ToggleVisibility();

	ShowTooltip();
}

void ATrackVizGameMode::OnClick()
{
	PC->bShowMouseCursor = false;
	PC->bEnableClickEvents = false;
	PC->bEnableMouseOverEvents = false;
	PC->SetIgnoreLookInput(false);
	bRotationEnabled = true;
	PC->GetLocalPlayer()->ViewportClient->Viewport->GetMousePos(MouseCursorPosition);
}

void ATrackVizGameMode::OnRelease()
{
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	PC->SetIgnoreLookInput(true);
	bRotationEnabled = false;
}

void ATrackVizGameMode::OnPressedX()
{
	if (PC->GetPawn() != DefaultPawn) {
		PC->UnPossess();
		PC->Possess(DefaultPawn);
		SetPawnsVisibility(true);
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(true);
		ShowTooltip();
	}
}

void ATrackVizGameMode::Tick(float DeltaSeconds)
{
	if (bRotationEnabled) {
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FViewport* viewport = PC->GetLocalPlayer()->ViewportClient->Viewport;
		FVector2D ViewportSize;
		PC->GetLocalPlayer()->ViewportClient->GetViewportSize(ViewportSize);
		viewport->SetMouse(MouseCursorPosition.X, MouseCursorPosition.Y);
	}
}

void ATrackVizGameMode::OnPressedStaticPawn(AActor* actor, FKey key)
{
	if (!bPawnsVisible) {
		return;
	}
	const auto pawn = dynamic_cast<AStaticCameraPawn*>(actor);
	if (key == EKeys::LeftMouseButton) {
		DefaultPawn->GetRootComponent()->ToggleVisibility();
		PC->UnPossess();
		PC->Possess(pawn);
		SetPawnsVisibility(false);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		GEngine->AddOnScreenDebugMessage(TrackRecords.Num(), 999999, FColor::White, "Press X to go back");
		return;
	}
	if (key == EKeys::RightMouseButton) {
		if (pawn->TrackIndex != -1 && pawn->PointIndex != -1) {
			for (int OtherTrackIndex = 0; OtherTrackIndex < TrackRecords.Num(); ++OtherTrackIndex)
			{
				if (OtherTrackIndex == pawn->TrackIndex
					|| TrackRecords[OtherTrackIndex].Positions.Num() != TrackRecords[pawn->TrackIndex].Positions.Num())
				{
					continue;
				}
				FVector from = TrackRecords[pawn->TrackIndex].Positions[pawn->PointIndex];
				FVector to = TrackRecords[OtherTrackIndex].Positions[pawn->PointIndex];
				UTrackVizBPLibrary::DrawLine(this, startPosition + from, startPosition + to, FColor(120, 120, 120), true, LineThickness / 2);
			}
		}
	}
}

void ATrackVizGameMode::SetPawnsVisibility(bool visibility)
{
	bPawnsVisible = visibility;
	for (const AStaticCameraPawn* pawn : Pawns) {
		TArray<UStaticMeshComponent*> Components;
		pawn->GetComponents<UStaticMeshComponent>(Components);
		for (auto c : Components) {
			c->SetVisibility(bPawnsVisible);
		}
		Components[0]->SetVisibility(bPawnsVisible);
	}
}

void ATrackVizGameMode::TogglePawnsVisibility()
{
	SetPawnsVisibility(!bPawnsVisible);
}

void ATrackVizGameMode::ShowTooltip()
{
	GEngine->AddOnScreenDebugMessage(
		TrackRecords.Num(),
		999999,
		FColor::White,
		"LMB to possess a point; RMB to match points between tracks with equal number of points; R to hide matches; Z to hide point markers"
	);
}
