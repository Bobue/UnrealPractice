#include "ServerDirectorySubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	bool DeserializeObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}

	FString ReadApiMessage(const FString& Json, const FString& Fallback)
	{
		TSharedPtr<FJsonObject> Object;
		FString Message;
		if (DeserializeObject(Json, Object) && Object->TryGetStringField(TEXT("message"), Message) && !Message.IsEmpty())
		{
			return Message;
		}
		return Fallback;
	}

	bool IsValidServerAddress(const FString& Address)
	{
		if (Address.IsEmpty() || Address.Contains(TEXT(" ")) || Address.Contains(TEXT("\t")) ||
			Address.Contains(TEXT("@")) || Address.Contains(TEXT("/")) || Address.Contains(TEXT("?")) ||
			Address.Contains(TEXT("#")) || Address.Contains(TEXT("\\")))
		{
			return false;
		}

		int32 SeparatorIndex = INDEX_NONE;
		if (!Address.FindLastChar(TEXT(':'), SeparatorIndex) || SeparatorIndex <= 0 || SeparatorIndex >= Address.Len() - 1)
		{
			return false;
		}

		const FString Host = Address.Left(SeparatorIndex);
		const FString PortText = Address.Mid(SeparatorIndex + 1);
		if (!PortText.IsNumeric())
		{
			return false;
		}

		const int32 Port = FCString::Atoi(*PortText);
		const bool bBracketedIpv6 = Host.StartsWith(TEXT("[")) && Host.EndsWith(TEXT("]")) && Host.Len() > 2;
		const bool bSimpleHost = !Host.Contains(TEXT(":"));
		return Port >= 1 && Port <= 65535 && (bSimpleHost || bBracketedIpv6);
	}
}

bool FServerDirectoryResponseParser::TryParseLoginResponse(
	const FString& Json,
	FString& OutServerAddress,
	FString& OutMessage)
{
	OutServerAddress.Reset();
	OutMessage.Reset();

	TSharedPtr<FJsonObject> Object;
	if (!DeserializeObject(Json, Object))
	{
		OutMessage = TEXT("Login response was not valid JSON");
		return false;
	}

	bool bSuccess = false;
	if (!Object->TryGetBoolField(TEXT("success"), bSuccess))
	{
		OutMessage = TEXT("Login response did not contain a success flag");
		return false;
	}

	if (!bSuccess)
	{
		if (!Object->TryGetStringField(TEXT("message"), OutMessage) || OutMessage.IsEmpty())
		{
			OutMessage = TEXT("Login failed");
		}
		return false;
	}

	if (!Object->TryGetStringField(TEXT("serverAddress"), OutServerAddress) || OutServerAddress.IsEmpty())
	{
		OutServerAddress.Reset();
		OutMessage = TEXT("Login response did not contain a server address");
		return false;
	}

	if (!IsValidServerAddress(OutServerAddress))
	{
		OutServerAddress.Reset();
		OutMessage = TEXT("Login response contained an invalid server address");
		return false;
	}

	return true;
}

void UServerDirectorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsRunningDedicatedServer())
	{
		RegisterServer(false);
	}
}

void UServerDirectorySubsystem::StartListenServer()
{
	OnStatusChanged.Broadcast(true, TEXT("서버 주소를 웹서버에 등록하는 중입니다..."));
	RegisterServer(true);
}

void UServerDirectorySubsystem::LoginAndConnect(const FString& UserId, const FString& Password)
{
	if (UserId.IsEmpty() || Password.IsEmpty())
	{
		BroadcastFailure(TEXT("아이디와 비밀번호를 입력해 주세요."));
		return;
	}

	const TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("userId"), UserId);
	Payload->SetStringField(TEXT("password"), Password);

	FString RequestBody;
	FJsonSerializer::Serialize(Payload, TJsonWriterFactory<>::Create(&RequestBody));

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(MakeApiUrl(TEXT("/api/login")));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerDirectorySubsystem::HandleLoginResponse);

	OnStatusChanged.Broadcast(true, TEXT("로그인하고 접속할 서버를 조회하는 중입니다..."));
	if (!Request->ProcessRequest())
	{
		BroadcastFailure(TEXT("로그인 요청을 시작하지 못했습니다."));
	}
}

void UServerDirectorySubsystem::RegisterServer(const bool bOpenListenLevelAfterRegistration)
{
	const FString ServerAddress = ResolvePublicServerAddress();
	const TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("serverAddress"), ServerAddress);

	FString RequestBody;
	FJsonSerializer::Serialize(Payload, TJsonWriterFactory<>::Create(&RequestBody));

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(MakeApiUrl(TEXT("/api/server/register")));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *RegistrationToken));
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(
		this,
		&UServerDirectorySubsystem::HandleRegistrationResponse,
		bOpenListenLevelAfterRegistration);

	if (!Request->ProcessRequest())
	{
		BroadcastFailure(TEXT("서버 등록 요청을 시작하지 못했습니다."));
	}
}

FString UServerDirectorySubsystem::ResolvePublicServerAddress() const
{
	FString CommandLineAddress;
	if (FParse::Value(FCommandLine::Get(), TEXT("PublicServerAddress="), CommandLineAddress) && !CommandLineAddress.IsEmpty())
	{
		return CommandLineAddress;
	}
	return DefaultServerAddress;
}

FString UServerDirectorySubsystem::MakeApiUrl(const FString& Path) const
{
	FString BaseUrl = WebServerBaseUrl;
	BaseUrl.RemoveFromEnd(TEXT("/"));
	return BaseUrl + Path;
}

void UServerDirectorySubsystem::HandleRegistrationResponse(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	const bool bConnectedSuccessfully,
	const bool bOpenListenLevel)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		BroadcastFailure(TEXT("웹서버에 연결하지 못했습니다. 웹서버 실행 여부와 주소를 확인해 주세요."));
		return;
	}

	const int32 StatusCode = Response->GetResponseCode();
	if (StatusCode < 200 || StatusCode >= 300)
	{
		BroadcastFailure(ReadApiMessage(Response->GetContentAsString(), FString::Printf(TEXT("서버 등록 실패 (HTTP %d)"), StatusCode)));
		return;
	}

	LastServerAddress = ResolvePublicServerAddress();
	OnStatusChanged.Broadcast(true, FString::Printf(TEXT("서버 등록 완료: %s"), *LastServerAddress));
	UE_LOG(LogTemp, Log, TEXT("Registered Unreal server: %s"), *LastServerAddress);

	if (bOpenListenLevel)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*ListenMapName), true, TEXT("listen"));
	}
}

void UServerDirectorySubsystem::HandleLoginResponse(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	const bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		BroadcastFailure(TEXT("웹서버에 연결하지 못했습니다. 웹서버 실행 여부와 주소를 확인해 주세요."));
		return;
	}

	FString ServerAddress;
	FString Message;
	if (!FServerDirectoryResponseParser::TryParseLoginResponse(Response->GetContentAsString(), ServerAddress, Message))
	{
		BroadcastFailure(Message);
		return;
	}

	if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
	{
		BroadcastFailure(FString::Printf(TEXT("로그인 실패 (HTTP %d)"), Response->GetResponseCode()));
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		BroadcastFailure(TEXT("접속에 사용할 플레이어 컨트롤러를 찾지 못했습니다."));
		return;
	}

	LastServerAddress = ServerAddress;
	OnStatusChanged.Broadcast(true, FString::Printf(TEXT("서버 접속 중: %s"), *ServerAddress));
	PlayerController->ClientTravel(ServerAddress, TRAVEL_Absolute);
}

void UServerDirectorySubsystem::BroadcastFailure(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("Server directory: %s"), *Message);
	OnStatusChanged.Broadcast(false, Message);
}
