// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackVizGameMode.h"
#include "TrackVizBPLibrary.h"
#include "Core.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerInput.h"
#include "Slate/SceneViewport.h"


ATrackVizGameMode::ATrackVizGameMode()
{
	DefaultPawnClass = ADefaultPawn::StaticClass();
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("TrackViz_LMB", EKeys::LeftMouseButton));
}


void ATrackVizGameMode::BeginPlay()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)	{
		PC->bShowMouseCursor = true;
	}
	PC->SetIgnoreLookInput(true);
	PC->InputComponent->BindAction(FName("TrackViz_LMB"), IE_Pressed, this, &ATrackVizGameMode::OnClick);
	PC->InputComponent->BindAction(FName("TrackViz_LMB"), IE_Released, this, &ATrackVizGameMode::OnRelease);

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

    TArray<FTrackRecord> records = UTrackVizBPLibrary::ReadTrackRecordsFromDir(absTracksDir);
    TArray<FColor> colors = UTrackVizBPLibrary::GetColorsForTrackRecords(records);
    for (int32 i = 0; i < records.Num(); ++i) {
        const FTrackRecord& record = records[i];
        const FColor& color = colors[i];
        UTrackVizBPLibrary::DrawTrackRecord(this, record, startPosition, color, 1);
		GEngine->AddOnScreenDebugMessage(i, 999999, color, record.FileName);
    }
}

void ATrackVizGameMode::OnClick()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->bShowMouseCursor = false;
	PC->SetIgnoreLookInput(false);
	bRotationEnabled = true;
	PC->GetLocalPlayer()->ViewportClient->Viewport->GetMousePos(MouseCursorPosition);
}

void ATrackVizGameMode::OnRelease()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->bShowMouseCursor = true;
	PC->SetIgnoreLookInput(true);
	bRotationEnabled = false;
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
