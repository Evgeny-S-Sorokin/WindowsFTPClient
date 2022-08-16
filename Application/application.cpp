#include "application.h"

#include <string>
#include <cstring>
#include <limits>
#include <exception>
#include <fstream>

extern "C" 
{
    #include "cJSON.h"
}
using pCJson = std::unique_ptr<cJSON, decltype( &cJSON_Delete )>;

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

        std::cout << "Reading [" << kCommandsLocation << "].\n";
        const auto json = ReadFile( kCommandsLocation );
        if ( json.empty() )
        {
            std::cout << "Failed to read [" << kCommandsLocation << "].\n";
        }
        const auto expect_successful_commands = FindCommands( json, "expected_true" );
        const auto expect_failed_commands = FindCommands( json, "expected_false" );

        std::cout << '\n' << '\n';
        for ( const auto& command : expect_successful_commands )
        {
            if ( !client_.ExecuteSimpleCommand( command.c_str() ) )
            {
                std::wcout << "Test [" << command << "] failed.\n";
                continue;
            }

            if ( !CheckExecuteCodeSuccession( client_.GetExecuteResult() ) )
            {
                std::wcout << "Test [" << command << "] failed.\n";
                continue;
            }

            std::wcout << "Test [" << command << "] successful.\n";
        }

        for ( const auto& command : expect_failed_commands )
        {
            if ( client_.ExecuteSimpleCommand( command.c_str() ) )
            {
                if ( CheckExecuteCodeSuccession( client_.GetExecuteResult() ) )
                {
                    std::wcout << "Test [" << command << "] failed.\n";
                    continue;
                }
            }

            std::wcout << "Test [" << command << "] successful.\n";
        }

        /* Manual input tests
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
        */

        return true;
    }

    bool Application::CheckExecuteCodeSuccession( DWORD code )
    {
        switch ( code )
        {
            case 200:
                [[fallthrough]];
            case 220:
            {
                std::cout << "Code [" << code << "]. Success! Ready for new command.\n";
                return true;
            }
            default:
            {
                std::cout << "Unexpected code [" << code << "] is treated as failure.\n";
                return false;
            }
        }
    }

    std::string Application::ReadFile( const std::string& filename )
    {
        std::string result;
        std::ifstream file( filename );
        if ( !file.is_open() )
        {
            std::cout << "Failed to read [" << filename << "] file.\n";
            return "";
        }

        std::string buffer;
        while ( file.good() )
        {
            buffer.clear();
            std::getline( file, buffer );
            result.append( buffer );
        }

        file.close();

        return result;
    }

    std::vector<std::wstring> Application::FindCommands( const std::string& json, const std::string& node )
    {
        std::vector<std::wstring> result{};

        pCJson parser( cJSON_Parse( json.data() ), &cJSON_Delete );
        if ( !parser )
        {
            std::cout << "Expected json format. Failed parsing.\n";
            return result;
        }

        const auto root = parser.get();
        auto expected_node = cJSON_GetObjectItem( root, node.c_str() );
        if ( !expected_node )
        {
            std::cout << "Node [" << node << "] was not found.\n";
            return result;
        }

        const auto commands_number = cJSON_GetArraySize( expected_node );
        std::cout << "[" << node << "] has [" << commands_number << "] commands.\n";
        result.reserve( commands_number );
        for ( int i = 0; i < commands_number; ++i )
        {
            const auto command_item = cJSON_GetArrayItem( expected_node, i );
            if ( cJSON_IsString( command_item ) )
            {
                std::string converter = command_item->valuestring;
                std::wstring converted( converter.begin(), converter.end() );
                result.push_back( std::move( converted ) );
            }
        }
        std::cout << "Got [" << result.size() << "] commands.\n";

        return result;
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