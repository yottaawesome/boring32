//#include <stdio.h>
//
//import std;
//import std.compat;
//import <win32.hpp>; 
//
//#include "Boring32.Tests.h"
//
//import boring32.winsock;
//import boring32.async;
//import boring32.strings;
//import boring32.com;
//import boring32.compression;
//import boring32.process;
//import boring32.util;
//import boring32.datastructures;
//import boring32.registry;
//import boring32.taskscheduler;
//import boring32.error;
//import boring32.ipc;
//import boring32.security;
//import boring32.crypto;
//import boring32.winhttp;
//
//#pragma comment(lib, "Pathcch.lib")
//
//void print_exception_info(const std::exception& e)
//{
//	std::wcout << e.what() << std::endl;
//	try
//	{
//		rethrow_if_nested(e);
//	}
//	catch (const std::exception& ne)
//	{
//		print_exception_info(ne);
//	}
//}
//
//void TestException()
//{
//	try
//	{
//		throw std::logic_error("first");
//	}
//	catch (const std::exception& ex)
//	{
//		try
//		{
//			std::throw_with_nested(std::logic_error("second"));
//		}
//		catch (const std::exception& ex2)
//		{
//			print_exception_info(ex);
//		}
//	}
//}
//
//void TestWaitableTime(int64_t relativeMillis)
//{
//	Boring32::Async::WaitableTimer timer1(L"WaitableTimer", false, false);
//	Boring32::Async::WaitableTimer timer2(L"WaitableTimer", false, false, SYNCHRONIZE);
//	std::wcout << L"Timer set for " << relativeMillis << L" from now" << std::endl;
//	timer1.SetTimerInMillis(-relativeMillis, relativeMillis, nullptr, nullptr);
//	timer1.WaitOnTimer(INFINITE, false);
//	timer1.CancelTimer();
//}
//
//void TestConversions()
//{
//	std::wstring wstrTest = L"Test1";
//	std::string strTest = Boring32::Strings::ConvertString(wstrTest);
//	std::wstring wstrTest2 = Boring32::Strings::ConvertString(strTest);
//
//	if (wstrTest != wstrTest2)
//		throw std::runtime_error("Strings do not match");
//}
//
//void TestMemoryMappedFile()
//{
//	Boring32::Async::MemoryMappedFile m1(L"HelloMmf1", 1000, false);
//	Boring32::Async::MemoryMappedFile m2(m1);
//	Boring32::Async::MemoryMappedFile m3(L"HelloMmf3", 1000, false);
//	m2 = m3;
//}
//
//void TestLibraryLoad()
//{
//	Boring32::Process::DynamicLinkLibrary d(L"wlanapi.dll");
//	//void* mainFunc = d.Resolve(L"GetMainInterface");
//	//if (mainFunc == nullptr)
//	//	throw new std::runtime_error("Could not resolve function");
//}
//
//void TestThreadSafeVector()
//{
//	Boring32::Async::ThreadSafeVector<int> testVector;
//	testVector.Add(1);
//	testVector.Add(2);
//	testVector.Add(1);
//	testVector.Add(1);
//	testVector.Add(5);
//	// Erase all 1s
//	testVector.EraseMultiple(
//		[](const int& val) -> bool
//		{
//			return val == 1;
//		});
//	if (testVector.Size() != 2)
//		throw std::runtime_error("Unexpected size of vector");
//	if (testVector.CopyOfElementAt(0) != 2)
//		throw std::runtime_error("Unexpected element at index 0");
//}
//
//void TestProcessBlockingNamedPipe()
//{
//	std::wstring directory;
//	directory.resize(2048);
//	GetModuleFileNameW(nullptr, &directory[0], directory.size());
//	PathCchRemoveFileSpec(&directory[0], directory.size());
//	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
//	std::wstring filePath = directory + L"\\TestProcess.exe";
//
//	Boring32::Async::Job job(false);
//	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{ 0 };
//	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
//	job.SetInformation(jeli);
//	std::wstringstream ss;
//	ss << "TestProcess.exe"
//		<< L" 1";
//	Boring32::Async::Process testProcess(filePath, ss.str(), directory, true);
//	testProcess.Start();
//	job.AssignProcessToThisJob(testProcess.GetProcessHandle());
//
//	Boring32::IPC::BlockingNamedPipeServer pipeServer(
//		L"\\\\.\\pipe\\mynamedpipe",
//		200,
//		5,
//		L"",
//		false,
//		true
//	);
//	pipeServer.Connect();
//	pipeServer.Write(L"HAHA!");
//	pipeServer.Write(L"HAHA2!");
//	WaitForSingleObject(testProcess.GetProcessHandle(), INFINITE);
//	pipeServer.Disconnect();
//}
//
//void TestProcessOverlappedNamedPipe()
//{
//	std::wstring directory;
//	directory.resize(2048);
//	GetModuleFileName(nullptr, &directory[0], directory.size());
//	PathCchRemoveFileSpec(&directory[0], directory.size());
//	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
//	std::wstring filePath = directory + L"\\TestProcess.exe";
//
//	Boring32::IPC::OverlappedNamedPipeServer serverPipe(
//		L"\\\\.\\pipe\\mynamedpipe",
//		1024,
//		5,
//		L"",
//		false,
//		true
//	);
//	Boring32::Async::OverlappedOp oio;
//	serverPipe.Connect(oio);
//
//	Boring32::Async::Job job(false);
//	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{ 0 };
//	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
//	job.SetInformation(jeli);
//	std::wstringstream ss;
//	ss << "TestProcess.exe"
//		<< L" 2";
//	Boring32::Async::Process testProcess(filePath, ss.str(), directory, true);
//	testProcess.Start();
//	job.AssignProcessToThisJob(testProcess.GetProcessHandle());
//
//	oio.WaitForCompletion(INFINITE);
//	Boring32::Async::OverlappedIo writeOp;
//	serverPipe.Write(L"HAHA!", writeOp);
//	WaitForSingleObject(testProcess.GetProcessHandle(), INFINITE);
//
//	Boring32::Async::OverlappedIo writeOp2;
//	serverPipe.Read(1024, writeOp2);
//
//
//	writeOp2.WaitForCompletion(INFINITE);
//	std::wcout << Boring32::Util::ByteVectorToString<std::wstring>(writeOp2.IoBuffer) << std::endl;
//	serverPipe.Read(1024, writeOp2);
//	writeOp2.WaitForCompletion(INFINITE);
//	std::wcout << Boring32::Util::ByteVectorToString<std::wstring>(writeOp2.IoBuffer) << std::endl;
//}
//
//void TestCompression()
//{
//	Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
//	Boring32::Compression::Decompressor decompressor(Boring32::Compression::CompressionType::MSZIP);
//}
//
//void TestTimerQueues()
//{
//	std::wcout << L"Testing timer queues..." << std::endl;
//	Boring32::Async::Event eventToWaitOn(false, true, false, L"");
//	Boring32::Async::TimerQueue timerQueue;
//	Boring32::Async::TimerQueueTimer timerQueueTimer(
//		timerQueue.GetHandle(),
//		3000,
//		0,
//		0,
//		[](void* lpParam, BOOLEAN TimerOrWaitFired) -> void
//		{
//			Boring32::Async::Event* eventObj = (Boring32::Async::Event*)lpParam;
//			eventObj->Signal();
//		},
//		&eventToWaitOn
//	);
//
//	eventToWaitOn.WaitOnEvent(INFINITE, true);
//	std::wcout << L"Timer Queue test OK" << std::endl;
//}
//
//void WebSocket()
//{
//	try
//	{
//		// Check https://github.com/julie-ng/nodejs-certificate-auth
//		// Follow the instructions in https://gist.github.com/pcan/e384fcad2a83e3ce20f9a4c33f4a13ae
//		// to generate all required files
//		// Check https://security.stackexchange.com/questions/25996/how-to-import-a-private-key-in-windows
//		// to create a pkcs file that can imported into MMC Certificates snap in
//		// Use the command below
//		// (winpty if in bash) openssl pkcs12 -export -out cert.pfx -inkey client1-key.pem -in client1-crt.pem -certfile ca-crt.pem
//		// Check https://www.sonicwall.com/support/knowledge-base/how-can-i-import-certificates-into-the-ms-windows-local-machine-certificate-store/170504615105398/
//		// to import the CA cert into Trusted Root Certification Authorities
//		// and to import the cert.pfx file into Personal \ Certificates
//		// Note that you should import into the current user store, not system store
//		// All the cert files are in my personal cloud drive under Programming >
//		// ws-client-auth-certs and a node server file is in socket.js
//		// Basically, the process is create CA keys and cert, create server
//		// keys and cert, create client keys and cert, create pfc file, import
//		// root ca into Windows user Trusted Root Certification Authorities,
//		// import client pfx file into personal user store, load cert, set cert
//		// context for winhttp and then connect
//
//
//		// ERROR_WINHTTP_CLIENT_CERT_NO_PRIVATE_KEY
//		Boring32::Crypto::CertStore certStore(L"MY");
//		Boring32::Crypto::Certificate clientCert =
//			certStore.GetCertByExactSubject(L"CN = client.localhost");
//		if (clientCert == nullptr)
//		{
//			std::wcerr << L"Failed" << std::endl;
//			return;
//		}
//
//		Boring32::WinHttp::WebSockets::WebSocket socket(
//			Boring32::WinHttp::WebSockets::WebSocketSettings{
//				.Server = L"127.0.0.1",
//				.Port = 8000,
//				.IgnoreSslErrors = true,
//				.WinHttpSession =
//					Boring32::WinHttp::Session(L"testUserAgent"),
//				.ClientCert = clientCert
//			}
//		);
//		socket.Connect();
//		std::vector<char> buffer;
//		socket.Receive(buffer);
//		std::cout << std::string(buffer.begin(), buffer.end()) << std::endl;
//	}
//	catch (const std::exception& ex)
//	{
//		std::wcerr << ex.what() << std::endl;
//	}
//}
//
//void CertStoreOpen()
//{
//	HCERTSTORE store = CertOpenStore(
//		CERT_STORE_PROV_SYSTEM_REGISTRY_W,
//		PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
//		0,
//		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE,
//		L"MY"
//	);
//
//	if (store == nullptr)
//	{
//		std::wcout << "Failed to load store" << std::endl;
//		return;
//	}
//
//	Boring32::Crypto::CertStore certStore(
//		store,
//		Boring32::Crypto::CertStoreType::System,
//		true
//	);
//	Boring32::Crypto::Certificate clientCert = certStore.GetCertByExactSubject(L"CN = NVIDIA GameStream Server");
//	if (clientCert == nullptr)
//	{
//		std::wcerr << L"Failed" << std::endl;
//	}
//}
//
//void CheckConcat(std::vector<std::wstring>& m_acceptTypes)
//{
//	std::unique_ptr<LPCWSTR[]> acceptHeader = nullptr;
//	acceptHeader = std::unique_ptr<LPCWSTR[]>(new LPCWSTR[m_acceptTypes.size() + 1]);
//	for (int i = 0; i < m_acceptTypes.size(); i++)
//		acceptHeader[i] = m_acceptTypes.at(i).c_str();
//	acceptHeader[m_acceptTypes.size()] = nullptr;
//
//	std::vector<LPCWSTR> acceptHeader2(m_acceptTypes.size() + 1);
//	for (int i = 0; i < m_acceptTypes.size(); i++)
//		acceptHeader2[i] = m_acceptTypes.at(i).c_str();
//	acceptHeader2.back() = nullptr;
//}
//
//int OldJunk()
//{
//	Boring32::DataStructures::CappedStack<int> i(2, true);
//	i.Push(1);
//	i.Push(1);
//	//i.Push(3);
//	//i.Push(4);
//	//std::wcout << "Popped: " << i.Pop() << std::endl;
//	for (const int c : i.GetContainer())
//	{
//		std::wcout << c << std::endl;
//	}
//
//
//	//WebSocket();
//	//CertStoreOpen();
//	return 0;
//
//	/*try
//	{
//		Boring32::IPC::OverlappedNamedPipeServer server(L"A", 512, 1, L"", false, true);
//		auto a = server.Connect();
//		Boring32::IPC::OverlappedNamedPipeClient client(L"A");
//		client.Connect(0);
//		client.CloseSocket();
//		auto b = server.Read(0);
//		b.WaitForCompletion(INFINITE);
//		std::wcout << b.IsSuccessful() << std::endl;
//	}
//	catch (const std::exception& ex)
//	{
//		std::wcout << ex.what() << std::endl;
//	}*/
//
//	PROCESS_MEMORY_EXHAUSTION_INFO pmei{ 0 };
//	pmei.Version = PME_CURRENT_VERSION;
//	pmei.Type = PMETypeFailFastOnCommitFailure;
//	pmei.Value = PME_FAILFAST_ON_COMMIT_FAIL_ENABLE;
//	bool succeeded = SetProcessInformation(
//		GetCurrentProcess(),
//		ProcessMemoryExhaustionInfo,
//		&pmei,
//		sizeof(pmei)
//	);
//
//	try
//	{
//		Boring32::Async::ThreadPools::ThreadPool tp1(1, 10);
//
//		// https://spys.one/en/https-ssl-proxy/
//		//Test proxy: L"185.20.224.239:3128"
//		//Test proxy: L"45.76.52.195:8081"
//		//Test proxy: L"36.67.96.217:3128"
//		//Test proxy: L"36.82.95.67:3128"
//		//Test proxy: 125.164.86.89:3128
//		/*Boring32::WinHttp::HttpWebClient webClient(
//			L"test-ua",
//			L"google.com",
//			L"36.90.12.80:8181",
//			443,
//			false,
//			{L"text/html"},
//			{L""}
//		);
//		webClient.Connect();
//		Boring32::WinHttp::HttpRequestResult result = webClient.Get(L"/");
//		std::wcout << result.StatusCode << std::endl;
//		std::wcout << result.ResponseBody.c_str() << std::endl;
//		*/
//		Boring32::WinHttp::WebSockets::WebSocket socket(
//			Boring32::WinHttp::WebSockets::WebSocketSettings{
//				.Server = L"echo.websocket.org",
//				.Port = 443,
//				.IgnoreSslErrors = false,
//				.WinHttpSession =
//					Boring32::WinHttp::Session(L"testUserAgent")
//			// Uncomment to use named proxy
//			//Boring32::WinHttp::Session(L"testUserAgent", L"125.164.86.89:3128")
//			}
//		);
//		socket.Connect();
//		std::vector<char> buffer;
//		socket.SendString("Hello!");
//		socket.Receive(buffer);
//		std::string response(buffer.begin(), buffer.end());
//		std::wcout << response.c_str() << std::endl;
//	}
//	catch (const std::exception& ex)
//	{
//		std::wcout << ex.what() << std::endl;
//	}
//
//	std::wcout << Boring32::Util::GetCurrentExecutableDirectory() << std::endl;
//
//	//TestProcessNamedPipe();
//	for (int i = 0; i < 14; i++)
//	{
//		try
//		{
//			std::wcout << L"Test: " << i << std::endl;
//			if (i == 0)
//				TestProcessOverlappedNamedPipe();
//			if (i == 1)
//				TestThreadSafeVector();
//			if (i == 2)
//				TestException();
//			if (i == 3)
//				TestWaitableTime(2000);
//			if (i == 6)
//				TestConversions();
//			if (i == 7)
//				TestMemoryMappedFile();
//			if (i == 9)
//				TestLibraryLoad();
//			if (i == 10)
//				TestProcessBlockingNamedPipe();
//			if (i == 12)
//				TestCompression();
//			if (i == 13)
//				TestTimerQueues();
//		}
//		catch (const std::exception& ex)
//		{
//			std::wcout << ex.what() << std::endl;
//		}
//	}
//
//	//Boring32::WinHttp::HttpWebClient client(
//	//	L"TestClientAgent", 
//	//	L"127.0.0.1", 
//	//	94873, 
//	//	true, 
//	//	{ L"application/json" },
//	//	L"Content-Type: application/json"
//	//);
//	//client.Post(L"/some/object", "");
//
//	return 0;
//}
//
//void TestTaskService()
//{
//	try
//	{
//		Boring32::COM::COMThreadScope scope;
//		scope.Initialise();
//		scope.InitialiseSecurity();
//
//		Boring32::TaskScheduler::TaskService taskService;
//		taskService.Connect();
//		Boring32::TaskScheduler::TaskFolder rootFolder = taskService.GetFolder(L"\\");
//		std::optional<Boring32::TaskScheduler::RegisteredTask> tasks =
//			rootFolder.GetTask(L"Time Trigger Test Task");
//		if (tasks)
//		{
//			std::wcout << L"Found!" << std::endl;
//			tasks.value().SetRandomDelay(120);
//			//tasks.value().SetRepetitionInterval(15);
//			//tasks.value().SetEnabled(true);
//			//tasks.value().Run();
//			rootFolder.SaveOrUpdate(tasks.value(), TASK_LOGON_TYPE::TASK_LOGON_NONE);
//			//tasks.value().SetEnabled(true);
//		}
//	}
//	catch (const std::exception& ex)
//	{
//		std::wcerr << ex.what() << std::endl;
//	}
//}
//
//void TestCertImport()
//{
//	Boring32::Crypto::CertStore personal(L"MY");
//	Boring32::Crypto::CertStore system(L"MY",
//		Boring32::Crypto::CertStoreType::System);
//	auto cert = personal.GetCertBySubstringSubject(L"client.localhost");
//	system.ImportCert((CERT_CONTEXT*)cert.GetCert());
//}
//
//void TestCertName()
//{
//	Boring32::Crypto::CertStore personal(L"MY");
//	auto cert = personal.GetCertBySubstringSubject(L"client.localhost");
//	std::wcout << cert.GetFormattedSubject(CERT_X500_NAME_STR) << std::endl;
//}
//
//void TestCertFileImport()
//{
//	Boring32::Crypto::CertStore personal(L"", Boring32::Crypto::CertStoreType::InMemory);
//	personal.ImportCertsFromFile(
//		L"blah\\blah.pfx",
//		L"blah"
//	);
//	Boring32::Crypto::Certificate rootCa =
//		personal.GetCertBySubjectCn(L"blahblahblah");
//	if ((bool)rootCa == false)
//	{
//		std::wcout << L"Failed to find root CA" << std::endl;
//		return;
//	}
//
//	Boring32::Crypto::CertStore root(
//		L"ROOT",
//		Boring32::Crypto::CertStoreType::CurrentUser
//	);
//	//root.ImportCert(rootCa.GetCert());
//	root.AddCertificate(rootCa.GetCert(), Boring32::Crypto::CertAddDisposition::AddNewer);
//	std::wcout << L"Finished successfully" << std::endl;
//}
//
//void DeleteCert()
//{
//	Boring32::Crypto::CertStore personal(L"", Boring32::Crypto::CertStoreType::InMemory);
//	personal.ImportCertsFromFile(
//		L"blah\\blah.pfx",
//		L"blah"
//	);
//	Boring32::Crypto::Certificate rootCa =
//		personal.GetCertBySubjectCn(L"blahblahblah");
//
//	Boring32::Crypto::CertStore root(
//		L"MY",
//		Boring32::Crypto::CertStoreType::System
//	);
//	root.ImportCert(rootCa.GetCert());
//	rootCa =
//		root.GetCertBySubjectCn(L"blahblahblah");
//	root.DeleteCert(rootCa.GetCert());
//}
//
//void TestCertGetByThumbprint()
//{
//	Boring32::Crypto::CertStore personal(L"MY");
//	auto x = personal.GetCertBySubjectCn(L"client.localhost");
//	if (x)
//	{
//		auto vec = x.GetSubject();
//		std::wcout
//			<< x.GetSignature()
//			<< std::endl
//			<< x.GetFormattedSubject(CERT_X500_NAME_STR)
//			<< std::endl
//			<< x.GetFormattedSubject(CERT_SIMPLE_NAME_STR)
//			<< std::endl
//			//<< x._GetSubjectName()
//			<< std::endl
//			<< x.GetFormattedIssuer(CERT_X500_NAME_STR)
//			<< std::endl
//			<< x.GetSignatureHashCngAlgorithm()
//			<< std::endl
//			//<< std::string((char*)&vec[0], vec.size()).c_str()
//			//<< std::endl
//			//<< vec.size()
//			;
//
//		auto y =
//			personal.GetCertByExactSubject(x.GetFormattedSubject(CERT_X500_NAME_STR));
//		//personal.GetCertByFormattedSubject(x.GetFormattedSubject(CERT_X500_NAME_STR));
//		if (y)
//		{
//			std::wcout
//				<< "OK!!!!!!!!!!!!!!!!"
//				<< std::endl;
//		}
//	}
//}
//
//void TestSyncWebSocket()
//{
//	Boring32::WinHttp::WebSockets::WebSocket socket(
//		Boring32::WinHttp::WebSockets::WebSocketSettings{
//			.UserAgent = L"Test-WinHttp-Client",
//			.Server = L"127.0.0.1",
//			.Port = 51935,
//			.IgnoreSslErrors = true,
//			.WinHttpSession = Boring32::WinHttp::Session(L"testUserAgent")
//		}
//	);
//
//	using WsrrPtr = std::shared_ptr<Boring32::WinHttp::WebSockets::WebSocket::ReadResult>;
//
//	socket.Connect();
//	WsrrPtr readResult = socket.AsyncReceive();
//	socket.SendString("echo");
//	readResult->Done.WaitOnEvent(INFINITE, true);
//	if (readResult->Succeeded)
//	{
//		std::string message(readResult->Buffer.begin(), readResult->Buffer.end());
//		std::wcout << message.c_str() << std::endl;
//	}
//	else
//	{
//		std::wcout << "The read operation failed" << std::endl;
//	}
//}
//
//void X(PTP_CALLBACK_INSTANCE Instance,
//	PVOID                 Context,
//	PTP_WORK              Work)
//{
//	std::wcout << L"Hello" << std::endl;
//}
//
//void Threadpool()
//{
//	if (PTP_WORK work = CreateThreadpoolWork(X, nullptr, nullptr)) 
//	{
//		SubmitThreadpoolWork(work);
//	}
//	else
//	{
//		std::wcout << L"CreateThreadpoolWork() failed" << std::endl;
//	}
//}
//
//void TestAsyncWebSocket()
//{
//	//Boring32::WinHttp::WebSockets::WebSocket socket1(
//	//	Boring32::WinHttp::WebSockets::WebSocketSettings{
//	//		.Server = L"echo.websocket.org",
//	//		.Port = 443,
//	//		.IgnoreSslErrors = false,
//	//		.WinHttpSession =
//	//			Boring32::WinHttp::Session(L"testUserAgent")
//	//	// Uncomment to use named proxy
//	//	//Boring32::WinHttp::Session(L"testUserAgent", L"125.164.86.89:3128")
//	//	}
//	//);
//	//socket1.Connect();
//	//std::vector<char> buffer;
//	//socket1.SendString("Hello!");
//	//socket1.Receive(buffer);
//	//std::string response(buffer.begin(), buffer.end());
//	//std::wcout << response.c_str() << std::endl;
//
//	Boring32::WinHttp::WebSockets::AsyncWebSocket socket(
//		Boring32::WinHttp::WebSockets::AsyncWebSocketSettings{
//			.UserAgent = L"Test-WinHttp-Client",
//			.Server = L"127.0.0.1",
//			.Port = 51935,
//			.IgnoreSslErrors = true
//			// Uncomment to use named proxy
//			//Boring32::WinHttp::Session(L"testUserAgent", L"125.164.86.89:3128")
//		}
//	);
//
//	auto& connectionStatus = socket.Connect();
//	socket.GetConnectionStatus().Complete.WaitOnEvent();
//	if (not connectionStatus.IsConnected)
//	{
//		std::wcerr << L"Failed to connect\n";
//		return;
//	}
//
//	std::wcout << L"Connected successfully" << std::endl;
//	socket.SendString("Hello!");
//	//socket.CloseSocket();
//	//Sleep(5000);
//	auto& result = socket.Receive();
//	result.Complete.WaitOnEvent((DWORD)INFINITE, false);
//
//
//	//auto result = socket.Receive2().get();
//	//auto result = socket.Receive3()->get_future().get();
//	std::string message(result.Data.begin(), result.Data.end());
//	std::wcout << message.size() << std::endl;
//	std::wcout << message.c_str() << std::endl;
//
//	/*Boring32::WinHttp::WebSockets::WebSocketReadResult& result2 = socket.Receive();
//	result2.Complete.WaitOnEvent(INFINITE, false);
//	std::string message2(result2.Data.begin(), result2.Data.end());
//	std::wcout << message2.size() << std::endl;
//	std::wcout << message2.c_str() << std::endl;*/
//
//	Sleep(5000);
//
//	/*std::vector<char> buffer;
//	socket.SendString("Hello!");
//	socket.Receive(buffer);
//	std::string response(buffer.begin(), buffer.end());
//	std::wcout << response.c_str() << std::endl;*/
//}
//
//void Subauths()
//{
//	const std::vector<DWORD> subauths{
//			SECURITY_BUILTIN_DOMAIN_RID,
//			DOMAIN_ALIAS_RID_ADMINS
//	};
//	Boring32::Security::SecurityIdentifier sid(
//		SECURITY_NT_AUTHORITY,
//		subauths
//	);
//	//TestAsyncWebSocket();
//}
//
//void SingleList()
//{
//	struct A
//	{
//		A(int x) :X(x) {}
//		int X;
//	};
//
//	Boring32::DataStructures::SinglyLinkedList<A> test;
//	test.Add(7);
//	test.Add(12);
//	//auto i = test.GetAt(1);
//	//std::wcout << i->X << std::endl;
//	std::wcout << test.Pop()->X << std::endl;
//	std::wcout << test.Pop()->X << std::endl;
//}
//
//void UserApc()
//{
//	QueueUserAPC([](ULONG_PTR ptr) {}, GetCurrentThread(), 0);
//	DWORD dumb = 0;
//	Boring32::Registry::GetValue(HKEY_LOCAL_MACHINE, L"", dumb);
//	SingleList();
//}
//
//// see https://en.cppreference.com/w/cpp/types/enable_if
//template<typename X>
//struct T
//{
//	enum { int_t, float_t } type;
//	template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
//	T(Integer) : type(int_t) {}
//
//	template <typename Floating, std::enable_if_t<std::is_floating_point<Floating>::value, bool> = true>
//	T(Floating) : type(float_t) {}
//
//	template <typename Stuff, std::enable_if_t<std::is_class<Stuff>::value, bool> = true>
//	void Blah(Stuff* s)
//	{
//	}
//
//	template <typename Stuff, std::enable_if_t<std::is_integral<Stuff>::value, bool> = true>
//	void Blah(Stuff s)
//	{
//	}
//
//	template <typename A = X, std::enable_if<std::is_integral<A>::value, bool>::type = true>
//	int Blah2()
//	{
//		return 1;
//	}
//
//	template <typename A=X> requires std::is_integral<A>::value
//	int Blah3()
//	{
//		return 1;
//	}
//
//	template <typename M = X, std::enable_if<std::is_class<M>::value, bool>::type = true>
//	void Blah2()
//	{
//
//	}
//
//	template <typename M = X, std::enable_if<true, bool>::type = true>
//	void BoolType()
//	{
//
//	}
//};
//class V {};
//
//void Stuff()
//{
//	T<int> s(5);
//	s.Blah(&s);
//	s.Blah(1);
//	s.Blah2();
//	s.BoolType();
//	s.Blah3();
//
//	T<V> f(5);
//	f.Blah2();
//
//	Boring32::WinSock::WinSockInit init;
//	/*std::vector<Boring32::WinSock::NetworkingAddress> names = Boring32::WinSock::Resolve(L"www.googledfsdf.com");
//	for (const Boring32::WinSock::NetworkingAddress& x : names)
//	{
//		std::cout << x << std::endl;
//	}*/
//
//	std::vector<Boring32::WinSock::NetworkingAddress> names =
//		Boring32::WinSock::Resolve2(L"www.google.com");
//	for (const Boring32::WinSock::NetworkingAddress& x : names)
//	{
//		std::cout << x << std::endl;
//	}
//}
//
//int Experiments1Main()
//{
//	TestSyncWebSocket();
//
//	return 0;
//}