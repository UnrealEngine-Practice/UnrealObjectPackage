// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"
// 8-2 이 헤더가 있어야 패키지를 저장할 떄 쓰이는 함수를 쓸 수  있다.
#include "UObject/SavePackage.h"

/* 3. pacakge 이름을 정할떄는 규칙이 있다.
 * 언리얼 엔진 프로젝트를 시작하면 각각에 대한 고유하 경로가 지정된다.
 * 그 고유한 경로 중 하나가 /Game이라고 하는 경로이다.
 * 이 /Game은 애샛들을 모아놓은 디렉토리이다.
 * Saved라는 위치는 /Temp에 매핑되어있다. 
*/
const FString UMyGameInstance::PackageName = TEXT("/Game/Student");
const FString UMyGameInstance::AssetName = TEXT("TopStudent");

UMyGameInstance::UMyGameInstance()
{
	
}

void UMyGameInstance::Init()
{
	Super::Init();
	FStudentData RawDataSrc(11, TEXT("abc"));

	// 저장할 파일의 위치
	const FString SavedDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	UE_LOG(LogTemp, Log, TEXT("저장할 파일위치 : %s"), *SavedDir);

	{
		const FString RawDataFilename(TEXT("RawData.bin"));
		FString RawDataAbsolutePath = FPaths::Combine(*SavedDir, *RawDataFilename);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);

		FPaths::MakeStandardFilename(RawDataAbsolutePath);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);
		
		// Ar는 Archive라는 뜻 ㅎ
		// 전달해준 Path에 write할 수 있는 아카이브 객체 생성됨.
		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (RawFileWriterAr != nullptr)
		{
			*RawFileWriterAr << RawDataSrc;
			RawFileWriterAr->Close();
			delete RawFileWriterAr;
			RawFileWriterAr = nullptr;

			// UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDa);
		}
		FStudentData RawDataDest;
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (RawFileReaderAr != nullptr)
		{
			// << 연산자라 입력 같지만, Reader에 대한 << 연산자는 Reader에서 Dest로 쓴다.
			*RawFileReaderAr << RawDataDest;
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;

			UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDataDest.Name, RawDataDest.Order);
		}
	}

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("김동호"));
	StudentSrc->SetOrder(27);
	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin"));
		FString ObjcetDataAbsolutePath = FPaths::Combine(*SavedDir, *ObjectDataFileName);
		FPaths::MakeStandardFilename(ObjcetDataAbsolutePath);

		/*
		 * 버퍼를 만들어주고, 아카이브를 해당 버퍼에 연결해준다.
		 * 이후 아카이브를 Serialize함수에 전달하여 Student객체의 데이터가 BufferArray에 직렬화된 상태로 저장된다
		 * 그 다음 파일로 버퍼를 저장한다.
		 */
		TArray<uint8> BufferArray;
		FMemoryWriter MemoryWriterAr(BufferArray);
		StudentSrc->Serialize(MemoryWriterAr);

		{
			TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjcetDataAbsolutePath));
			if (FileWriterAr != nullptr)
			{
				*FileWriterAr << BufferArray;
				FileWriterAr->Close();
			}
		}
		
		TArray<uint8> BufferArrayFromFile;
		{
			TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjcetDataAbsolutePath));
			if (FileReaderAr != nullptr)
			{
				*FileReaderAr << BufferArrayFromFile;
				FileReaderAr->Close();
			}
		}

		FMemoryReader  MemoryReaderAr(BufferArrayFromFile);
		UStudent* StudentDest = NewObject<UStudent>();
		StudentDest->Serialize(MemoryReaderAr);

		StudentDest->PrintInfo(TEXT("ObjectData"));
	}

	{
		FString JsonDataFileName(TEXT("StudenJsonData.txt"));
		FString JsonDataAbsolutePath = FPaths::Combine(*SavedDir, *JsonDataFileName);
		FPaths::MakeStandardFilename(JsonDataAbsolutePath);

		// TSharedRef의 사용법. MakeShared()를 이용하여 초기화한다.
		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);

		FString JsonOutString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString);
		if (FJsonSerializer::Serialize(JsonObjectSrc, JsonWriterAr))
		{
			// 성공했다면 json이 만들어졌따.
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsolutePath);
		}

		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsolutePath);
		
		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString);
		//읽어지지 않으면 null이 올 수 있기 때문에 ref아닌 ptr 사용한다.
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest))
		{
			UStudent* JsonStudentDest = NewObject<UStudent>();
			if (FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				JsonStudentDest->PrintInfo(TEXT("JsonData"));
			}
		}
	}

	// 2. 패키지를 사용하기 위핸서는 패키지와 패키지에 담긴 애셋을 설정해야한다.
	SaveStudentPackage();

	
	LoadStudentPackage();
}

void UMyGameInstance::SaveStudentPackage() const
{
	// 12. 이미 패키지가 있다면, 로드를 디 끝내고 저장하는 것이 맞다.
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (StudentPackage)
	{
		StudentPackage->FullyLoad();
	}
	
	// 4. 이렇게 하면 패키지가 하나 만들어진다.
	StudentPackage = CreatePackage(*PackageName);
	// 5. 패키지를 저장하는 욥션
	constexpr EObjectFlags ObjectFlag = RF_Public | RF_Standalone;
	// 6. 패키지에 어떤 내용을 담을지 겵정.
	// 인자가 없는 경우는 Transient package라는 임시 패키지 안에 오브젝트가 저장된다.
	// 인자가 있는 경우는 인자로 전달된 패키지 안에 오브젝트가 들어간다.
	// 이후 필요한 옵션들을 추가한다.
	// 지금 껏 언리얼 오브젝트를 추가하던 방식보다 복잡하지만 이렇게 해야 안전하게 생성할 수 있다.
	UStudent* TopStudent = NewObject<UStudent>(StudentPackage, UStudent::StaticClass(), *AssetName, ObjectFlag);
	TopStudent->SetName("김동호");
	TopStudent->SetOrder(100);
	
	constexpr int NumOfSubObjects = 10;
	for (int32 i = 0 ; i < NumOfSubObjects ; i++)
	{
		FString SubObjectName = FString::Printf(TEXT("Student#%2d"), i+1);
		// 7. 이 경우에는 TopStudent의 하위로 들어가는 오브젝트이므로 첫 매개변수를 TopStudent로 한다.
		UStudent* SubStudent = NewObject<UStudent>(TopStudent, UStudent::StaticClass(), *SubObjectName, ObjectFlag);
		SubStudent->SetName( FString::Printf(TEXT("학생#%2d"), i+1));
		SubStudent->SetOrder(i+1);
	}

	// 8. 이제 이 패키지를 저장할텐데, 패키지를 저장할 경로와 패키지 저장 파일의 확장자를 결정해야한다.
	// GetAssetPackageExtension()에 의해서 최종적으로 .uasset이라는 확장자로 저장된다.
	const FString PackageFileName =FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = ObjectFlag;

	if (UPackage::SavePackage(StudentPackage, nullptr, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("패키지가 성공적으로 저장되었습니다."));
	}
}

void UMyGameInstance::LoadStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (nullptr == StudentPackage)
	{
		UE_LOG(LogTemp, Warning, TEXT("패키지를 불러올 수 없습니다."));
		return;
	}
	// 10. 로드에 성공하면 안에 가지고 있는 모든 오브젝트를 로드한다.
	StudentPackage->FullyLoad();
	// 11. 로드된 StudentPackage에서 애셋 네임(여기선 "TopStudent")를 찾아준다.
	UStudent* TopStudent = FindObject<UStudent>(StudentPackage, *AssetName);
	TopStudent->PrintInfo(TEXT("UE Object 찾음!"));
}
