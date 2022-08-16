#include "windows_ftp_client.h"

#include <string>
#include <cstring>
#include <exception>

namespace sorokin
{
    bool WindowsFtpClient::Open( CString login, CString pwd,
        CString ip, INTERNET_PORT port,
        BOOL passive_flag )
    {
        std::wcout << "Trying to connect to [" << 
            static_cast<const wchar_t*>( ip ) << ":" << port << "].\n";
        std::wcout << "Login [" << static_cast<const wchar_t*>( login ) << "].\n";
        std::cout << "...\n";
        try
        {
            pSession_.reset( new CInternetSession );
            std::cout << "Connection established.\n...\n";
            pConnection_.reset( pSession_->GetFtpConnection( ip, login, pwd, port, passive_flag ) );
            std::cout << "Ftp protocol established.\n";
            return true;
        }
        catch ( CInternetException* error )
        {
            ProcessError( error );
            error->Delete();
        }
        catch ( ... )
        {
            std::cout << "Caught unknown exception on connection!\n";
        }

        return false;
    }

    bool WindowsFtpClient::ExecuteSimpleCommand( const wchar_t* command )
    {
        if ( !IsOpen() )
        {
            std::cout << "No connection established.\n";
            return false;
        }

        std::wcout << "Executing command [" << command << "].\n";
        auto connection_handler = static_cast<HINTERNET>( *( pConnection_.get() ) );
        decltype(connection_handler) answer_handler = nullptr;
        auto context = pSession_->GetContext();
        if ( !FtpCommand( connection_handler, FALSE, 
            FTP_TRANSFER_TYPE_BINARY, command, context, &answer_handler ) )
        {
            std::cout << "Failed to execute command. Error: [" << GetLastError() << "].\n";
            return false;
        }

        return true;
    }

    DWORD WindowsFtpClient::GetExecuteResult()
    {
        DWORD error = 0;
        DWORD size = 0;
        DWORD code = 0;

        InternetGetLastResponseInfo( &error, NULL, &size );

        if ( size != 0 )
        {
            std::unique_ptr<wchar_t[]> psz( new wchar_t[size] );
            while ( InternetGetLastResponseInfo( &error, psz.get(), &size ) )
            {
                code = std::stoul( std::wstring( psz.get() ) );
            }
        }

        return code;
    }

    void WindowsFtpClient::ProcessError( const CInternetException* const error )
    {
        TCHAR local_buffer[kErrorBufferSize] = { 0 };
        error->GetErrorMessage( local_buffer, kErrorBufferSize );
        std::wcout << "Caught error on connection: [" <<
            static_cast<const wchar_t*>( local_buffer ) << "].\n";
    }
} // namespace sorokin