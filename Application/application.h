#pragma once

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
        WindowsFtpClient client_;
        static constexpr size_t kInputBufferSize = 25;

    private:
        static CString EnterUserInfo( const std::string& name, bool required = true );
    };

} // namespace sorokin