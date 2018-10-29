// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackVizGameMode.h"
#include "TrackVizBPLibrary.h"
#include "Core.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Engine/GameEngine.h"


void ATrackVizGameMode::BeginPlay()
{
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
