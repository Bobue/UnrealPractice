#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ServerDirectorySubsystem.generated.h"

class FSubsystemCollectionBase;

class PRACTIVEUNREAL_API FServerDirectoryResponseParser
{
public:
	static bool TryParseLoginResponse(const FString& Json, FString& OutServerAddress, FString& OutMessage);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FServerDirectoryStatusChanged, bool, bSuccess, const FString&, Message);

UCLASS(Config=Game)
class PRACTIVEUNREAL_API UServerDirectorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Server Directory")
	void StartListenServer();

	UFUNCTION(BlueprintCallable, Category="Server Directory")
	void LoginAndConnect(const FString& UserId, const FString& Password);

	UFUNCTION(BlueprintPure, Category="Server Directory")
	const FString& GetLastServerAddress() const { return LastServerAddress; }

	UPROPERTY(BlueprintAssignable, Category="Server Directory")
	FServerDirectoryStatusChanged OnStatusChanged;

private:
	UPROPERTY(Config, EditAnywhere, Category="Server Directory")
	FString WebServerBaseUrl = TEXT("http://127.0.0.1:8080");

	UPROPERTY(Config, EditAnywhere, Category="Server Directory")
	FString RegistrationToken = TEXT("assignment-secret");

	UPROPERTY(Config, EditAnywhere, Category="Server Directory")
	FString DefaultServerAddress = TEXT("127.0.0.1:7777");

	UPROPERTY(Config, EditAnywhere, Category="Server Directory")
	FString ListenMapName = TEXT("Lobby");

	FString LastServerAddress;

	void RegisterServer(bool bOpenListenLevelAfterRegistration);
	FString ResolvePublicServerAddress() const;
	FString MakeApiUrl(const FString& Path) const;
	void HandleRegistrationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, bool bOpenListenLevel);
	void HandleLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void BroadcastFailure(const FString& Message);
};
