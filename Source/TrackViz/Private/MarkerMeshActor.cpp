// Fill out your copyright notice in the Description page of Project Settings.

#include "MarkerMeshActor.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"


AMarkerMeshActor::AMarkerMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> MeshFinder(TEXT("StaticMesh'/TrackViz/Marker.Marker'"));
	Mesh->SetStaticMesh(MeshFinder.Get());
	RootComponent = Mesh;
	RootComponent->Mobility = EComponentMobility::Static;
	// SetMobility(EComponentMobility::Static);
}
