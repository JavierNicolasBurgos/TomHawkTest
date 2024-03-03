// Copyright Epic Games, Inc. All Rights Reserved.

#include "TomHawkTestCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstanceProxy.h"
#include "Helpers/TomHawkTestHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATomHawkTestCharacter

ATomHawkTestCharacter::ATomHawkTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	SkateboardStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateStaticMesh"));
	SkateboardStaticMesh->SetupAttachment(RootComponent);
}

void ATomHawkTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATomHawkTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector LineTraceStart = SkateboardStaticMesh->GetSocketLocation(UTomHawkTestHelpers::GetSkateboardForwardSocketName());
	FVector LineTraceEnd = LineTraceStart + FVector(0.0, 0.0, -30.0);
	LineTraceStart += FVector(0.0f, 0.0f, +30.0f);
	
	FHitResult ForwardSocketHitResult;
	TArray<AActor*> ActorToIgnore;
	ActorToIgnore.Emplace(this);
	ActorToIgnore.Emplace(SkateboardStaticMesh->GetStaticMesh());
	ActorToIgnore.Emplace(GetCapsuleComponent()->ShapeBodySetup);
	bool bLineTraceSingle = UKismetSystemLibrary::LineTraceSingle(this, LineTraceStart, LineTraceEnd, ETraceTypeQuery::TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::ForOneFrame, ForwardSocketHitResult, true);
	DrawDebugLine(GetWorld(), LineTraceStart, LineTraceEnd, FColor::White, false, 0.0f, -1, 1.0f);
	FVector ForwardSkateboardVector;
	if (bLineTraceSingle)
	{
		ForwardSkateboardVector = ForwardSocketHitResult.Location;
	}
	else
	{
		ForwardSkateboardVector = LineTraceStart;
	}
	
	FVector LineTraceStart2 = SkateboardStaticMesh->GetSocketLocation(UTomHawkTestHelpers::GetSkateboardBackwardSocketName());
	FVector LineTraceEnd2 = LineTraceStart2 + FVector(0.0, 0.0, -30.0);
	LineTraceStart2 += FVector(0.0f, 0.0f, +30.0f);
	
	FHitResult BackwardSocketHitResult;
	bool bLineTraceSingle2 = UKismetSystemLibrary::LineTraceSingle(this, LineTraceStart2, LineTraceEnd2, ETraceTypeQuery::TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::ForOneFrame, BackwardSocketHitResult, true);
	DrawDebugLine(GetWorld(), LineTraceStart2, LineTraceEnd2, FColor::Blue, false, 0.0f, -1, 1.0f);
	FVector BackwardSkateboardVector;
	if (bLineTraceSingle2)
	{
		BackwardSkateboardVector = BackwardSocketHitResult.Location;
	}
	else
	{
		BackwardSkateboardVector = LineTraceStart2;
	}

	const FRotator NewSkateboardRotation = UKismetMathLibrary::FindLookAtRotation(BackwardSkateboardVector, ForwardSkateboardVector);
	SkateboardStaticMesh->SetWorldRotation(NewSkateboardRotation);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATomHawkTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATomHawkTestCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATomHawkTestCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATomHawkTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	// For this example we do not take into account the backward movement.
	if (MovementVector.Y >= 0 && Controller != nullptr)
	{
		ForwardAccelerationValue = FMath::Lerp(ForwardAccelerationValue, MovementVector.Y, 0.01f);
		
		// add movement 
		AddMovementInput(SkateboardStaticMesh->GetForwardVector(), ForwardAccelerationValue);
		AddMovementInput(SkateboardStaticMesh->GetRightVector(), MovementVector.X * TurnRightVelocity);
	}
}

void ATomHawkTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// using to the character move in the skateboard direction
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}