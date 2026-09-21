#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ServerDirectorySubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerDirectoryLoginSuccessTest,
	"PractiveUnreal.ServerDirectory.LoginSuccessResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerDirectoryLoginSuccessTest::RunTest(const FString& Parameters)
{
	FString ServerAddress;
	FString Message;
	const bool bParsed = FServerDirectoryResponseParser::TryParseLoginResponse(
		TEXT("{\"success\":true,\"serverAddress\":\"192.168.0.10:7777\"}"),
		ServerAddress,
		Message);

	TestTrue(TEXT("A successful response is accepted"), bParsed);
	TestEqual(TEXT("The registered server address is returned"), ServerAddress, FString(TEXT("192.168.0.10:7777")));
	TestTrue(TEXT("A successful response has no error message"), Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerDirectoryLoginErrorTest,
	"PractiveUnreal.ServerDirectory.LoginErrorResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerDirectoryLoginErrorTest::RunTest(const FString& Parameters)
{
	FString ServerAddress;
	FString Message;
	const bool bParsed = FServerDirectoryResponseParser::TryParseLoginResponse(
		TEXT("{\"success\":false,\"message\":\"Invalid credentials\"}"),
		ServerAddress,
		Message);

	TestFalse(TEXT("An API error response is rejected"), bParsed);
	TestEqual(TEXT("The API error message is preserved"), Message, FString(TEXT("Invalid credentials")));
	TestTrue(TEXT("An error response cannot provide a travel address"), ServerAddress.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerDirectoryMissingAddressTest,
	"PractiveUnreal.ServerDirectory.MissingAddressResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerDirectoryMissingAddressTest::RunTest(const FString& Parameters)
{
	FString ServerAddress;
	FString Message;
	const bool bParsed = FServerDirectoryResponseParser::TryParseLoginResponse(
		TEXT("{\"success\":true}"),
		ServerAddress,
		Message);

	TestFalse(TEXT("A response without a server address is rejected"), bParsed);
	TestEqual(TEXT("The parser explains the malformed response"), Message, FString(TEXT("Login response did not contain a server address")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerDirectoryMalformedAddressTest,
	"PractiveUnreal.ServerDirectory.MalformedAddressResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerDirectoryMalformedAddressTest::RunTest(const FString& Parameters)
{
	FString ServerAddress;
	FString Message;
	const bool bParsed = FServerDirectoryResponseParser::TryParseLoginResponse(
		TEXT("{\"success\":true,\"serverAddress\":\"not-an-address\"}"),
		ServerAddress,
		Message);

	TestFalse(TEXT("A malformed travel address is rejected"), bParsed);
	TestEqual(TEXT("The parser explains the malformed address"), Message, FString(TEXT("Login response contained an invalid server address")));
	TestTrue(TEXT("A malformed address cannot be used for travel"), ServerAddress.IsEmpty());
	return true;
}

#endif
