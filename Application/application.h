#pragma once

#include <vector>
#include <string>

#include "windows_ftp_client.h"

namespace sorokin
{
    class Application : public CWinApp
    {
    public:
        Application();
        ~Application() noexcept = default;

        Application( const Application& ) = delete;
        Application( Application&& ) = delete;
        Application& operator=( const Application& ) = delete;
        Application& operator=( Application&& ) = delete;

        bool Run( int argc, char* argv[] );

    private:
        static constexpr size_t kInputBufferSize = 25;
        inline static const std::string kCommandsLocation = "commands.json";

        WindowsFtpClient client_;

    private:
        static bool CheckExecuteCodeSuccession( DWORD code );
        static std::string ReadFile( const std::string& filename );
        static std::vector<std::wstring> FindCommands( const std::string& json, const std::string& node );
        static CString EnterUserInfo( const std::string& name, bool required = true );
    };

} // namespace sorokin