/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/
//#define _WIN32

#ifdef _WIN32

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600
#else
#	if _WIN32_WINNT < 0x0600
#		error Unsupported platform
#	endif
#endif

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Input/Windows.h"

<<<<<<< HEAD
#include <memory>
=======
>>>>>>> e122ca3948a4b9b8ab6dbfd23d8926b3b9f9c45b
#include <thread>
#include <chrono>
#include <windows.h>
//#include <ShlObj.h>
//#include <Shobjidl.h>

<<<<<<< HEAD
//#ifndef MSWINDOWS_REFRESH_MSEC
//#   define MSWINDOWS_REFRESH_MSEC 50
//#endif

=======
>>>>>>> e122ca3948a4b9b8ab6dbfd23d8926b3b9f9c45b
using namespace Gwen;
using namespace Gwen::Platform;

static const size_t FileStringSize		= 2048;
static const size_t FilterBufferSize	= 512;

static Gwen::Input::Windows GwenInput;

static LPCTSTR iCursorConversion[] =
{
	IDC_ARROW,
	IDC_IBEAM,
	IDC_SIZENS,
	IDC_SIZEWE,
	IDC_SIZENWSE,
	IDC_SIZENESW,
	IDC_SIZEALL,
	IDC_NO,
	IDC_WAIT,
	IDC_HAND
};
<<<<<<< HEAD
//
//class WakeupThread
//{
//    public:
//
//    WakeupThread(HWND hwnd, int ms_refresh)
//        : m_hWindow(hwnd),m_msRefresh(ms_refresh){
//        if (m_msRefresh > 0) {
//        /* If there are no mouse/keyboard events, try to refresh the
//           view roughly every x ms; this is to support animations
//           such as progress bars while keeping the system load
//           reasonably low */
//            m_RefreshThread = std::thread(
//                [this]() {
//                    std::chrono::milliseconds time(this->m_msRefresh);
//                    while (this->m_msRefresh > 0) {
//                        std::this_thread::sleep_for(time);
//                        PostMessage(this->m_hWindow,WM_USER, 0,0);
//                    }
//                }
//            );
//        }
//    }
//
//    WakeupThread(WakeupThread&& other)
//        :m_hWindow(NULL),m_msRefresh(-1){
//          m_hWindow = other.m_hWindow; other.m_hWindow=NULL;
//          m_msRefresh = other.m_msRefresh; other.m_msRefresh=-1;
//          m_RefreshThread = other.m_RefreshThread;
//    }
//
//    WakeupThread& operator=(WakeupThread&& other){
//        if(this != &other){
//            m_hWindow = other.m_hWindow; other.m_hWindow=NULL;
//            m_msRefresh = other.m_msRefresh; other.m_msRefresh=-1;
//            m_RefreshThread = other.m_RefreshThread;
//        }
//        return *this;
//    }
//
//    ~WakeupThread() {
//        if (m_msRefresh > 0) {
//            m_msRefresh = 0;
//            m_RefreshThread.join();
//        }
//        m_hWindow = NULL;
//    };
//
//    int  getRefreshRate(){return m_msRefresh;};
//    bool isActive() const{return m_msRefresh > 0;};
//
//private:
//    HWND           m_hWindow;
//    int            m_msRefresh;
//    std::thread    m_pRefreshThread;
//};


//static std::map<HWND, WakeupThread> WakeupThreadList;
static std::map<HWND, std::thread> WakeupThreadList;
static std::map<HWND, int> WakeupParamList;

////////////////////////////////////////////////////////////////////////////////////////

void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle, int msecRefresh)
{
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	WNDCLASSA	wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.style			= CS_OWNDC | CS_DROPSHADOW;
	wc.lpfnWndProc		= DefWindowProc;
	wc.hInstance		= GetModuleHandle( NULL );
	wc.lpszClassName	= "GWEN_Window_Class";
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	RegisterClassA( &wc );
	HWND hWindow = CreateWindowExA( WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, wc.lpszClassName, strWindowTitle.c_str(), WS_POPUP | WS_VISIBLE, x, y, w, h, NULL, NULL, GetModuleHandle( NULL ), NULL );
	ShowWindow( hWindow, SW_SHOW );
	UpdateWindow(hWindow);
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );
	// Curve the corners
	{
		HRGN rgn = CreateRoundRectRgn( 0, 0, w + 1, h + 1, 4, 4 );
		SetWindowRgn( hWindow, rgn, false );
	}
    //create a thread to wake up the message que in the main thread for animation, etc;
    if (msecRefresh > 0) {
        auto pRefresh = &WakeupParamList.insert(std::pair<HWND,int>(hWindow,msecRefresh)).first->second;
        /* If there are no mouse/keyboard events, try to refresh the
           view roughly every x ms; this is to support animations
           such as progress bars while keeping the system load
           reasonably low */
            WakeupThreadList[hWindow] = std::thread(
                [pRefresh,hWindow]() {
                    std::chrono::milliseconds time(*pRefresh);
                    while (*pRefresh > 0) {
                        std::this_thread::sleep_for(time);
                        PostMessage(hWindow,WM_USER, 0,0);
                    }
                }
            );
        }
	return ( void* ) hWindow;
}

void Gwen::Platform::DestroyPlatformWindow( void* pWindow )
{
	//finish and destroy the wakeup thread, if any
	auto wakeupThread = WakeupThreadList.find(( HWND ) pWindow);
	if (wakeupThread != WakeupThreadList.end() and
        wakeupThread->second.joinable()) {
            WakeupParamList[( HWND ) pWindow]=0;//stop the associated thread's loop
            wakeupThread->second.join();//wait for thread to terminate
            //remove the list objects
            WakeupThreadList.erase(( HWND ) pWindow );
            WakeupParamList.erase(( HWND ) pWindow );
    }
	DestroyWindow( ( HWND ) pWindow );
	CoUninitialize();
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	GwenInput.Initialize( ptarget );
	MSG msg;
    BOOL bRet;
    bool peekMessages;

    auto wakeupThread = WakeupThreadList.find(( HWND ) pWindow);
    /* if we don't have a wake-up thread or it's not active then
     * we have to 'peek' instead of'wait' for messages*/
    peekMessages = true;//(wakeupThread == WakeupThreadList.end()) or (not wakeupThread->second.joinable());

	while ( true )//it breaks on error, quit or when only 'wake-up' messages occur
	{
		if(peekMessages){
            bRet = PeekMessage(&msg, ( HWND ) pWindow, 0, 0, PM_REMOVE);
		}
		else {
            bRet = GetMessage( &msg, ( HWND ) pWindow, 0, 0);
		}
        /*Get*/
		if(bRet == -1 or msg.message == WM_QUIT ){
            //time to quit
            ptarget->InputQuit();
            break;
		}
		if (/*Peek*/bRet == 0 or /*Get*/ msg.message == WM_USER ){
//			ptarget->Redraw();
			break;
		}
		if ( GwenInput.ProcessMessage( msg ) )
		{ continue; }

		if ( msg.message == WM_PAINT ){
			ptarget->Redraw();
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
		Sleep(50);//give away the control for a short while
	}

	// If the active window has changed then force a redraw of our canvas
	// since we might paint ourselves a different colour if we're inactive etc
	{
		static HWND g_LastFocus = NULL;

		if ( GetActiveWindow()  != g_LastFocus )
		{
			g_LastFocus = GetActiveWindow();
			ptarget->Redraw();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

=======
>>>>>>> e122ca3948a4b9b8ab6dbfd23d8926b3b9f9c45b

void Gwen::Platform::SetCursor( unsigned char iCursor )
{
	// Todo.. Properly.
	::SetCursor( LoadCursor( NULL, iCursorConversion[iCursor] ) );
}

void Gwen::Platform::GetCursorPos( Gwen::Point & po )
{
	POINT p;
	::GetCursorPos( &p );
	po.x = p.x;
	po.y = p.y;
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
	w = GetSystemMetrics( SM_CXFULLSCREEN );
	h = GetSystemMetrics( SM_CYFULLSCREEN );
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	if ( !OpenClipboard( NULL ) ) { return L""; }

	HANDLE hData = GetClipboardData( CF_UNICODETEXT );

	if ( hData == NULL )
	{
		CloseClipboard();
		return L"";
	}

	wchar_t* buffer = ( wchar_t* ) GlobalLock( hData );
	UnicodeString str = buffer;
	GlobalUnlock( hData );
	CloseClipboard();
	return str;
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
	if ( !OpenClipboard( NULL ) ) { return false; }

	EmptyClipboard();
	// Create a buffer to hold the string
	size_t iDataSize = ( str.length() + 1 ) * sizeof( wchar_t );
	HGLOBAL clipbuffer = GlobalAlloc( GMEM_DDESHARE, iDataSize );
	// Copy the string into the buffer
	wchar_t* buffer = ( wchar_t* ) GlobalLock( clipbuffer );
	wcscpy( buffer, str.c_str() );
	GlobalUnlock( clipbuffer );
	// Place it on the clipboard
	SetClipboardData( CF_UNICODETEXT, clipbuffer );
	CloseClipboard();
	return true;
}

double GetPerformanceFrequency()
{
	static double Frequency = 0.0f;

	if ( Frequency == 0.0f )
	{
		__int64 perfFreq;
		QueryPerformanceFrequency( ( LARGE_INTEGER* ) &perfFreq );
		Frequency = 1.0 / ( double ) perfFreq;
	}

	return Frequency;
}

float Gwen::Platform::GetTimeInSeconds()
{
	static float fCurrentTime = 0.0f;
	static __int64 iLastTime = 0;
	__int64 thistime;
	QueryPerformanceCounter( ( LARGE_INTEGER* ) &thistime );
	float fSecondsDifference = ( double )( thistime - iLastTime ) * GetPerformanceFrequency();

	if ( fSecondsDifference > 0.1f ) { fSecondsDifference = 0.1f; }

	fCurrentTime += fSecondsDifference;
	iLastTime = thistime;
	return fCurrentTime;
}



bool Gwen::Platform::FileOpen(const String & Name,
                              const String & StartPath,
                              const String & Extension,
                              List & sqFileName,
                              bool MultiSelect,
                              Gwen::Event::Handler* pHandler,
                              Event::Handler::FunctionWithInformation fnCallback )
{
	char Filestring[FileStringSize];

	char FilterBuffer[FilterBufferSize];
	{
		memset( FilterBuffer, 0, sizeof( FilterBuffer ) );
		memcpy( FilterBuffer, Extension.c_str(), Gwen::Min( Extension.length(), sizeof( FilterBuffer ) ) );

		for ( int i = 0; i < FilterBufferSize; i++ )
		{
			if ( FilterBuffer[i] == '|' )
			{ FilterBuffer[i] = 0; }
		}
	}
	OPENFILENAMEA opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = FilterBuffer;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = FileStringSize;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle = 50;
	opf.lpstrInitialDir = StartPath.c_str();
	opf.lpstrTitle = Name.c_str();
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = "*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR ) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof( OPENFILENAME );
	if(MultiSelect)
        opf.Flags = opf.Flags | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if ( GetOpenFileNameA( &opf ) )
	{
        sqFileName.clear();
        if(MultiSelect){//decode the Windows multi-select string pattern
            //in Explorer-type FileOpen, the folder path and all filenames are separated by '\0', with a double '\0' at the end
            auto *p = opf.lpstrFile;
            std::string fpath(p);//the file path
            p = strchr(p, '\0')+1;//beginning of first fname
            while (*p) {
                sqFileName.push_back(fpath+"\\"+std::string(p));
                p = strchr(p, '\0')+1;//go past the substring end
            }
        }
        if(sqFileName.empty())//only one file selected, no matter the selection flag
            sqFileName.push_back(opf.lpstrFile);

		if ( pHandler && fnCallback )
		{
			Gwen::Event::Information info;
			info.Control		= NULL;
			info.ControlCaller	= NULL;
			for(auto fpath : sqFileName){
                info.String	= fpath;
                ( pHandler->*fnCallback )( info );
			}
		}
	}

	return true;
}

bool Gwen::Platform::FolderOpen( const String & Name, const String & StartPath, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	bool bSuccess = false;
//	IFileDialog* pfd = NULL;
//
//#ifndef _MSC_VER
//    // The line below is from the Qt Toolkit "src/plugins/platforms/windows/qwindowsdialoghelpers.cpp"
//    static const CLSID CLSID_FileOpenDialog = {0xdc1c5a9c, 0xe88a, 0x4dde, {0xa5, 0xa1, 0x60, 0xf8, 0x2a, 0x20, 0xae, 0xf7}};
//#endif
//
//	if ( CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &pfd ) ) != S_OK )
//	{ return bSuccess; }
//
//	DWORD dwOptions;
//
//	if ( pfd->GetOptions( &dwOptions ) != S_OK )
//	{
//		pfd->Release();
//		return bSuccess;
//	}
//
//	pfd->SetOptions( dwOptions | FOS_PICKFOLDERS );
//
//	//
//	// TODO: SetDefaultFolder -> StartPath
//	//
//
//	if ( pfd->Show( NULL ) == S_OK )
//	{
//		IShellItem* psi;
//
//		if ( pfd->GetResult( &psi ) == S_OK )
//		{
//			WCHAR* strOut = NULL;
//
//			if ( psi->GetDisplayName( SIGDN_DESKTOPABSOLUTEPARSING, &strOut ) != S_OK )
//			{
//				return bSuccess;
//			}
//
//			//
//			// GWEN callback - call it.
//			//
//			if ( pHandler && fnCallback )
//			{
//				Gwen::Event::Information info;
//				info.Control		= NULL;
//				info.ControlCaller	= NULL;
//				info.String			= Gwen::Utility::UnicodeToString( strOut );
//				( pHandler->*fnCallback )( info );
//			}
//
//			CoTaskMemFree( strOut );
//			psi->Release();
//			bSuccess = true;
//		}
//	}
//
//	pfd->Release();
	return bSuccess;
}

bool Gwen::Platform::FileSave( const String & Name, const String & StartPath, const String & Extension, String & FileName, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionWithInformation fnCallback )
{
	char Filestring[FileStringSize];
	String returnstring;

	char FilterBuffer[FilterBufferSize];
	{
		memset( FilterBuffer, 0, sizeof( FilterBuffer ) );
		memcpy( FilterBuffer, Extension.c_str(), Gwen::Min( Extension.size(), sizeof( FilterBuffer ) ) );

		for ( int i = 0; i < FilterBufferSize; i++ )
		{
			if ( FilterBuffer[i] == '|' )
			{ FilterBuffer[i] = 0; }
		}
	}
	OPENFILENAMEA opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = FilterBuffer;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = FileStringSize;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle = 50;
	opf.lpstrInitialDir = StartPath.c_str();
	opf.lpstrTitle = Name.c_str();
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = "*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR ) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof( OPENFILENAME );

	if ( GetSaveFileNameA( &opf ) )
	{
		if ( pHandler && fnCallback )
		{
			Gwen::Event::Information info;
			info.Control		= NULL;
			info.ControlCaller	= NULL;
			info.String			= opf.lpstrFile;
			( pHandler->*fnCallback )( info );
		}
		FileName = opf.lpstrFile;
	}

	return true;
}


<<<<<<< HEAD
=======
void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle )
{
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	WNDCLASSA	wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.style			= CS_OWNDC | CS_DROPSHADOW;
	wc.lpfnWndProc		= DefWindowProc;
	wc.hInstance		= GetModuleHandle( NULL );
	wc.lpszClassName	= "GWEN_Window_Class";
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	RegisterClassA( &wc );
	HWND hWindow = CreateWindowExA( WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, wc.lpszClassName, strWindowTitle.c_str(), WS_POPUP | WS_VISIBLE, x, y, w, h, NULL, NULL, GetModuleHandle( NULL ), NULL );
	ShowWindow( hWindow, SW_SHOW );
	UpdateWindow(hWindow);
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );
	// Curve the corners
	{
		HRGN rgn = CreateRoundRectRgn( 0, 0, w + 1, h + 1, 4, 4 );
		SetWindowRgn( hWindow, rgn, false );
	}
	return ( void* ) hWindow;
}

void Gwen::Platform::DestroyPlatformWindow( void* pPtr )
{
	DestroyWindow( ( HWND ) pPtr );
	CoUninitialize();
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	GwenInput.Initialize( ptarget );
	MSG msg;
    BOOL bRet;
    int refresh = 50;
    bool wantsQuit = false;

//    std::thread refresh_thread;
//    if (refresh > 0) {
//        /* If there are no mouse/keyboard events, try to refresh the
//           view roughly every 50 ms (default); this is to support animations
//           such as progress bars while keeping the system load
//           reasonably low */
//        refresh_thread = std::thread(
//            [refresh, pWindow, wantsQuit]() {
//                std::chrono::milliseconds time(refresh);
//                while (not wantsQuit) {
//                    std::this_thread::sleep_for(time);
//                    PostMessage(( HWND ) pWindow,WM_USER, 0,0);
//                }
//            }
//        );
//    }

	while ( bRet = PeekMessage( &msg, ( HWND ) pWindow, 0, 0, PM_REMOVE ) )
	{
		if(bRet == -1){
            //something bad happened, just quit
            ptarget->InputQuit();
            wantsQuit = true;
            break;
		}
		if ( GwenInput.ProcessMessage( msg ) )
		{ continue; }

		if ( msg.message == WM_PAINT )
		{
			ptarget->Redraw();
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
		//give away the control for 1 ms
//		Sleep(50);
	}

	// If the active window has changed then force a redraw of our canvas
	// since we might paint ourselves a different colour if we're inactive etc
	{
		static HWND g_LastFocus = NULL;

		if ( GetActiveWindow()  != g_LastFocus )
		{
			g_LastFocus = GetActiveWindow();
			ptarget->Redraw();
		}
	}
}

>>>>>>> e122ca3948a4b9b8ab6dbfd23d8926b3b9f9c45b
void Gwen::Platform::SetBoundsPlatformWindow( void* pPtr, int x, int y, int w, int h )
{
	SetWindowPos( ( HWND ) pPtr, HWND_NOTOPMOST, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING );
	// Curve the corners
	{
		HRGN rgn = CreateRoundRectRgn( 0, 0, w + 1, h + 1, 4, 4 );
		SetWindowRgn( ( HWND ) pPtr, rgn, false );
	}
}

void Gwen::Platform::SetWindowMaximized( void* pPtr, bool bMax, Gwen::Point & pNewPos, Gwen::Point & pNewSize )
{
	if ( bMax )
	{
		ShowWindow( ( HWND ) pPtr, SW_SHOWMAXIMIZED );
		RECT rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 );  // size excluding task bar
		SetWindowPos( ( HWND ) pPtr, HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING );
		// Remove the corner curves
		{
			SetWindowRgn( ( HWND ) pPtr, NULL, false );
		}
	}
	else
	{
		ShowWindow( ( HWND ) pPtr, SW_RESTORE );
		// Curve the corners
		{
			RECT r;
			GetWindowRect( ( HWND ) pPtr, &r );
			HRGN rgn = CreateRoundRectRgn( 0, 0, ( r.right - r.left ) + 1, ( r.bottom - r.top ) + 1, 4, 4 );
			SetWindowRgn( ( HWND ) pPtr, rgn, false );
		}
	}

	RECT r;
	GetWindowRect( ( HWND ) pPtr, &r );
	pNewSize.x = r.right - r.left;
	pNewSize.y = r.bottom - r.top ;
	pNewPos.x = r.left;
	pNewPos.y = r.top;
}

void Gwen::Platform::SetWindowMinimized( void* pPtr, bool bMinimized )
{
	if ( bMinimized )
	{
		ShowWindow( ( HWND ) pPtr, SW_SHOWMINIMIZED );
	}
	else
	{
		ShowWindow( ( HWND ) pPtr, SW_RESTORE );
	}
}

bool Gwen::Platform::HasFocusPlatformWindow( void* pPtr )
{
	return GetActiveWindow() == ( HWND ) pPtr;
}

void Gwen::Platform::Sleep( unsigned int iMS )
{
	::Sleep( iMS );
}

#endif // WIN32
