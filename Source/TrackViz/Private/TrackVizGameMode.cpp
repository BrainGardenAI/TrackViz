// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackVizGameMode.h"
#include "TrackVizBPLibrary.h"
#include "Core.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "GameFramework/PlayerInput.h"
#include "Slate/SceneViewport.h"
#include "MarkerMeshActor.h"


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
	PC->InputComponent->BindAction(FName("TrackViz_Z"), IE_Pressed, this, &ATrackVizGameMode::ToggleMarkersVisibility);
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

			auto p = GetWorld()->SpawnActor<AMarkerMeshActor>(startPosition + position, FRotator(0, 0, 0), FActorSpawnParameters());
			p->TrackIndex = iTrack;
			p->PointIndex = iPoint;
			p->Color = color;
			Markers.Add(p);
			p->OnClicked.AddDynamic(this, &ATrackVizGameMode::OnPressedMarker);
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
	if (bMarkerPossessed) {
		PC->SetIgnoreMoveInput(false);
		DefaultPawn->SetActorLocation(SavedDefaultPawnLocation);
		PC->SetControlRotation(SavedControlRotation);
		SetMarkersVisibility(true);
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

void ATrackVizGameMode::OnPressedMarker(AActor* actor, FKey key)
{
	if (!bPawnsVisible) {
		return;
	}
	const auto MarkerActor = dynamic_cast<AMarkerMeshActor*>(actor);

	if (key == EKeys::LeftMouseButton) {

		SavedDefaultPawnLocation = DefaultPawn->GetActorLocation();
		SavedControlRotation = PC->GetControlRotation();
		DefaultPawn->SetActorLocation(MarkerActor->GetActorLocation());
		PC->SetControlRotation(MarkerActor->GetActorRotation());

		PC->SetIgnoreMoveInput(true);
		bMarkerPossessed = true;
		SetMarkersVisibility(false);
		GEngine->AddOnScreenDebugMessage(TrackRecords.Num(), 999999, FColor::White, "Press X to go back");
		
		return;
	}
	if (key == EKeys::RightMouseButton) {
		const FTrackRecord& MarkerTrackRecord = TrackRecords[MarkerActor->TrackIndex];
		const FVector& MarkerPosition = MarkerTrackRecord.Positions[MarkerActor->PointIndex];
		UTrackVizBPLibrary::DrawArrow(
			this,
			startPosition + MarkerPosition,
			MarkerTrackRecord.Rotators[MarkerActor->PointIndex],
			MarkerActor->Color,
			true,
			ArrowThickness
		);
		if (MarkerActor->TrackIndex != -1 && MarkerActor->PointIndex != -1) {
			for (int OtherTrackIndex = 0; OtherTrackIndex < TrackRecords.Num(); ++OtherTrackIndex)
			{
				if (OtherTrackIndex == MarkerActor->TrackIndex
					|| TrackRecords[OtherTrackIndex].Positions.Num() != MarkerTrackRecord.Positions.Num())
				{
					continue;
				}
				FVector from = MarkerPosition;
				FVector to = TrackRecords[OtherTrackIndex].Positions[MarkerActor->PointIndex];
				UTrackVizBPLibrary::DrawLine(this, startPosition + from, startPosition + to, FColor(120, 120, 120), true, ConnectionThickness);
				UTrackVizBPLibrary::DrawArrow(
					this,
					startPosition + to,
					TrackRecords[OtherTrackIndex].Rotators[MarkerActor->PointIndex],
					Colors[OtherTrackIndex],
					true,
					ArrowThickness
				);
			}
		}
	}
}

void ATrackVizGameMode::SetMarkersVisibility(bool visibility)
{
	bPawnsVisible = visibility;
	for (const AMarkerMeshActor* marker : Markers) {
		TArray<UStaticMeshComponent*> Components;
		marker->GetComponents<UStaticMeshComponent>(Components);
		for (auto c : Components) {
			c->SetVisibility(bPawnsVisible);
		}
		Components[0]->SetVisibility(bPawnsVisible);
	}
}

void ATrackVizGameMode::ToggleMarkersVisibility()
{
	SetMarkersVisibility(!bPawnsVisible);
}

void ATrackVizGameMode::ShowTooltip()
{
	GEngine->AddOnScreenDebugMessage(
		TrackRecords.Num(),
		999999,
		FColor::White,
		"LMB to possess a point; RMB to match points between tracks with equal number of points and show track rotators; R to hide matches; Z to hide point markers"
	);
}
