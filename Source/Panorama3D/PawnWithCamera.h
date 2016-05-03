// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "PawnWithCamera.generated.h"

UCLASS()
class PANORAMA3D_API APawnWithCamera : public APawn
{
	GENERATED_BODY()

public:

	// Sets default values for this pawn's properties
	APawnWithCamera();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitProperties() override;

	virtual void  APawnWithCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;


	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;


	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

	//Creat Masks
	void creatStitchingMask();
	void creatSimCaliMask();


	void CalDirs();
	void MoveCameras();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 MaskSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		int32 focalLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
		float InterpupillaryDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		UTexture2D* FigStitchingMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		UTexture2D* SimCaliMask;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInterface * MasterMaterialRef;

	//~~~ BP Referenced Materials ~~~
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		UMaterialInstanceDynamic * RV_MatInst;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		FVector FishEyePos1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		FVector FishEyePos2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		FVector FishEyePos3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreTexes")
		FVector FishEyePos4;

protected:
	UPROPERTY(EditAnywhere)
		USceneComponent* OurCameraBase;
	UPROPERTY(EditAnywhere)
		UCameraComponent* OurCamera;

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* SphereVisual;



	UPROPERTY(EditAnywhere)
		USceneComponent* LeftEyePos;
	UPROPERTY(EditAnywhere)
		USceneComponent* RightEyePos;

	FVector dir[4];
	FVector FishEyePos[4];
	float length[4];



	//Input variables
	FVector2D MovementInput;
	FVector2D CameraInput;

	//Input functions
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);
	
	
};
