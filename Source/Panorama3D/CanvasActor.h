// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CanvasActor.generated.h"

UCLASS()
class PANORAMA3D_API ACanvasActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACanvasActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;


	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

	//Creat Masks
	void creatStitchingMask();
	void creatSimCaliMask();

	FVector getVRCamPos();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 FigSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 MaskSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 focalLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		bool TestTex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		bool useFig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 tempSrcBpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		UTexture2D* FigStitchingMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		UTexture2D* SimCaliMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInterface * MasterMaterialRef;


	//~~~ BP Referenced Materials ~~~
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInstanceDynamic * RV_MatInst;


	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* SphereVisual;

	UPROPERTY(EditAnywhere)
		AActor* CameraOne;

	UPROPERTY(EditAnywhere)
		AActor* CameraTwo;

	float TimeToNextCameraChange;
	
};
