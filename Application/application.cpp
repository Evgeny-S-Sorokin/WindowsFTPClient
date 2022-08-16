#include "application.h"

#include <string>
#include <limits>
#include <exception>

namespace sorokin
{
    Application::Application()
    {
        HMODULE hModule = ::GetModuleHandle( nullptr );

        if ( hModule != nullptr )
        {
            // initialize MFC and print and error on failure
            if ( !AfxWinInit( hModule, nullptr, ::GetCommandLine(), 0 ) )
            {
                throw std::runtime_error( "Fatal Error: MFC initialization failed.\n" );
            }
        }
        else
        {
            throw std::runtime_error( "Fatal Error: GetModuleHandle failed.\n" );
        }
    }

    bool Application::Run( int argv, char* argc[] )
    {
        std::string input_str;

        CString login;
        CString pwd;
        CString ip;
        INTERNET_PORT port = INTERNET_DEFAULT_FTP_PORT;
        BOOL passive_flag = FALSE;

        if ( argv > 1 )
        {
            // implement later maybe
        }
        else
        {
            login = EnterUserInfo( "login", false );
            pwd = EnterUserInfo( "password", false ); // password will be inserted without hiding for now
            ip = EnterUserInfo( "ip" );
            try
            {
                const auto& port_str = EnterUserInfo( "port", false );
                port = !port_str.GetLength() ? INTERNET_INVALID_PORT_NUMBER :
                    static_cast<INTERNET_PORT>( std::stoul( port_str.GetString() ) );
            }
            catch ( const std::exception& error )
            {
                std::cout << "Port input failed: [" << error.what() << "].\n";
                std::cout << "Port set to [" << port << "].\n";
            }
            try
            {
                const auto& flag_str = EnterUserInfo( "passive flag", false );
                passive_flag = !flag_str.GetLength() ? FALSE :
                    static_cast<BOOL>( std::stoi( flag_str.GetString() ) );
            }
            catch ( const std::exception& error )
            {
                std::cout << "Flag input failed: [" << error.what() << "].\n";
                std::cout << "Flag set to [" << passive_flag << "].\n";
            }
        }

        if ( !client_.Open( login, pwd, ip, port, passive_flag ) )
        {
            std::cout << "Can't connect.\n";
            return false;
        }

        std::cout << "Connected. Enter command to send to ftp server. Enter \"exit\" to exit.\n";
        while ( true )
        {
            CString command = EnterUserInfo( "command" );
            if ( !command.Compare( L"exit" ) )
            {
                return true;
            }

            if ( client_.ExecuteSimpleCommand( command ) )
            {
                std::cout << "Execute result:\n[" << client_.GetExecuteResult() << "].\n";
            }
        }

    }

    CString Application::EnterUserInfo( const std::string& name, bool required )
    {
        char input_buffer[kInputBufferSize] = { 0 };
        CString input_str;
        while ( !input_str.GetLength() )
        {
            std::cout << "Enter " << name << ": ";
            std::cin.clear();
            std::cin.getline( input_buffer, kInputBufferSize - 1, '\n');
            input_str = input_buffer;
            if ( !input_str.GetLength() )
            {
                if ( !required )
                {
                    break;
                }
                std::cout << "Empty " << name << " entered.\n";
            }
        }

        return input_str;
    }

} // namespace sorokin